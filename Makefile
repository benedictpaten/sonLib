include ../../include.mk
binPath = ../../bin
libPath = ../../lib

libSources = impl/*.c
libHeaders = inc/*.h
libInternalHeaders = impl/*.h
libTests = tests/sonLib*.c

all : ${libPath}/sonLib.a ${binPath}/sonLibTests

clean : 
	rm -f  ${binPath}/sonLibTests ${libPath}/sonLib*.h ${libPath}/sonLib.a ${libPath}/avl.h ${libPath}/bioioC.h ${libPath}/commonC.h ${libPath}/fastCMaths.h ${libPath}/hashTableC.h ${libPath}/pairwiseAlignment.h

${binPath}/sonLibTests : ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/allTests.c
	${cxx} ${cflags} -I inc -I ${libPath} -o ${binPath}/sonLibTests tests/allTests.c ${libTests} ${libPath}/sonLib.a ${libPath}/cuTest.a

${libPath}/sonLib.a : ${libSources} ${libHeaders} ${libInternalHeaders}
	cp ${libHeaders} ${libPath}/
	${cxx} ${cflags} -I inc -I ${libPath}/ -c ${libSources}
	ar rc sonLib.a *.o
	ranlib sonLib.a 
	rm *.o
	mv sonLib.a ${libPath}/
