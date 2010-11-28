import unittest

import bioioTest
import cigarsTest
import treeTest
import socket
from sonLib.bioio import system

from sonLib.bioio import parseSuiteTestOptions


class TestCase(unittest.TestCase):        
    def testSonLibCTests(self):
        """Run m,ost the sonLib CuTests, fail if any of them fail.
        """
        system("sonLibTests")

    def testSonLibKVDatabaseTests(self):
        """Run all the sonLib KV database tests, fail if any of them fail.
        """
        system("sonLib_kvDatabaseTest")
        if socket.gethostname() == "hgwdev":
            system("sonLib_kvDatabaseTest --type=mysql --host=kolossus-10 --user=cactus --pass=cactus  --db=cactus")

def allSuites():
    bioioSuite = unittest.makeSuite(bioioTest.TestCase, 'test')
    cigarsSuite = unittest.makeSuite(cigarsTest.TestCase, 'test')
    treeSuite = unittest.makeSuite(treeTest.TestCase, 'test')
    cuTestsSuite = unittest.makeSuite(TestCase, 'test')
    
    allTests = unittest.TestSuite((bioioSuite, cigarsSuite, treeSuite, cuTestsSuite))
    return allTests
        
def main():
    parseSuiteTestOptions()
    
    suite = allSuites()
    runner = unittest.TextTestRunner()
    runner.run(suite)
        
if __name__ == '__main__':
    main()
