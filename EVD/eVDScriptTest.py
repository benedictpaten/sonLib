import unittest
import os
import random

from sonLib.bioio import getTempFile
from sonLib.bioio import system

from sonLib.bioio import TestStatus

#from sonLib.misc import sonTraceRootPath
import xml.etree.ElementTree as ET

class TestCase(unittest.TestCase):
    
    def setUp(self):
        self.testNo = TestStatus.getTestSetup()
        unittest.TestCase.setUp(self)
        self.tempFiles = []
    
    def tearDown(self):
        unittest.TestCase.tearDown(self)
        for tempFile in self.tempFiles:
            os.remove(tempFile)
            
    def testEVDScript(self):
        """Tests we can get a EVD distribution using the EVD script.
        """
        tempScoresFile = getTempFile()
        self.tempFiles.append(tempScoresFile)
        tempOutputFile = getTempFile()
        self.tempFiles.append(tempOutputFile)
        
        for test in xrange(self.testNo):
            scoreNo = random.choice(xrange(1000, 10000))
            scores = [ max([ random.gauss(100.0, 20.0) for j in xrange(100) ]) for i in xrange(scoreNo) ]
            
            fileHandle = open(tempScoresFile, 'w')
            for score in scores:
                fileHandle.write("%f\n" % score)
                print "We have score %i" % score
            fileHandle.close()
            
            system("sonLib_eVDScript --scoresFile %s --outputFile %s --logLevel DEBUG" % (tempScoresFile, tempOutputFile))
            system("cat %s" % tempOutputFile)
            eVDNode = ET.parse(tempOutputFile).getroot().find("evd")
            
if __name__ == '__main__':
    unittest.main()