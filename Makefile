include include.mk

.PHONY: all clean cP cP.clean externalToolsP.clean test 

all : cP

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
	PYTHONPATH=.. PATH=../../bin:$$PATH python allTests.py --testLength=SHORT --logDebug
