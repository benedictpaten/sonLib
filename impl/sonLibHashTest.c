#include "sonLibGlobalsPrivate.h"

static st_Hash *hash;
static st_Hash *hash2;
static stIntTuple *one, *two, *three, *four, *five, *six;

static void testSetup() {
	//compare by value of memory address
	hash = stHash_construct();
	//compare by value of ints.
	hash2 = stHash_construct3((uint32_t (*)(const void *))stIntTuple_hashKey,
			(int (*)(const void *, const void *))stIntTuple_equalsFn,
			(void (*)(void *))stIntTuple_destruct,
			(void (*)(void *))stIntTuple_destruct);
	one = stIntTuple_construct(1, 0);
	two = stIntTuple_construct(1, 1);
	three = stIntTuple_construct(1, 2);
	four = stIntTuple_construct(1, 3);
	five = stIntTuple_construct(1, 4);
	six = stIntTuple_construct(1, 5);

	st_hash_insert(hash, one, two);
	st_hash_insert(hash, three, four);
	st_hash_insert(hash, five, six);

	st_hash_insert(hash2, one, two);
	st_hash_insert(hash2, three, four);
	st_hash_insert(hash2, five, six);
}

static void testTeardown() {
	st_hash_destruct(hash);
	st_hash_destruct(hash2);
}

static void testHash_construct(CuTest* testCase) {
	testSetup();
	/* Do nothing */
	testTeardown();
}

static void testHash_search(CuTest* testCase) {
	testSetup();

	stIntTuple *i = stIntTuple_construct(1, 0);

	//Check search by memory address
	CuAssertTrue(testCase, st_hash_search(hash, one) == two);
	CuAssertTrue(testCase, st_hash_search(hash, three) == four);
	CuAssertTrue(testCase, st_hash_search(hash, five) == six);
	//Check not present
	CuAssertTrue(testCase, st_hash_search(hash, six) == NULL);
	CuAssertTrue(testCase, st_hash_search(hash, i) == NULL);

	//Check search by memory address
	CuAssertTrue(testCase, st_hash_search(hash2, one) == two);
	CuAssertTrue(testCase, st_hash_search(hash2, three) == four);
	CuAssertTrue(testCase, st_hash_search(hash2, five) == six);
	//Check not present
	CuAssertTrue(testCase, st_hash_search(hash2, six) == NULL);
	//Check is searching by memory.
	CuAssertTrue(testCase, st_hash_search(hash2, i) == two);

	stIntTuple_destruct(i);

	testTeardown();
}

static void testHash_remove(CuTest* testCase) {
	testSetup();

	CuAssertTrue(testCase, st_hash_remove(hash, one) == two);
	CuAssertTrue(testCase, st_hash_search(hash, one) == NULL);

	CuAssertTrue(testCase, st_hash_remove(hash2, one) == two);
	CuAssertTrue(testCase, st_hash_search(hash2, one) == NULL);

	st_hash_insert(hash2, one, two);
	CuAssertTrue(testCase, st_hash_search(hash2, one) == two);

	testTeardown();
}

static void testHash_insert(CuTest* testCase) {
	/*
	 * Tests inserting already present keys.
	 */
	testSetup();

	CuAssertTrue(testCase, st_hash_search(hash, one) == two);
	st_hash_insert(hash, one, two);
	CuAssertTrue(testCase, st_hash_search(hash, one) == two);
	st_hash_insert(hash, one, three);
	CuAssertTrue(testCase, st_hash_search(hash, one) == three);
	st_hash_insert(hash, one, two);
	CuAssertTrue(testCase, st_hash_search(hash, one) == two);

	testTeardown();
}

static void testHash_size(CuTest *testCase) {
	/*
	 * Tests the size function of the hash.
	 */
	testSetup();

	CuAssertTrue(testCase, st_hash_size(hash) == 3);
	CuAssertTrue(testCase, st_hash_size(hash2) == 3);
	st_Hash *hash3 = stHash_construct();
	CuAssertTrue(testCase, st_hash_size(hash3) == 0);
	st_hash_destruct(hash3);

	testTeardown();
}

static void testHash_testIterator(CuTest *testCase) {
	testSetup();

	st_HashIterator *iterator = st_hash_getIterator(hash);
	st_HashIterator *iteratorCopy = st_hash_copyIterator(iterator);
	int32_t i=0;
	st_Hash *seen = stHash_construct();
	for(i=0; i<3; i++) {
		void *o = st_hash_getNext(iterator);
		CuAssertTrue(testCase, o != NULL);
		CuAssertTrue(testCase, st_hash_search(hash, o) != NULL);
		CuAssertTrue(testCase, st_hash_search(seen, o) == NULL);
		CuAssertTrue(testCase, st_hash_getNext(iteratorCopy) == o);
		st_hash_insert(seen, o, o);
	}
	CuAssertTrue(testCase, st_hash_getNext(iterator) == NULL);
	CuAssertTrue(testCase, st_hash_getNext(iterator) == NULL);
	CuAssertTrue(testCase, st_hash_getNext(iteratorCopy) == NULL);
	st_hash_destruct(seen);
	st_hash_destructIterator(iterator);
	st_hash_destructIterator(iteratorCopy);

	testTeardown();
}

static void testHash_testGetKeys(CuTest *testCase) {
	testSetup();
	st_List *list = st_hash_getKeys(hash2);
	CuAssertTrue(testCase, st_list_length(list) == 3);
	CuAssertTrue(testCase, st_list_contains(list, one));
	CuAssertTrue(testCase, st_list_contains(list, three));
	CuAssertTrue(testCase, st_list_contains(list, five));
	st_list_destruct(list);
	testTeardown();
}

static void testHash_testGetValues(CuTest *testCase) {
	testSetup();
	st_List *list = st_hash_getValues(hash2);
	CuAssertTrue(testCase, st_list_length(list) == 3);
	CuAssertTrue(testCase, st_list_contains(list, two));
	CuAssertTrue(testCase, st_list_contains(list, four));
	CuAssertTrue(testCase, st_list_contains(list, six));
	st_list_destruct(list);
	testTeardown();
}

CuSuite* sonLibHashTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, testHash_search);
	SUITE_ADD_TEST(suite, testHash_remove);
	SUITE_ADD_TEST(suite, testHash_insert);
	SUITE_ADD_TEST(suite, testHash_size);
	SUITE_ADD_TEST(suite, testHash_testIterator);
	SUITE_ADD_TEST(suite, testHash_construct);
	SUITE_ADD_TEST(suite, testHash_testGetKeys);
	SUITE_ADD_TEST(suite, testHash_testGetValues);
	return suite;
}
