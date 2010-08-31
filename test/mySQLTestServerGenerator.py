import os
import time
import sys
import subprocess

from sonLib.bioio import system

class MySqlServer:
    def __init__(self, datadir="./", host="127.0.0.1", port="30000", user="root", password=""):
        self.datadir = datadir
        self.host = host
        self.port = port
        self.user = user
        self.password = password
        self.process = None
    
    def getDatadir(self):
        return self.datadir
    
    def getHost(self):
        return self.host
    
    def getPort(self):
        return self.port
    
    def setPort(self, port):
        self.port = port
    
    def getUser(self):
        return self.user
    
    def getPassword(self):
        return self.password
    
    def addDatabase(self, databaseName):
        ("mysql --host %s --port %s --user %s --password %s -e CREATE DATABASE %s")
    
    def start(self):
        if self.started():
            raise RuntimeError("The server is already running")
        
        #Create the conf file
        fileHandle = open(os.path.join(self.getLocation(), "sandbox.cnf"), 'w')
        fileHandle.write("[mysqld]\ndatadir=%s\nport=%s\nbind-address=%s\nsocket=%s/socket\nconsole\n\
\n[client]\nport=%s\nsocket=%s/socket\nhost=%s\nuser=%s\n" % \
        self.getDatadir(), self.getPort(), self.getHost(), self.getDatadir(),
        self.getPort(), self.getDatadir(), self.getHost(), self.getUser())
        fileHandle.close()
        
        #Now start the subprocess
        system("mysqld --defaults-file=%s/sandbox.cnf > ${datadir}/mysqld.log 2>&1 &" % (self.getDatadir(), self.getDatadir()))
        while not os.path.exists("%s/socket" % self.getDatadir()):
            time.sleep(1) #We are waiting for the server to get going
        self.running = 1
        self.pid = 0
    
    def started(self):
        return self.process != None
        
    def stop(self):
        if not self.started():
            raise RuntimeError("The server is not running")
        system("kill %s" % self.process.kill())
        self.running = 0
    
    def removeFromDisk(self):
        if self.started():
            raise RuntimeError("The server is still running, stop it first")
        system("rm -rf %s" % self.getDatadir())

def generateTestMysqlServer(datadir="./", trys=100):
    port = 30000
    while trys > 0:
        try:
            return MySqlServer(datadir=datadir, port=port)
        except RuntimeError:
            sys.stderr.write("Tried to open a test mysql server at %s with port %s" % (datadir, port))
            port += 1
        trys -= 1
    raise RuntimeError("Tried too many times to open a mysql server")
