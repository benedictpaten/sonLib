import unittest

import eVDScriptTest
import eVDTest

def allSuites():
    eVDScriptSuite = unittest.makeSuite(eVDScriptTest.TestCase, 'test')
    eVDTestSuite = unittest.makeSuite(eVDTest.TestCase, 'test')
    
    allTests = unittest.TestSuite((eVDScriptSuite, eVDTestSuite))
    return allTests
        
if __name__ == '__main__':
    suite = allSuites()
    runner = unittest.TextTestRunner()
    runner.run(suite)