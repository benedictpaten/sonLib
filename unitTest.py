"""
Support for unit testing.
"""
import os, unittest

class TestCase(unittest.TestCase):
    """A class that add specialized support methods for test cases"""

    def getTestTempFile(self, suffix):
        "create a temporary file based on test case name"
        tempDir = "tmp"
        if not os.path.exists(tempDir):
            os.makedirs(tempDir)
        tempFile = tempDir + "/" + self.id() + "." + suffix
        if os.path.exists(tempFile):
            os.unlink(tempFile)
        return tempFile
    
