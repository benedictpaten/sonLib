#!/usr/bin/env python

#Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
#
#Released under the MIT license, see LICENSE.txt

import unittest

import os
import sys
import socket

from sonLib.bioio import system

class TestCase(unittest.TestCase):
    
    def testSonLibKVTokyoCabinet(self):
        system("sonLib_kvDatabaseTest --type=tokyocabinet")

    def testSonLibKVKyotoTycoon(self):
            #Needs a ktserver process running on the local machine, we need to add a check for this condition to stop the test failing
            return #Disabled for now
            system("sonLib_kvDatabaseTest --type=kyototycoon --host=localhost --port=1978 --maxKTRecordSize=6500000")

    def testSonLibKVMySQLTest(self):
        if socket.gethostname() == "hgwdev": 
            system("sonLib_kvDatabaseTest --type=mysql --host=kolossus-10 --user=cactus --pass=cactus  --db=cactus")

if __name__ == '__main__':
    unittest.main()
