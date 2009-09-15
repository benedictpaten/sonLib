#!/usr/bin/env python

import sys
import os
import re
import time
import sqlite3

from sonLib.bioio import getTempFile as getTempFileGlobal
from sonLib.bioio import getTempDirectory
from sonLib.bioio import logger
from sonLib.bioio import readFirstLine
from sonLib.bioio import padWord

#########################################################
#########################################################
#########################################################
#database functions
#########################################################
#########################################################
#########################################################

def getTempFile():
    return getTempFileGlobal(".resultsManager")

def convertTabSeperatedFileToDB(inputFile, outputFile, tableName):
    #Generates a db from a tab file
    #The first line is the program name, the remaining lines are real valued.
    fields = readFirstLine(inputFile).split()
    connection = sqlite3.connect(outputFile)
    cursor = connection.cursor()
    i = " real, ".join(fields[1:]) + " real"
    cmd = "create table %s (%s text, %s)" % (tableName, fields[0], i)
    logger.info("Building table with SQL command: %s" % cmd)
    cursor.execute(cmd)
    inputFile = open(inputFile, 'r')
    for line in inputFile.readlines()[1:]:
        i = line.split()
        j = ", ".join(i[1:])
        cmd = "insert into %s values ('%s', %s)" % (tableName, i[0], j)
        logger.info("Inserting data using :" + cmd)
        cursor.execute(cmd)
    connection.commit()
    cursor.close()
    inputFile.close()
    logger.info("Created table okay")
    return fields

def buildAggregateTables(dataBase, tableName, fields):
    buildAggregateTable(dataBase, tableName, fields, "avg")
    #buildAggregateTable(dataBase, tableName, fields, "var")
    buildAggregateTable(dataBase, tableName, fields, "min")
    buildAggregateTable(dataBase, tableName, fields, "max")

def buildAggregateTable(dataBase, tableName, fields, aggregateFnName):
    #Get different program names
    connection = sqlite3.connect(dataBase)
    cursor = connection.cursor()
    i = " real, ".join(fields[1:]) + " real"
    cmd = "create table %s_%s (%s text, %s)" % (aggregateFnName, tableName, fields[0], i)
    logger.info("Building table with SQL command: %s" % cmd)
    cursor.execute(cmd)
    j = ", ".join([ ('%s(' % aggregateFnName) + str(i) + ')' for i in fields[1:] ])
    for key in cursor.execute("select distinct %s from %s" % (fields[0], tableName)):
        cmd = 'select %s from %s where %s="%s"' % (j, tableName, fields[0], key[0])
        logger.info("Getting from table: %s" % cmd)
        cursor2 = connection.cursor()
        for row in cursor2.execute(cmd): 
            k = " , ".join([ str(k) for k in row ])
            cmd = 'insert into %s_%s values ("%s", %s)' % (aggregateFnName, tableName, key[0], k)
            logger.info("Inserting data: " + cmd)
            cursor3 = connection.cursor()
            cursor3.execute(cmd)
            cursor3.close()
        cursor2.close()
    cursor.close()
    connection.commit()

def writeAggregateTables(dataBase, tableName, fields, outputFile):
    connection = sqlite3.connect(dataBase)
    cursor = connection.cursor()
    i = open(outputFile, 'w')
    for functionName in [ "avg", "max", "min" ]:
        i.write("Function: %s\n" % functionName)
        i.write("\t".join([ padWord(j) for j in fields ]) + "\n")
        for row in cursor.execute("select * from %s_%s" % (functionName, tableName)):
            i.write("\t".join([ padWord(str(j)) for j in row ]) + "\n")
    i.close()
    connection.commit()

def logDataBaseTables(dataBase):
    connection = sqlite3.connect(dataBase)
    logger.info("Printing data-base")
    cursor = connection.cursor()
    for tableName in cursor.execute("select name from sqlite_master where type='table'"):
        cursor2 = connection.cursor()
        logger.info("Printing table: %s" % tableName[0])
        for row in cursor2.execute("select * from %s" % tableName[0]):
            logger.info("Row: %s" % " ".join([ str(i) for i in row ]))
        cursor2.close()
    cursor.close()
    connection.commit()

def buildDB(outputFile, tableName):
    tempDB = getTempFile()
    fields = convertTabSeperatedFileToDB(outputFile, tempDB, tableName)
    buildAggregateTables(tempDB, tableName, fields)
    logDataBaseTables(tempDB)
    return tempDB, fields

def aggregateOutput(outputFile, tableName, aggregatedOutputFile):
    tempDB, fields = buildDB(outputFile, tableName)
    #fields = convertTabSeperatedFileToDB(outputFile, tempDB, tableName)
    #buildAggregateTables(tempDB, tableName, fields)
    #logDataBaseTables(tempDB)
    writeAggregateTables(tempDB, tableName, fields, aggregatedOutputFile)
    os.remove(tempDB)