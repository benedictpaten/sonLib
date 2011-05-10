include ../include.mk
binPath = ../bin
libPath = ../lib

libSources = impl/*.c
libHeaders = $(wildcard inc/*.h)
libInternalHeaders = impl/*.h
libTests = tests/sonLib*.c

testProgs = ${binPath}/sonLibTests ${binPath}/sonLib_kvDatabaseTest ${binPath}/sonLib_kvDatabaseTransactionTest ${binPath}/sonLib_cigarTest ${binPath}/sonLib_fastaCTest

cflags += ${tokyoCabinetIncl} ${tokyoTyrantIncl} ${mysqlIncl} ${pgsqlIncl}

all : ${libPath}/sonLib.a ${testProgs}

clean : 
	rm -rf  ${binPath} ${libPath}

${binPath}/sonLibTests : ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/allTests.c
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -o $@ tests/allTests.c ${libTests} ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs}

${binPath}/sonLib_kvDatabaseTest : ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -I tests -o $@ tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs}

${binPath}/sonLib_kvDatabaseTransactionTest : ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a tests/kvDatabaseTransactionTest.c tests/kvDatabaseTestCommon.c
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -I tests -o $@ tests/kvDatabaseTransactionTest.c tests/kvDatabaseTestCommon.c ${libPath}/sonLib.a ${dblibs} ${mysqlLibs}

${binPath}/sonLib_cigarTest : tests/cigarsTest.c ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a 
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -o $@ tests/cigarsTest.c ${libPath}/sonLib.a -lm

${binPath}/sonLib_fastaCTest : tests/fastaCTest.c ${libTests} ${libSources} ${libHeaders} ${libInternalHeaders} ${libPath}/sonLib.a
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -o $@ tests/fastaCTest.c ${libPath}/sonLib.a -lm

${libPath}/%.h: inc/%.h
	@mkdir -p $(dir $@)
	cp $< $@.tmp
	mv $@.tmp $@

${libPath}/sonLib.a : ${libSources} $(foreach l, ${libHeaders}, ${libPath}/$(notdir $l)) ${libInternalHeaders}
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath}/ -c ${libSources}
	ar rc sonLib.a *.o
	ranlib sonLib.a 
	rm *.o
	mv sonLib.a ${libPath}/

test :
	python allTests.py --testLength=SHORT --logDebug
