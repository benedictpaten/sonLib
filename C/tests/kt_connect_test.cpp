// test function for increment, bulk operations

//Database functions
#include <ktremotedb.h>

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

  rdb->add("1", "123");
  cerr << "add record error: " << rdb->error().name() << endl;
  rdb->increment("1", 23);
  //450 (the existing record was not compatible).
  cerr << "increment error: " << rdb->error().name() << endl;
  rdb->get("1", &value);
  cout << "key 1 incremented to (should be 146): " << value << endl;

  rdb->clear();

  map<string,string> recs;
  recs.insert( pair<string,string>("5", "5") );
  recs.insert( pair<string,string>("6", "10") );
  recs.insert( pair<string,string>("7", "15") );
  recs.insert( pair<string,string>("8", "20") );
  int64_t retVal = rdb->set_bulk(recs, 5, true);
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

 // C version of increment

    /* 
 int64_t key = 2;
 size_t keysize = sizeof(int64_t);
 int64_t incAmt = 55;
 int64_t xt = 5;
 
 const char *initialVal = "43";
 int64_t sizeOfRecord = sizeof(int64_t);

 rdb->add((char *)&key, keysize, initialVal, sizeOfRecord);
 cerr << "add record error: " << rdb->error().name() << endl;
 rdb->increment((char *)&key, keysize, incAmt, xt);
 cerr << "increment record error: " << rdb->error().name() << endl;

  // close the database
  if (!rdb->close()) {
    cerr << "close error: " << rdb->error().name() << endl;
  }
    */

  return 0;
}
