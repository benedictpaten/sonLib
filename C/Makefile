include ../include.mk
binPath = ../bin
libPath = ../lib

libSources = impl/*.c
cppSources = impl/*.cpp
libHeaders = inc/*.h
libInternalHeaders = impl/*.h
libTests = tests/sonLib*.c

testProgs = ${binPath}/sonLibTests ${binPath}/sonLib_kvDatabaseTest ${binPath}/sonLib_cigarTest ${binPath}/sonLib_fastaCTest

cflags += ${tokyoCabinetIncl} ${kyotoTycoonIncl} ${tokyoTyrantIncl} ${mysqlIncl} ${pgsqlIncl}
cppflags += ${kyotoTycoonIncl} 

all : ${libPath}/sonLib.a ${testProgs}

clean : 
	rm -f  ${testProgs} ${libPath}/sonLib*.h ${libPath}/st*.h ${libPath}/sonLib.a ${libPath}/hashTable* ${libPath}/avl.h ${libPath}/bioioC.h ${libPath}/commonC.h ${libPath}/fastCMaths.h ${libPath}/pairwiseAlignment.h

${binPath}/sonLibTests : ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/allTests.c
	${cxx} ${cflags} -I inc -I ${libPath} -o $@ tests/allTests.c ${libTests} ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs}

${binPath}/sonLib_kvDatabaseTest : ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c
	${cxx} ${cflags} -I inc -I ${libPath} -I tests -o $@ tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs}

${binPath}/sonLib_cigarTest : tests/cigarsTest.c ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a 
	${cxx} ${cflags} -I inc -I ${libPath} -o $@ tests/cigarsTest.c ${libPath}/sonLib.a -lm

${binPath}/sonLib_fastaCTest : tests/fastaCTest.c ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a
	${cxx} ${cflags} -I inc -I ${libPath} -o $@ tests/fastaCTest.c ${libPath}/sonLib.a -lm
 
${libPath}/sonLib.a : ${libSources} ${libHeaders} ${libInternalHeaders}
	cp ${libHeaders} ${libPath}/
	${cxx} ${cflags} -I inc -I ${libPath}/ -c ${libSources}
	${cpp} ${cppflags} -I inc -I ${libPath}/ -c ${cppSources}
	ar rc sonLib.a *.o
	ranlib sonLib.a 
	rm *.o
	mv sonLib.a ${libPath}/

test :
	python allTests.py --testLength=SHORT --logDebug
