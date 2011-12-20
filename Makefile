include include.mk
binPath = ./bin

.PHONY: all clean cP cP.clean externalToolsP.clean test 

all : cP ${binPath}/sonLib_daemonize.py

clean : cP.clean externalToolsP.clean

cP: externalToolsP
	cd C && make all

externalToolsP:
	cd externalTools && make all

cP.clean :
	cd C && make clean

externalToolsP.clean :
	cd externalTools && make clean

test :
	PYTHONPATH=.. PATH=../../bin:$$PATH python allTests.py --testLength=SHORT --logLevel=CRITICAL

${binPath}/sonLib_daemonize.py : sonLib_daemonize.py
	cp sonLib_daemonize.py ${binPath}/sonLib_daemonize.py
	chmod +x ${binPath}/sonLib_daemonize.py