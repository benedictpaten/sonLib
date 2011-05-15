// test function for increment, bulk operations

//Database functions
#include <ktremotedb.h>
#include <kclangc.h>

using namespace std;
using namespace kyototycoon;

// main routine
int main(int argc, char** argv) {

  // create the database object
  RemoteDB *rdb = new RemoteDB();

  // open the database
  if (!rdb->open("kolossus-10.kilokluster.ucsc.edu", 1978, -1)) {
    cerr << "open error: " << rdb->error().name() << endl;
  }

  rdb->clear();
  string value;
  // arbitrary expiration time
  int64_t xt = 50;

  // test integer increment function
  uint64_t key = 7;
  size_t sizeOfKey = sizeof(uint64_t);
  uint64_t initialValue = 123;
  uint64_t incrValue = 23;
  uint64_t sizeOfRecord = sizeof(uint64_t);
  size_t sp;

  char *record;
  uint64_t currValue = 0;
  
  cout << "adding record " << initialValue << " of size " << sizeOfRecord << endl;


  // Normalize a 64-bit number in the native order into the network byte order.
  // little endian (our x86 linux machine) to big Endian....
  uint64_t KCSafeIV = kyotocabinet::hton64(initialValue);

  rdb->add((char *)&key, sizeOfKey, (const char *)&KCSafeIV, sizeOfRecord);
  cerr << "add record error: " << rdb->error().name() << endl;

  record = rdb->get((char *)&key, sizeOfKey, &sp, NULL);
  currValue = kyotocabinet::ntoh64(*((uint64_t*)record));

  cout << "added record " << currValue << " of size " << sp << endl;

  rdb->increment((char *)&key, sizeOfKey, incrValue, xt);
  //450 (the existing record was not compatible).
  cerr << "increment error: " << rdb->error().name() << endl;
  record = rdb->get((char *)&key, sizeOfKey, &sp, NULL);

  // Denormalize a 64-bit number in the network byte order into the native order.
  // (big-endian to little endian)
  currValue = kyotocabinet::ntoh64(*((uint64_t*)record));

  printf("value after increment (should be 146): %d and size %d\n", currValue, sp);

  rdb->clear();

  // the CPP version doesn't work either
  rdb->add("1", "123");
  cerr << "add record error: " << rdb->error().name() << endl;
  rdb->increment("1", 23);
  //450 (the existing record was not compatible).
  cerr << "increment error: " << rdb->error().name() << endl;
  rdb->get("1", &value);
  cout << "key 1 incremented to (should be 146): " << value << endl;

  rdb->clear();

  // test bulk set operation
  map<string,string> recs;
  recs.insert( pair<string,string>("5", "5") );
  recs.insert( pair<string,string>("6", "10") );
  recs.insert( pair<string,string>("7", "15") );
  recs.insert( pair<string,string>("8", "20") );
  int64_t retVal = rdb->set_bulk(recs, xt, true);
  cerr << "retval bulk set: " << retVal << endl;
  cerr << " bulk retval: " << rdb->error().name() << endl;

     // retrieve a record
  rdb->get("5", &value);
  cout << "key 5 set to (should be 5): " << value << endl;

  rdb->get("6", &value);
  cout << "key 6 set to (should be 10): " << value << endl;

  rdb->get("7", &value);
  cout << "key 7 set to (should be 15): " << value << endl;

  rdb->get("8", &value);
  cout << "key 8 set to (should be 20): " << value << endl;

  value = "";

  vector<string> keys;
  keys.push_back("5");
  keys.push_back("6");
  keys.push_back("7");
  keys.push_back("8");
  retVal = rdb->remove_bulk(keys,true);
  cerr << " bulk records removed: " << retVal << endl;
  cerr << " bulk retval: " << rdb->error().name() << endl;
  rdb->get("7", &value);
  cout << "key 8 set to (should be zero): " << value << endl;

  rdb->clear();
  if (!rdb->close()) {
    cerr << "close error: " << rdb->error().name() << endl;
  }

  return 0;
}
