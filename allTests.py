import unittest

import EVD.allTests
import bioioTest
import cigarsTest
import treeTest
from sonLib.bioio import system

from sonLib.bioio import parseSuiteTestOptions


class TestCase(unittest.TestCase):        
    def testSonLibCTests(self):
        """Run all the sonLibAPI CuTests, fail if any of them fail.
        """
        system("sonLibAPITests")

def allSuites():
    eVDSuite = EVD.allTests.allSuites()
    bioioSuite = unittest.makeSuite(bioioTest.TestCase, 'test')
    cigarsSuite = unittest.makeSuite(cigarsTest.TestCase, 'test')
    treeSuite = unittest.makeSuite(treeTest.TestCase, 'test')
    cuTestsSuite = unittest.makeSuite(TestCase, 'test')
    
    allTests = unittest.TestSuite((eVDSuite, bioioSuite, cigarsSuite, treeSuite, cuTestsSuite))
    return allTests
        
def main():
    parseSuiteTestOptions()
    
    suite = allSuites()
    runner = unittest.TextTestRunner()
    runner.run(suite)
        
if __name__ == '__main__':
    main()