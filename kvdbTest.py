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

    def testSonLibKVTokyoTyrant(self):
        if socket.gethostname() == "hgwdev":
            system("sonLib_kvDatabaseTest --type=tokyotyrant --host=kolossus-10.kilokluster.ucsc.edu --port=1978 --user=cactus --pass=cactus  --db=cactus")

    def testSonLibKVSQLTest(self):
        if socket.gethostname() == "hgwdev": 
            system("sonLib_kvDatabaseTest --type=mysql --host=kolossus-10 --user=cactus --pass=cactus  --db=cactus")


if __name__ == '__main__':
    unittest.main()
