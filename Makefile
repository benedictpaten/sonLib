include ../../include.mk
binPath = ../../bin
libPath = ../../lib

all : cP

cP:
	cd C && make all

test :
	python allTests.py --testLength=SHORT --logDebug
