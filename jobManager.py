#!/usr/bin/env python

import sys
import os
import re
import time

from sonLib.bioio import getTempFile as getTempFileGlobal
from sonLib.bioio import getTempDirectory
from sonLib.bioio import logger
from sonLib.bioio import readFirstLine
from sonLib.bioio import padWord


tempFileNo = 0

def getTempFile(tempDir):
    global tempFileNo
    tempFileNo += 1
    f = tempDir + ("/%s.temp" % tempFileNo)
    i = open(f, 'w')
    i.close()
    return f

def batchScript(worker, programName, dataSet, outputFile, 
                otherArgs, maximumJobNumber=sys.maxint):
    """Runs the unit in batch mode using a slave script to run each job in a data set.
    """
    startTime = time.time()
    logger.info("Starting batch script, arguments: worker %s, collator %s, dataSet %s, outputFile %s, otherArgs %s" %\
                (str(worker), programName, dataSet, outputFile, " ".join([ str(i) for i in otherArgs ])))
    #Copy data set and then unzip and un-tar it.
    tempDir = os.getcwd() + "/temp"
    os.system("mkdir temp")
    logger.info("Working using directory: %s" % tempDir)
    def cleanup():
        if os.system("rm -r %s" % tempDir):
            logger.info("Something went wrong removing temporary directory")
            raise RuntimeError()
        logger.info("Cleaned up")
    if os.system("cp %s %s" % (dataSet, tempDir)):
        cleanup()
        logger.info("Something went wrong copying data set to temp dir")
        raise RuntimeError()
    startDir = os.getcwd()
    #Move to the temp dir to do a few operations of unzipping etc.
    os.chdir(tempDir)
    logger.info("Copied data-set to temp dir")
    dataSet = dataSet.split('/')[-1]
    if os.system("bunzip2 %s" % dataSet):
        cleanup()
        logger.info("Something went wrong unzipping (using bzip2) data set")
        raise RuntimeError()
    assert dataSet[-4:] == '.bz2'
    dataSet = dataSet[:-4]
    logger.info("Unzipped data set: %s" % dataSet)
    if os.system("tar -xf %s" % dataSet):
        cleanup()
        logger.info("Something went wrong extracting data set tar ball")
        cleanup()
        raise RuntimeError()
    logger.info("Extracted tar ball")
    os.remove(dataSet)
    assert dataSet[-4:] == '.tar'
    dataSet = dataSet[:-4]
    #Now move back to the original dir.
    os.chdir(startDir)
    dataSet = tempDir + '/' + dataSet 
    #Now iterate through data-set
    tempFiles = []
    jobsDone = 0
    for line in os.popen("ls -l %s | grep job" % dataSet):
        if jobsDone >= maximumJobNumber:
            break
        jobName = line.split()[-1]
        jobString = dataSet + '/' + jobName
        tempFile = getTempFile(tempDir)
        tempFiles.append(tempFile)
        logger.info("Starting job with temp output file: %s" % tempFile)
        try:
            worker.runJob(jobString, tempFile, otherArgs)
        except Exception:
            raise 
        logger.info("Job issued")
        jobsDone += 1
    #Wait until jobs complete
    logger.info("Going to sleep until jobs finished")
    time.sleep(5)
    while not worker.complete():
        time.sleep(3)
    logger.info("Finished sleeping, jobs all done")
    #Collate files
    tempOutputFile = getTempFile(tempDir)
    for tempFile in tempFiles:
        logger.info("Collating output in tempFile : %s " % tempFile)
        catOutputRemovingExtraHeaders(tempFile, tempOutputFile)
        os.remove(tempFile)
    logger.info("Adding details to global output file")
    addProgramDetailsToOutputFile(programName, tempOutputFile, outputFile)
    os.remove(tempOutputFile)
    logger.info("Finished collating output.")
    #Clean-up
    cleanup()
    logger.info("Finished batch sript, total time taken : %s (seconds)" % (time.time()-startTime))

def runJobRemotely(unit, farmstring, jobString, outputFile, otherArgs):
    unitPath = os.path.abspath(unit.__file__)
    if unitPath[-1] == 'c':
        unitPath = unitPath[:-1]
    cmd = '%s python %s %s %s %s' % \
    (farmstring, unitPath, jobString, outputFile, " ".join([ '"%s"' % i for i in otherArgs ]))
    #farmstring
    logger.info("Going to run farm command: %s" % cmd)
    if os.system(cmd):
        raise RuntimeError()
    
bsubFarmString = 'bsub -P birneygroup -q long -e ~/error/ -R"select[mem>3000] rusage[mem=3000]" -M 3000000'

def localOrRemote(runLocal, localWorker, remoteWorker):
    #Choose slave to run job.
    if runLocal:
        worker=localWorker
        logger.info("Running locally")
    else:
        worker=remoteWorker
        logger.info("Running remotely")
    return worker

def catOutputRemovingExtraHeaders(resultFile, outputFile):
    i = open(outputFile, 'r')
    j = i.readlines()
    i.close()
    if len(j) == 0:
        os.system("cat %s >> %s" % (resultFile, outputFile))
    else:
        outputFile = open(outputFile, 'a')
        i = open(resultFile, 'r')
        j = i.readlines()
        i.close()
        outputFile.write(j[1] + "\n")
        outputFile.close()
    
def bsubJobsComplete():
    logger.info("Seeing if jobs are done")
    return len(os.popen('bjobs').readlines()) == 0

def writeOutputToTabFile(resultList, outputFile):
    #Spitting out results in output file.
    outputFile = open(outputFile, 'w')
    for i, j in resultList:
        outputFile.write('\t%s' % i)
    outputFile.write('\n')
    for i, j in resultList:
        outputFile.write('\t%s' % j)
    outputFile.write('\n')
    outputFile.close()

def addProgramDetailsToOutputFile(programName, tempOutputFile, outputFile):
    #Remove lines from output-file that are to be replaced.
    #Then write out new file with lines added
    try:
        i = open(outputFile, 'r')
        j = i.readlines()
        i.close()
    except IOError:
        j = []
    if len(j) != 0:
        j = j[0:1] + [ k for k in j[1:] if k.split()[0] != programName ]
    else:
        j = [ "programName\t" + readFirstLine(tempOutputFile) ]
    i = open(tempOutputFile, 'r')
    j2 = i.readlines()[1:]
    i.close()
    i = open(outputFile, 'w')
    i.write('\t'.join(j[0].split()) + '\n')
    fieldNo = len(j[0].split())
    for k in j[1:]:
        i.write('\t'.join(k.split()) + '\n')
    for k in j2:
        if fieldNo == len(k.split())+1:
            i.write('\t'.join([ programName ] + k.split()) + '\n')
    i.close()

def parameterFn(parameters):
    l = [ "" ]
    for parameterName, start, end, step in parameters:
        l2 = []
        for i in l:
            j = start
            while j <= end:
                l2.append(i + (" -%s %s " % (parameterName, j)))
                j += step
        l = l2
    return [ ("".join(i.split()), i) for i in l ]

