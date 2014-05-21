include ../include.mk
binPath = ../bin
libPath = ../lib

libSources = impl/*.c
cppSources = impl/*.cpp
libHeaders = $(wildcard inc/*.h)
libInternalHeaders = impl/*.h
libTests = tests/sonLib*.c tests/st*

testProgs = ${binPath}/sonLibTests ${binPath}/sonLib_kvDatabaseTest ${binPath}/sonLib_cigarTest ${binPath}/sonLib_fastaCTest

cflags += ${tokyoCabinetIncl} ${kyotoTycoonIncl} ${tokyoTyrantIncl} ${mysqlIncl} ${pgsqlIncl}
cppflags += ${kyotoTycoonIncl} 

all : ${libPath}/sonLib.a ${testProgs}

clean : 
	rm -rf  ${binPath} ${libPath}

${binPath}/sonLibTests : ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/allTests.c
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -o $@.tmp tests/allTests.c ${libTests} ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs} -lm
	mv $@.tmp $@

${binPath}/sonLib_kvDatabaseTest : ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -I tests -o $@.tmp tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs} -lm
	mv $@.tmp $@

${binPath}/sonLib_cigarTest : tests/cigarsTest.c ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a 
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -o $@.tmp tests/cigarsTest.c ${libPath}/sonLib.a -lm
	mv $@.tmp $@

${binPath}/sonLib_fastaCTest : tests/fastaCTest.c ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath} -o $@.tmp tests/fastaCTest.c ${libPath}/sonLib.a -lm
	mv $@.tmp $@

${binPath}/kt_connect_test : tests/kt_connect_test.cpp ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a
	@mkdir -p $(dir $@)
	${cpp} ${cppflags} -I inc -I ${libPath}/ -o $@.tmp tests/kt_connect_test.cpp ${libPath}/sonLib.a ${dblibs} -lm
	mv $@.tmp $@

${libPath}/%.h: inc/%.h
	@mkdir -p $(dir $@)
	cp $< $@.tmp
	mv $@.tmp $@

${libPath}/sonLib.a : ${libSources} ${cppSources} $(foreach l, ${libHeaders}, ${libPath}/$(notdir $l)) ${libInternalHeaders}
	@mkdir -p $(dir $@)
	${cxx} ${cflags} -I inc -I ${libPath}/ -c ${libSources}
	${cpp} ${cppflags} -I inc -I ${libPath}/ -c ${cppSources}
	ar rc sonLib.a *.o
	ranlib sonLib.a 
	rm *.o
	mv sonLib.a ${libPath}/

test:
	python allTests.py --testLength=SHORT --logLevel CRITICAL
