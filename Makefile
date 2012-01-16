-include include.mk
binPath = ./bin

.PHONY: all clean cP cP.clean externalToolsP.clean test 

all : cP ${binPath}/sonLib_daemonize.py

clean : cP.clean externalToolsP.clean
	rm include.mk ${binPath}/sonLib_daemonize.py

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
	cp $^ $@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@

include.mk: include.mk.tmpl
	cp $^ $@.tmp
	mv $@.tmp $@
