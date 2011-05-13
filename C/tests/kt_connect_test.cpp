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

  rdb->add("1", "123");
  rdb->increment("1", 23);
  // 450 (the existing record was not compatible).
  cerr << "increment error: " << rdb->error().name() << endl;

     // retrieve a record
  string value;
  if (rdb->get("1", &value)) {
    cout << "key 1 set to (123 + 23 incr)" << value << endl;
  } else {
    cerr << "get error: " << rdb->error().name() << endl;
  }

  // close the database
  if (!rdb->close()) {
    cerr << "close error: " << rdb->error().name() << endl;
  }

  return 0;
}
