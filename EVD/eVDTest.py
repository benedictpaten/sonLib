import unittest

from sonLib.misc import sonTraceRootPath
import xml.etree.ElementTree as ET
from sonLib.EVD.eVD import calculateEVDProbFromScore
from sonLib.EVD.eVD import calculateScoreFromEVDProb

class TestCase(unittest.TestCase):
    
    def setUp(self):
        unittest.TestCase.setUp(self)
    
    def tearDown(self):
        unittest.TestCase.tearDown(self)
        
    def testEVDFunctions(self):
        paramFile = sonTraceRootPath() + "/src/pecan2/pecan2_modelParams.xml"
        eVDNode = ET.parse(paramFile).getroot().find("evd")
        for score in xrange(2000, 4000, 50):
            i = calculateEVDProbFromScore(score, eVDNode, 10)
            j = calculateScoreFromEVDProb(i, eVDNode, 10)
            print "For score %s got prob: %s and reversed it to get score of %s, diff: %s" % (score, i, j, score-j)
        
if __name__ == '__main__':
    unittest.main()