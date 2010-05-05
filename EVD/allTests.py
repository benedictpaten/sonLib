import unittest

import eVDScriptTest

def allSuites():
    eVDScriptSuite = unittest.makeSuite(eVDScriptTest.TestCase, 'test')
    
    allTests = unittest.TestSuite((eVDScriptSuite))
    return allTests
        
if __name__ == '__main__':
    suite = allSuites()
    runner = unittest.TextTestRunner()
    runner.run(suite)