include ../include.mk
.PHONY: all clean cutestM cutestM.clean quicktreeM.clean

libPath = ../lib

all : cutestM quicktreeM

clean : cutestM.clean quicktreeM.clean

cutestM :
	+cd cutest && $(MAKE) all

cutestM.clean :
	+cd cutest && $(MAKE) clean

quicktreeM :
	+cd quicktree_1.1 && $(MAKE) all CC="${cxx}"

quicktreeM.clean :
	+cd quicktree_1.1 && $(MAKE) clean
