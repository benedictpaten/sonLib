#include "sonLibGlobalsPrivate.h"

static st_Hash *hash;
static st_Hash *hash2;
static int32_t *one, *two, *three, *four, *five, *six;

static uint32_t hashKey(void *o) {
	return *((int32_t *)o);
}

static int32_t hashEqualsKey(void *o, void *o2) {
	return *((int32_t *)o) == *((int32_t *)o2);
}

static void destructKey(void *o) {
	destructInt(o);
}

static void destructValue(void *o) {
	destructInt(o);
}

static void testSetup() {
	//compare by value of memory address
	hash = st_hash_construct();
	//compare by value of ints.
	hash2 = st_hash_construct3(hashKey, hashEqualsKey, destructKey, destructValue);
	one = constructInt(0);
	two = constructInt(1);
	three = constructInt(2);
	four = constructInt(3);
	five = constructInt(4);
	six = constructInt(5);

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

	int32_t *i = constructInt(0);

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

	destructInt(i);

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
	st_Hash *hash3 = st_hash_construct();
	CuAssertTrue(testCase, st_hash_size(hash3) == 0);
	st_hash_destruct(hash3);

	testTeardown();
}

static void testHash_testIterator(CuTest *testCase) {
	testSetup();

	st_HashIterator *iterator = st_hash_getIterator(hash);
	st_HashIterator *iteratorCopy = st_hash_copyIterator(iterator);
	int32_t i=0;
	st_Hash *seen = st_hash_construct();
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


CuSuite* sonLibHashTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, testHash_search);
	SUITE_ADD_TEST(suite, testHash_remove);
	SUITE_ADD_TEST(suite, testHash_insert);
	SUITE_ADD_TEST(suite, testHash_size);
	SUITE_ADD_TEST(suite, testHash_testIterator);
	SUITE_ADD_TEST(suite, testHash_construct);
	return suite;
}
