include include.mk

all : cP

cP: externalToolsP
	cd C && make all
	
externalToolsP:
	cd externalTools && make all

clean :
	cd C && make clean

test :
	PYTHONPATH=.. PATH=../../bin:$$PATH python allTests.py --testLength=SHORT --logDebug
