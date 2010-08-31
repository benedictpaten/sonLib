import unittest

class TestCase(unittest.TestCase):
    
    def setUp(self):
        unittest.TestCase.setUp(self)
    
    def tearDown(self):
        unittest.TestCase.tearDown(self)
        
    def test(self):
        pass
        
if __name__ == '__main__':
    unittest.main()