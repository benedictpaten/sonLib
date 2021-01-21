include include.mk
binPath = ./bin

.PHONY: all clean cP cP.clean externalToolsP.clean test 

all : cP ${binPath}/sonLib_daemonize.py

clean : cP.clean externalToolsP.clean
	rm -f ${binPath}/sonLib_daemonize.py

cP: externalToolsP
	cd C && $(MAKE) all

externalToolsP:
	cd externalTools && $(MAKE) all

cP.clean :
	cd C && $(MAKE) clean

externalToolsP.clean :
	cd externalTools && $(MAKE) clean

test : all
	PYTHONPATH=.. PATH=../../bin:$$PATH python3 allTests.py --testLength=SHORT --logLevel=CRITICAL

${binPath}/sonLib_daemonize.py : sonLib_daemonize.py cP
	cp sonLib_daemonize.py ${binPath}/sonLib_daemonize.py
	chmod +x ${binPath}/sonLib_daemonize.py
