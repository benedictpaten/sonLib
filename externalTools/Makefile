include ../include.mk
.PHONY: all clean cutestM cutestM.clean quicktreeM.clean spimapM.clean

libPath = ../lib

# We only use a few files from spimap to avoid the GSL dependency
SPIMAPSRCS = spimap/src/Tree.cpp spimap/src/phylogeny.cpp spimap/src/Sequences.cpp spimap/src/parsing.cpp spimap/src/logging.cpp spimap/src/seq.cpp
SPIMAPOBJS = $(SPIMAPSRCS:.cpp=.o)

all : cutestM quicktreeM ${libPath}/spimapRecon.a

clean : cutestM.clean quicktreeM.clean

$(SPIMAPOBJS): %.o: %.cpp
# UNDEBUG is required!!
	${cpp} -c $(cppflags) -UNDEBUG -o $@ $<

${libPath}/spimapRecon.a: $(SPIMAPOBJS)
	ar rc spimapRecon.a $(SPIMAPOBJS)
	ranlib spimapRecon.a
	mv spimapRecon.a ${libPath}

cutestM :
	cd cutest && make all

cutestM.clean :
	cd cutest && make clean

quicktreeM :
	cd quicktree_1.1 && make all

quicktreeM.clean :
	cd quicktree_1.1 && make clean

spimapM.clean :
	rm $(SPIMAPOBJS)
	rm ${libPath}/spimapRecon.a
