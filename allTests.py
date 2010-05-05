import unittest

import EVD.allTests
import bioioTest
import cigarsTest
import treeTest

from sonLib.bioio import parseSuiteTestOptions

def allSuites():
    eVDSuite = EVD.allTests.allSuites()
    bioioSuite = unittest.makeSuite(bioioTest.TestCase, 'test')
    cigarsSuite = unittest.makeSuite(cigarsTest.TestCase, 'test')
    treeSuite = unittest.makeSuite(treeTest.TestCase, 'test')
    
    allTests = unittest.TestSuite((eVDSuite, bioioSuite, cigarsSuite, treeSuite))
    return allTests
        
def main():
    parseSuiteTestOptions()
    
    suite = allSuites()
    runner = unittest.TextTestRunner()
    runner.run(suite)
        
if __name__ == '__main__':
    main()