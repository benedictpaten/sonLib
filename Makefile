include include.mk

all : cP

cP:
	cd C && make all

clean :
	cd C && make clean

test :
	PYTHONPATH=.. PATH=../../bin:$$PATH python allTests.py --testLength=SHORT --logDebug
