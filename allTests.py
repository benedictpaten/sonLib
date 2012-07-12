#!/usr/bin/env python

#Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
#
#Released under the MIT license, see LICENSE.txt
import unittest

import bioioTest
import cigarsTest
import treeTest
import kvdbTest
import socket
try:
    import networkx as NX
    networkx_installed = True
    import nxtreeTest
    import nxnewickTest
except ImportError:
    networkx_installed = False

from sonLib.bioio import system
from sonLib.bioio import parseSuiteTestOptions
from sonLib.bioio import getLogLevelString


class TestCase(unittest.TestCase):        
    def testSonLibCTests(self):
        """Run m,ost the sonLib CuTests, fail if any of them fail.
        """
        system("sonLibTests %s" % getLogLevelString())

def allSuites():
    bioioSuite = unittest.makeSuite(bioioTest.TestCase, 'test')
    cigarsSuite = unittest.makeSuite(cigarsTest.TestCase, 'test')
    treeSuite = unittest.makeSuite(treeTest.TestCase, 'test')
    kvdbSuite = unittest.makeSuite(kvdbTest.TestCase, 'test')
    cuTestsSuite = unittest.makeSuite(TestCase, 'test')
    if not networkx_installed:
        allTests = unittest.TestSuite((bioioSuite, cigarsSuite, treeSuite, kvdbSuite, cuTestsSuite))
    else:
        nxtreeSuite = unittest.makeSuite(nxtreeTest.TestCase, 'test')
        nxnewickSuite = unittest.makeSuite(nxnewickTest.TestCase, 'test')
        allTests = unittest.TestSuite((bioioSuite, cigarsSuite, treeSuite, kvdbSuite, cuTestsSuite,
                                       nxtreeSuite, nxnewickSuite))
    return allTests
        
def main():
    parseSuiteTestOptions()
    
    suite = allSuites()
    runner = unittest.TextTestRunner()
    runner.run(suite)
    i = runner.run(suite)
    return len(i.failures) + len(i.errors)
        
if __name__ == '__main__':
    import sys
    sys.exit(main())
