#!/usr/bin/env python

import sys
import os
import re
import logging
import logging.handlers
import tempfile
import random
import math
from optparse import OptionParser
from tree import BinaryTree
from misc import close
import subprocess
import array

DEFAULT_DISTANCE = 0.001

#########################################################
#########################################################
#########################################################  
#global logging settings / log functions
#########################################################
#########################################################
#########################################################

loggingFormatter = logging.Formatter('%(asctime)s %(levelname)s %(lineno)s %(message)s')

def __setDefaultLogger():
    l = logging.getLogger()
    l.setLevel(logging.CRITICAL)
    handler = logging.StreamHandler(sys.stderr)
    #handler.setLevel(logging.CRITICAL) #null logger, to stop annoying error message
    l.addHandler(handler) 
    return l

logger = __setDefaultLogger()
logLevelString = "CRITICAL"

def getLogLevelString():
    return logLevelString

def addLoggingFileHandler(fileName, rotatingLogging=False):
    if rotatingLogging:
        handler = logging.handlers.RotatingFileHandler(fileName, maxBytes=1000000, backupCount=1)
    else:
        handler = logging.FileHandler(fileName)
    logger.addHandler(handler)
    
def setLogLevel(logLevel):
    assert logLevel in [ "INFO", "CRITICAL", "DEBUG" ] #Log level must be one of these strings.
    global logLevelString
    logLevelString = logLevel
    if logLevel == "INFO":
        logger.setLevel(logging.INFO)
    elif logLevel == "DEBUG":
        logger.setLevel(logging.DEBUG)
    elif logLevel == "CRITICAL":
        logger.setLevel(logging.CRITICAL)

def logFile(fileName, printFunction=logger.info):
    """Writes out a formatted version of the given log file
    """
    printFunction("Reporting file: %s" % fileName)
    shortName = fileName.split("/")[-1]
    fileHandle = open(fileName, 'r')
    line = fileHandle.readline()
    while line != '':
        if line[-1] == '\n':
            line = line[:-1]
        printFunction("%s:\t%s" % (shortName, line))
        line = fileHandle.readline()
    fileHandle.close()


#########################################################
#########################################################
#########################################################
#system wrapper command
#########################################################
#########################################################
#########################################################

def system(command):
    logger.debug("Running the command: %s" % command)
    process = subprocess.Popen(command, shell=True, stdout=sys.stdout, stderr=sys.stderr)
    sts = os.waitpid(process.pid, 0)
    i = sts[1]
    if i != 0:
        print >>sys.stderr, "Something went wrong: %i" % i
        raise RuntimeError("Command: %s exited with non-zero status %i" % (command, i))

def popen(command, tempFile):
    """Runs a command and captures standard out in the given temp file.
    """
    fileHandle = open(tempFile, 'w')
    logger.debug("Running the command: %s" % command)
    process = subprocess.Popen(command, shell=True, stdout=fileHandle)
    sts = os.waitpid(process.pid, 0)
    fileHandle.close()
    i = sts[1]
    if i != 0:
        print >>sys.stderr, "Something went wrong: %i" % i
        raise RuntimeError("Command: %s exited with non-zero status %i" % (command, i))
 
#########################################################
#########################################################
#########################################################  
#testing settings
#########################################################
#########################################################
#########################################################

class TestStatus:
    ###Global variables used by testing framework to run tests.
    TEST_SHORT = 0
    TEST_MEDIUM = 1
    TEST_LONG = 2
    TEST_VERY_LONG = 3
    
    TEST_STATUS = TEST_SHORT
    
    SAVE_ERROR_LOCATION = None
    
    def getTestStatus():
        return TestStatus.TEST_STATUS
    getTestStatus = staticmethod(getTestStatus)
    
    def setTestStatus(status):
        assert status in (TestStatus.TEST_SHORT, TestStatus.TEST_MEDIUM, TestStatus.TEST_LONG, TestStatus.TEST_VERY_LONG)
        TestStatus.TEST_STATUS = status
    setTestStatus = staticmethod(setTestStatus)
    
    def getSaveErrorLocation():
        """Location to in which to write inputs which created test error.
        """
        return TestStatus.SAVE_ERROR_LOCATION
    getSaveErrorLocation = staticmethod(getSaveErrorLocation)
    
    def setSaveErrorLocation(dir):
        """Set location in which to write inputs which created test error.
        """
        logger.info("Location to save error files in: %s" % dir)
	assert os.path.isdir(dir)
        TestStatus.SAVE_ERROR_LOCATION = dir
    setSaveErrorLocation = staticmethod(setSaveErrorLocation)
    
    def getTestSetup(shortTestNo=5, mediumTestNo=20, longTestNo=200, veryLongTestNo=0):
        if TestStatus.TEST_STATUS == TestStatus.TEST_SHORT:
            return shortTestNo
        elif TestStatus.TEST_STATUS == TestStatus.TEST_MEDIUM:
            return mediumTestNo
        elif TestStatus.TEST_STATUS == TestStatus.TEST_LONG:
            return longTestNo
        else: #Used for long example tests
            return veryLongTestNo
    getTestSetup = staticmethod(getTestSetup)
    
    def getPathToDataSets():
        """This method is used to store the location of 
        the path where all the data sets used by tests for analysis are kept.
        These are not kept in the distrbution itself for reasons of size.
        """
        assert "SON_TRACE_DATASETS" in os.environ
        return os.environ["SON_TRACE_DATASETS"]
    getPathToDataSets = staticmethod(getPathToDataSets)
    

def saveInputs(savedInputsDir, listOfFilesAndDirsToSave):
    """Copies the list of files to a directory created in the save inputs dir,
    and returns the name of this directory.
    """
    logger.info("Saving the inputs: %s to the directory: %s" % (" ".join(listOfFilesAndDirsToSave), savedInputsDir))
    assert os.path.isdir(savedInputsDir)
    #savedInputsDir = getTempDirectory(saveInputsDir)
    createdFiles = []
    for fileName in listOfFilesAndDirsToSave:
        if os.path.isfile(fileName):
            copiedFileName = os.path.join(savedInputsDir, os.path.split(fileName)[-1])
            system("cp %s %s" % (fileName, copiedFileName))
        else:
            copiedFileName = os.path.join(savedInputsDir, os.path.split(fileName)[-1]) + ".tar"
            system("tar -cf %s %s" % (copiedFileName, fileName))
        createdFiles.append(copiedFileName)
    return createdFiles

#########################################################
#########################################################
#########################################################
#options parser functions
#########################################################
#########################################################
#########################################################

def getBasicOptionParser(usage, version):
    usage = "usage: %prog [options]"
    version = "%prog 0.1"
    parser = OptionParser(usage=usage, version=version)
    
    parser.add_option("--logInfo", dest="logInfo", action="store_true",
                     help="Turn on logging at INFO level",
                     default=False)
    
    parser.add_option("--logDebug", dest="logDebug", action="store_true",
                     help="Turn on logging at DEBUG level",
                     default=False)
    
    parser.add_option("--logLevel", dest="logLevel", type="string",
                      help="Log at level (may be either INFO/DEBUG/CRITICAL)")
    
    parser.add_option("--tempDirRoot", dest="tempDirRoot", type="string",
                      help="Path to where temporary directory containing all temp files are created, by default uses the current working directory as the base",
                      default=os.getcwd())
    
    parser.add_option("--logFile", dest="logFile", type="string",
                      help="File to log in")
    
    parser.add_option("--noRotatingLogging", dest="logRotating", action="store_false",
                     help="Turn off rotating logging, which prevents log files getting too big",
                     default=True)
    
    return parser

def parseBasicOptions(parser):
    (options, args) = parser.parse_args()
        
    #We can now set up the logging info.
    if options.logLevel != None:
        setLogLevel(options.logLevel) #Use log level, unless flags are set..   
     
    if options.logInfo:
        setLogLevel("INFO")
    elif options.logDebug:
        setLogLevel("DEBUG")
        
    logger.info("Logging set at level: %s" % logLevelString)  
    
    if options.logFile != None:
        addLoggingFileHandler(options.logFile, options.logRotating)
    
    logger.info("Logging to file: %s" % options.logFile)  
    
    #Set up the temp dir root
    if options.tempDirRoot == "None":
        options.tempDirRoot = os.getcwd()
    
    return options, args

def parseSuiteTestOptions():
    parser = getBasicOptionParser("usage: %prog [options]", "%prog 0.1")
    
    parser.add_option("--testLength", dest="testLength", type="string",
                     help="Control the length of the tests either SHORT/MEDIUM/LONG/VERY_LONG",
                     default="SHORT")
    
    parser.add_option("--saveError", dest="saveError", type="string",
                     help="Directory in which to store the inputs of failed tests")
    
    options, args = parseBasicOptions(parser)
    logger.info("Parsed arguments")
    
    if options.testLength == "SHORT":
        TestStatus.setTestStatus(TestStatus.TEST_SHORT)
    elif options.testLength == "MEDIUM":
        TestStatus.setTestStatus(TestStatus.TEST_MEDIUM)
    elif options.testLength == "LONG":
        TestStatus.setTestStatus(TestStatus.TEST_LONG)
    else: 
        assert options.testLength == "VERY_LONG" #Otherwise an unrecognised option
        TestStatus.setTestStatus(TestStatus.TEST_VERY_LONG)
    
    if options.saveError != None:
        TestStatus.setSaveErrorLocation(options.saveError)
        
    return options, args
    
   
#########################################################
#########################################################
#########################################################
#temp files
#########################################################
#########################################################
#########################################################

def getRandomAlphaNumericString(length=10):
    """Returns a random alpha numeric string of the given length.
    """
    return "".join([ random.choice('0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz') for i in xrange(0, length) ])
    
def getTempFile(suffix="", rootDir=None):
    """Returns a string representing a temporary file, that must be manually deleted
    """
    if rootDir == None:
        handle, tmpFile = tempfile.mkstemp(suffix)
        os.close(handle)
        return tmpFile
    else:
        tmpFile = os.path.join(rootDir, getRandomAlphaNumericString() + suffix)
        open(tmpFile, 'w').close()
        return tmpFile

def getTempDirectory(rootDir=None):
    """
    returns a temporary directory that must be manually deleted
    """
    if rootDir == None:
        return tempfile.mkdtemp()
    else:
        rootDir = os.path.join(rootDir, getRandomAlphaNumericString())
        os.mkdir(rootDir)
        return rootDir
    
class TempFileTree:
    """A hierarchical tree structure for storing directories of files/dirs/
    
    The total number of legal files is equal to filesPerDir**levels.
    filesPerDer and levels must both be greater than zero.
    The rootDir may or may not yet exist (and may or may not be empty), though
    if files exist in the dirs of levels 0 ... level-1 then they must be dirs,
    which will be indexed the by tempfile tree.
    """
    def __init__(self, rootDir, filesPerDir=100, levels=4, ):
        #Do basic checks of input
        assert(filesPerDir) >= 1
        assert(levels) >= 1
        if not os.path.isdir(rootDir):
            #Make the root dir
            os.mkdir(rootDir)
        
        #Basic attributes of system at start up.
        self.levelNo = levels
        self.filesPerDir = filesPerDir
        self.rootDir = rootDir
        #Dynamic variables
        self.tempDir = rootDir
        self.level = 0
        #These two variables will only refer to the existance of this class instance.
        self.tempFilesCreated = 0
        self.tempFilesDestroyed = 0
        
        currentFiles = self.listFiles()
        logger.info("We have setup the temp file tree, it contains %s files currently, \
        %s of the possible total" % \
        (len(currentFiles), len(currentFiles)/math.pow(filesPerDir, levels)))
    
    def getTempFile(self, suffix="", makeDir=False):
        while self.level < self.levelNo:
            #Basic checks for start of loop
            assert self.level >= 0
            assert os.path.isdir(self.tempDir)
            fileNames = os.listdir(self.tempDir)
            #If tempDir contains max file number then:
            if len(fileNames) >= self.filesPerDir:
                #if level number is already 0 raise an exception
                if self.level == 0:
                    raise RuntimeError("We ran out of space to make temp files")
                #reduce level number by one, chop off top of tempDir.
                self.level -= 1
                self.tempDir = os.path.split(self.tempDir)[0]
            else:
                if self.level == self.levelNo-1:
                    #make temporary file in dir and return it.
                    if makeDir:
                        return getTempDirectory(rootDir=self.tempDir)
                    else:
                        return getTempFile(suffix=suffix, rootDir=self.tempDir)
                else:
                    #mk new dir, and add to tempDir path, inc the level buy one.
                    self.tempDir = getTempDirectory(rootDir=self.tempDir)
                    self.level += 1
    
    def getTempDirectory(self):
        return self.getTempFile(makeDir=True)
    
    def __destroyFile(self, tempFile):
        #If not part of the current tempDir, from which files are being created.
        baseDir = os.path.split(tempFile)[0]
        if baseDir != self.tempDir:
            while True: #Now remove any parent dirs that are empty.
                if os.listdir(baseDir) == [] and baseDir != self.rootDir:
                    os.rmdir(baseDir)
                    baseDir = os.path.split(baseDir)[0]
                else:
                    break
    
    def destroyTempFile(self, tempFile):
        """Removes the temporary file in the temp file dir, checking its in the temp file tree.
        """
        #Do basic assertions for goodness of the function
        assert os.path.isfile(tempFile)
        assert os.path.commonprefix((self.rootDir, tempFile)) == self.rootDir #Checks file is part of tree
        #Update stats.
        self.tempFilesDestroyed += 1
        #Do the actual removal
        os.remove(tempFile)
        self.__destroyFile(tempFile)
    
    def destroyTempDir(self, tempDir):
        """Removes a temporary directory in the temp file dir, checking its in the temp file tree.
        The dir will be removed regardless of if it is empty.
        """
        #Do basic assertions for goodness of the function
        assert os.path.isdir(tempDir)
        assert os.path.commonprefix((self.rootDir, tempDir)) == self.rootDir #Checks file is part of tree
        #Update stats.
        self.tempFilesDestroyed += 1
        #Do the actual removal
        system("rm -rf %s" % tempDir)
        self.__destroyFile(tempDir)
   
    def listFiles(self):
        """Gets all files in the temp file tree (which may be dirs).
        """
        def fn(dirName, level, files):
            if level == self.levelNo-1:
                for fileName in os.listdir(dirName):
                    absFileName = os.path.join(dirName, fileName)
                    files.append(absFileName)
            else:
                for subDir in os.listdir(dirName):
                    absDirName = os.path.join(dirName, subDir)
                    assert os.path.isdir(absDirName)
                    fn(absDirName, level+1, files)
        files = []
        fn(self.rootDir, 0, files)
        return files
   
    def destroyTempFiles(self):
        """Destroys all temp temp file hierarchy, getting rid of all files.
        """
        os.system("rm -rf %s" % self.rootDir)
        logger.debug("Temp files created: %s, temp files actively destroyed: %s" % (self.tempFilesCreated, self.tempFilesDestroyed))     

#########################################################
#########################################################
#########################################################
#misc input/output functions
#########################################################
#########################################################
#########################################################

def getNextNonCommentLine(file):
    line = file.readline()
    while line != '' and line[0] == '#':
        line = file.readline()
    return line

def removeNewLine(line):
    if line != '' and line[-1] == '\n':
        return line[:-1]
    return line

def readFirstLine(inputFile):
    i = open(inputFile, 'r')
    j = removeNewLine(i.readline())
    i.close()
    return j

def padWord(word, length=25):
    if len(word) > length:
        return word[:length]
    if len(word) < length:
        return word + " "*(length-len(word))
    return word

#########################################################
#########################################################
#########################################################
#fasta functions
#########################################################
#########################################################
#########################################################

def fastaNormaliseHeader(fastaHeader):
    """Removes white space which is treated weirdly by many programs.
    """
    i = fastaHeader.split()
    if len(i) > 0:
        return i[0]
    return ""

def fastaDecodeHeader(fastaHeader):
    """Decodes the fasta header
    """
    return fastaHeader.split("|")

def fastaEncodeHeader(attributes):
    """Decodes the fasta header
    """
    for i in attributes:
        assert len(str(i).split()) == 1
    return "|".join([ str(i) for i in attributes ])

def fastaRead(fileHandle):
    """iteratively a sequence for each '>' it encounters, ignores '#' lines
    """
    line = fileHandle.readline()
    while line != '':
        if line[0] == '>':
            name = line[1:-1]
            line = fileHandle.readline()
            seq = array.array('c')
            while line != '' and line[0] != '>':
                if line[0] != '#':
                    seq.extend([ i for i in line[:-1] if i != '\t' and i != ' ' ])
                line = fileHandle.readline()
            for i in seq:
                assert (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z') or i == '-' #For safety and sanity I only allows roman alphabet characters in fasta sequences. 
            yield name, seq.tostring()
        else:
            line = fileHandle.readline()

def fastaWrite(fileHandle, name, seq):
    """Writes out fasta file
    """
    assert seq.__class__ == "".__class__
    for i in seq:
        assert (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z') or i == '-' #For safety and sanity I only allows roman alphabet characters in fasta sequences. 
    fileHandle.write(">%s\n%s\n" % (name, seq))

def _getMultiFastaOffsets(fasta):
    """Reads in columns of multiple alignment and returns them iteratively
    """
    f = open(fasta, 'r')
    i = 0
    j = f.read(1)
    l = []
    while j != '':
        i += 1
        if j == '>':
            i += 1
            while f.read(1) != '\n':
                i += 1
            l.append(i)
        j = f.read(1)
    f.close()
    return l

def fastaAlignmentRead(fasta, mapFn=(lambda x : x), l=None):
    """
    reads in columns of multiple alignment and returns them iteratively
    """
    if l == None:
        l = _getMultiFastaOffsets(fasta)
    else:
        l = l[:]
    seqNo = len(l)
    for i in xrange(0, seqNo):
        j = open(fasta, 'r')
        j.seek(l[i])
        l[i] = j
    column = [sys.maxint]*seqNo
    if seqNo != 0:
        while True:
            for j in xrange(0, seqNo):
                i = l[j].read(1)
                while i == '\n':
                    i = l[j].read(1)
                column[j] = i
            if column[0] == '>' or column[0] == '':
                for j in xrange(1, seqNo):
                    assert column[j] == '>' or column[j] == ''
                break
            for j in xrange(1, seqNo):
                 assert column[j] != '>' and column[j] != ''
                 column[j] = mapFn(column[j])
            yield column[:]
    for i in l:
        i.close()

def fastaAlignmentWrite(columnAlignment, names, seqNo, fastaFile, 
                        filter=lambda x : True):
    """
    Writes out column alignment to given file multi-fasta format
    """
    fastaFile = open(fastaFile, 'w')
    columnAlignment = [ i for i in columnAlignment if filter(i) ]
    for seq in xrange(0, seqNo):
        fastaFile.write(">%s\n" % names[seq])
        for column in columnAlignment:
            fastaFile.write(column[seq])
        fastaFile.write("\n")
    fastaFile.close()

def getRandomSequence(length=500):
    """Generates a random name and sequence.
    """
    fastaHeader = ""
    for i in xrange(int(random.random()*100)):
        fastaHeader = fastaHeader + random.choice([ 'A', 'C', '0', '9', ' ', '\t' ])
    return (fastaHeader, \
            "".join([ random.choice([ 'A', 'C', 'T', 'G', 'A', 'C', 'T', 'G', 'A', 'C', 'T', 'G', 'A', 'C', 'T', 'G', 'A', 'C', 'T', 'G', 'N' ]) for i in xrange((int)(random.random() * length))]))

def _expLength(i=0, prob=0.95):
    if random.random() >= prob:
        return _expLength(i+1)
    return i

def mutateSequence(seq, distance):
    """Mutates the DNA sequence for use in testing.
    """
    subProb=distance
    inProb=0.05*distance
    deProb=0.05*distance
    contProb=0.9
    l = []
    bases = [ 'A', 'C', 'T', 'G' ]
    i=0
    while i < len(seq):
        if random.random() < subProb:
            l.append(random.choice(bases))
        else:
            l.append(seq[i])
        if random.random() < inProb:
            l += getRandomSequence(_expLength(0, contProb))[1]
        if random.random() < deProb:
            i += int(_expLength(0, contProb))
        i += 1
    return "".join(l)

def reverseComplement(seq):
    seq = list(seq)
    seq.reverse()
    dNA = { 'A':'T', 'T':'A', 'C':'G', 'G':'C', 'a':'t', 't':'a', 'c':'g', 'g':'c' }
    def fn(i):
        if i in dNA:
            return dNA[i]
        return i
    return "".join([ fn(i) for i in seq ])
 
        
#########################################################
#########################################################
#########################################################
#newick tree functions
#########################################################
#########################################################
#########################################################
       
def newickTreeParser(newickTree, defaultDistance=DEFAULT_DISTANCE, \
                     sortNonBinaryNodes=False, reportUnaryNodes=False):
    """
    lax newick tree parser
    """
    newickTree = newickTree.replace("(", " ( ")
    newickTree = newickTree.replace(")", " ) ")
    newickTree = newickTree.replace(":", " : ")
    newickTree = newickTree.replace(";", "")
    newickTree = newickTree.replace(",", " , ")
    
    newickTree = re.compile("[\s]*").split(newickTree)
    while "" in newickTree:
        newickTree.remove("")
    def fn(newickTree, i):
        if i[0] < len(newickTree):
            if newickTree[i[0]] == ':':
                d = float(newickTree[i[0]+1])
                i[0] += 2
                return d
        return defaultDistance
    def fn2(newickTree, i):
        if i[0] < len(newickTree):
            j = newickTree[i[0]]
            if j != ':' and j != ')' and j != ',':
                i[0] += 1
                return j
        return None
    def fn3(newickTree, i):
        if newickTree[i[0]] == '(':
            #subTree1 = None
            subTreeList = []
            i[0] += 1
            k = []
            while newickTree[i[0]] != ')':
                if newickTree[i[0]] == ',':
                    i[0] += 1
                subTreeList.append(fn3(newickTree, i))
            i[0] += 1
            def cmp(i, j):
                if i.distance < j.distance:
                    return -1
                if i.distance > j.distance:
                    return 1
                return 0
            if sortNonBinaryNodes:
                subTreeList.sort(cmp)
            subTree1 = subTreeList[0]
            if len(subTreeList) > 1:
                for subTree2 in subTreeList[1:]:
                    subTree1 = BinaryTree(0.0, True, subTree1, subTree2, None)
                subTree1.iD = fn2(newickTree, i)
                subTree1.distance += fn(newickTree, i)
            elif reportUnaryNodes:
                subTree1 = BinaryTree(0.0, True, subTree1, None, None)
                subTree1.iD = fn2(newickTree, i)
                subTree1.distance += fn(newickTree, i)
            else:
                fn2(newickTree, i)
                subTree1.distance += fn(newickTree, i)
            return subTree1
        leafID = fn2(newickTree, i)
        return BinaryTree(fn(newickTree, i), False, None, None, leafID)
    return fn3(newickTree, [0])

def printBinaryTree(binaryTree, includeDistances, dontStopAtID=True, distancePrintFn=(lambda f : "%f" % f)):
    def fn(binaryTree):
        #print " tree Node ", binaryTree.left, binaryTree.right, binaryTree.distance, binaryTree.internal, binaryTree.iD 
        if binaryTree.iD != None:
            iD = str(binaryTree.iD)
        else:
            iD = ''
        if binaryTree.internal and (dontStopAtID or binaryTree.iD == None):
            if binaryTree.right != None:
                s = '(' + fn(binaryTree.left) + ',' + fn(binaryTree.right) + ')' + iD
            else:
                s = '(' + fn(binaryTree.left) + ')' + iD 
        else:
            s = iD
        if includeDistances:
            return s + ':' + distancePrintFn(binaryTree.distance)
        return s
    return fn(binaryTree) + ';'

#########################################################
#########################################################
#########################################################
#functions for postion weight matrices
#########################################################
#########################################################
#########################################################

def pWMRead(fileHandle, alphabetSize=4):
    """
    reads in standard position weight matrix format,
    rows are different types of base, columns are individual residues
    """
    lines = fileHandle.readlines()
    assert len(lines) == alphabetSize
    l = [ [ float(i) ] for i in lines[0].split() ]
    for line in lines[1:]:
        l2 = [ float(i) for i in line.split() ]
        assert len(l) == len(l2)
        for i in xrange(0, len(l)):
            l[i].append(l2[i])
    for i in xrange(0, len(l)):
        j = sum(l[i]) + 0.0
        l[i] = [ k/j for k in l[i] ]
    return l

def pWMWrite(fileHandle, pWM, alphabetSize=4):
    """
    Writes file in standard PWM format, is reverse of pWMParser
    """
    for i in xrange(0, alphabetSize):
        fileHandle.write("%s\n" % ' '.join([ str(pWM[j][i]) for j in xrange(0, len(pWM)) ]))
    


#########################################################
#########################################################
#########################################################
#Cigar/UCSC Chain functions
#########################################################
#########################################################
#########################################################

def _checkSegment(start, end, strand):
    assert start >= 0
    assert start <= end
    assert strand == '+' or strand == '-'
    
class AlignmentOperation:
    def __init__(self, opType, length, score):
        self.type = opType
        self.length = length
        self.score = score
    
    def __eq__(self, op):
        if op == None:
            return False
        return self.type == op.type and self.length == op.length and close(self.score, op.score, 0.0001)
    
    def __str__(self):
        return "Type: %i Length: %i Score: %f" % (self.type, self.length, self.score)

class PairwiseAlignment:
    #A match in both sequences
    PAIRWISE_MATCH = 0
    #A deletion in the query sequence (seq 1)
    PAIRWISE_INDEL_Y = 1
    #An insertion in the query sequence (seq 1)
    PAIRWISE_INDEL_X = 2
    PAIRWISE_PLUS = '+'
    PAIRWISE_MINUS = '-'
    
    def __init__(self, queryID, queryStart, queryEnd, queryStrand, 
                 targetID, targetStart, targetEnd, targetStrand, score, oL):
        _checkSegment(queryStart, queryEnd, queryStrand)
        _checkSegment(targetStart, targetEnd, targetStrand)
        
        self.contig1 = queryID
        self.start1 = queryStart
        self.end1 = queryEnd
        self.strand1 = queryStrand
        self.contig2 = targetID
        self.start2 = targetStart
        self.end2 = targetEnd
        self.strand2 = targetStrand
        self.score = score
        self.operationList = oL
        
        i = sum([ oP.length for oP in oL if oP.type != PairwiseAlignment.PAIRWISE_INDEL_Y ])
        assert i == queryEnd - queryStart #Check alignment is of right length with respect to the query
        
        i = sum([ oP.length for oP in oL if oP.type != PairwiseAlignment.PAIRWISE_INDEL_X ])
        assert i == targetEnd - targetStart #Check alignment is of right length with respect to the target
        
    def __eq__(self, pairwiseAlignment):
        if pairwiseAlignment == None:
            return False
        return self.contig1 == pairwiseAlignment.contig1 and \
        self.start1 == pairwiseAlignment.start1 and \
        self.end1 == pairwiseAlignment.end1 and \
        self.strand1 == pairwiseAlignment.strand1 and \
        self.contig2 == pairwiseAlignment.contig2 and \
        self.start2 == pairwiseAlignment.start2 and \
        self.end2 == pairwiseAlignment.end2 and \
        self.strand2 == pairwiseAlignment.strand2 and \
        close(self.score, pairwiseAlignment.score, 0.001) and \
        self.operationList == pairwiseAlignment.operationList
    
def cigarRead(fileHandle):
    """Reads a list of pairwise alignments into a pairwise alignment structure.
    
    Query and target are reversed!
    """
    #p = re.compile("cigar:\\s+(.+)\\s+([0-9]+)\\s+([0-9]+)\\s+([\\+\\-\\.])\\s+(.+)\\s+([0-9]+)\\s+([0-9]+)\\s+([\\+\\-\\.])\\s+(.+)\\s+(.*)\\s*)*")
    p = re.compile("cigar:\\s+(.+)\\s+([0-9]+)\\s+([0-9]+)\\s+([\\+\\-\\.])\\s+(.+)\\s+([0-9]+)\\s+([0-9]+)\\s+([\\+\\-\\.])\\s+([^\\s]+)(\\s+(.*)\\s*)*")
    line = fileHandle.readline()
    while line != '':
        i = p.match(line)
        if i != None:
            m = i.groups()
            if len(m) == 11:
                l = m[10].split(" ")
                ops = []
                if l != ['']:
                    j = 0
                    while j < len(l):
                        if l[j] == 'M':
                            ops.append(AlignmentOperation(PairwiseAlignment.PAIRWISE_MATCH, int(l[j+1]), 0.0))
                            j += 2
                        elif l[j] == 'D':
                            ops.append(AlignmentOperation(PairwiseAlignment.PAIRWISE_INDEL_X, int(l[j+1]), 0.0)) #a gap in the query
                            j += 2
                        elif l[j] == 'I':
                            ops.append(AlignmentOperation(PairwiseAlignment.PAIRWISE_INDEL_Y, int(l[j+1]), 0.0)) #a gap in the target
                            j += 2
                        elif l[j] == 'X':
                            ops.append(AlignmentOperation(PairwiseAlignment.PAIRWISE_MATCH, int(l[j+1]), float(l[j+2])))
                            j += 3
                        elif l[j] == 'Y':
                            ops.append(AlignmentOperation(PairwiseAlignment.PAIRWISE_INDEL_X, int(l[j+1]), float(l[j+2]))) #a gap in the query
                            j += 3
                        else:
                            assert l[j] == 'Z'
                            ops.append(AlignmentOperation(PairwiseAlignment.PAIRWISE_INDEL_Y, int(l[j+1]), float(l[j+2]))) #a gap in the target
                            j += 3
            else:
                ops = []
            strand1 = m[3]
            strand2 = m[7]
            if strand1 == '+':
                start1, end1 = int(m[1]), int(m[2])
            else:
                end1, start1 = int(m[1]), int(m[2])
            if strand2 == '+':
                start2, end2 = int(m[5]), int(m[6])
            else:
                end2, start2 = int(m[5]), int(m[6])
            yield PairwiseAlignment(m[4], start2, end2, strand2, m[0], start1, end1, strand1, float(m[8]), ops)
        line = fileHandle.readline()

def cigarWrite(fileHandle, pairwiseAlignment, withProbs=True):
    """Writes out the pairwiseAlignment to the file stream.
    
    Query and target are reversed from normal order.
    """
    if len(pairwiseAlignment.operationList) == 0:
        logger.info("Writing zero length pairwiseAlignment to file!")
        
    if pairwiseAlignment.strand1 == '+':
        start1 = pairwiseAlignment.start1
        end1 = pairwiseAlignment.end1
    else:
        start1 = pairwiseAlignment.end1
        end1 = pairwiseAlignment.start1
    
    if pairwiseAlignment.strand2 == '+':
        start2 = pairwiseAlignment.start2
        end2 = pairwiseAlignment.end2
    else:
        start2 = pairwiseAlignment.end2
        end2 = pairwiseAlignment.start2
        
    fileHandle.write("cigar: %s %i %i %s %s %i %i %s %f" % (pairwiseAlignment.contig2, start2, end2, pairwiseAlignment.strand2,\
                                                            pairwiseAlignment.contig1, start1, end1, pairwiseAlignment.strand1,\
                                                            pairwiseAlignment.score))
    if withProbs == True:
        hashMap = { PairwiseAlignment.PAIRWISE_INDEL_Y:'Z',PairwiseAlignment.PAIRWISE_INDEL_X:'Y', PairwiseAlignment.PAIRWISE_MATCH:'X' }
        for op in pairwiseAlignment.operationList:
            fileHandle.write(' %s %i %f' % (hashMap[op.type], op.length, op.score))
    else:
        hashMap = { PairwiseAlignment.PAIRWISE_INDEL_Y:'I',PairwiseAlignment.PAIRWISE_INDEL_X:'D', PairwiseAlignment.PAIRWISE_MATCH:'M' }
        for op in pairwiseAlignment.operationList:
            fileHandle.write(' %s %i' % (hashMap[op.type], op.length))
    fileHandle.write("\n")
    
def _getRandomSegment():
    contig = random.choice([ "one", "two", "three", "four" ])
    start = random.choice(xrange(0, 10000))
    end = start + random.choice(xrange(0, 1000))
    strand = random.choice([ '+', '-' ])
    return contig, start, end, strand

def getRandomOperationList(xLength, yLength, operationMaxLength=100):
    assert operationMaxLength >= 1
    operationList = []
    while xLength > 0 or yLength > 0:
        opType = random.choice([ PairwiseAlignment.PAIRWISE_INDEL_Y, PairwiseAlignment.PAIRWISE_INDEL_X, PairwiseAlignment.PAIRWISE_MATCH ])
        if operationMaxLength == 1:
            length = 1
        else:
            length = random.choice(xrange(1, operationMaxLength))
        if opType != PairwiseAlignment.PAIRWISE_INDEL_Y and xLength - length < 0:
            continue
        if opType != PairwiseAlignment.PAIRWISE_INDEL_X and yLength - length < 0:
            continue
        if opType != PairwiseAlignment.PAIRWISE_INDEL_Y:
            xLength -= length
        if opType != PairwiseAlignment.PAIRWISE_INDEL_X:
            yLength -= length    
        operationList.append(AlignmentOperation(opType, length, random.random()))
        assert xLength >= 0 and yLength >= 0
    return operationList
        
def getRandomPairwiseAlignment():
    """Gets a random pairwiseAlignment.
    """
    i, j, k, l = _getRandomSegment()
    m, n, o, p = _getRandomSegment()
    score = random.choice(xrange(-1000, 1000))
    return PairwiseAlignment(i, j, k, l, m, n, o, p, score, getRandomOperationList(k - j, o - n))

#########################################################
#########################################################
#########################################################
#Graph viz functions.
#########################################################
#########################################################
#########################################################

def addNodeToGraph(nodeName, graphFileHandle, label, width=0.3, height=0.3, shape="circle", colour="black", fontsize=14):
    """Adds a node to the graph.
    """
    graphFileHandle.write("node[width=%s,height=%s,shape=%s,colour=%s,fontsize=%s];\n" % (width, height, shape, colour, fontsize))
    graphFileHandle.write("%s [label=\"%s\"];\n" % (nodeName, label))

def addEdgeToGraph(parentNodeName, childNodeName, graphFileHandle, colour="black", length="10", weight="1", dir="none"):
    """Links two nodes in the graph together.
    """
    graphFileHandle.write("edge[color=%s,len=%s,weight=%s,dir=%s];\n" % (colour, length, weight, dir))
    graphFileHandle.write("%s -- %s;\n" % (parentNodeName, childNodeName))

def setupGraphFile(graphFileHandle):
    """Sets up the dot file.
    """
    graphFileHandle.write("graph G {\n")
    graphFileHandle.write("overlap=false\n")
    logger.info("Starting to write the graph")

def finishGraphFile(graphFileHandle):
    """Finishes up the dot file.
    """
    graphFileHandle.write("}\n")
    logger.info("Finished writing the graph")
    
def runGraphViz(graphFile, outputFile, command="dot"):
    """Runs graphviz.
    """
    system("%s -Tpdf %s > %s" % (command, graphFile, outputFile))


def main():
    pass

def _test():
    import doctest      
    return doctest.testmod()

if __name__ == '__main__':
    _test()
    main()
