include ../include.mk
binPath = ../bin
libPath = ../lib

libSources = impl/*.c
cppSources = impl/*.cpp
libHeaders = $(wildcard inc/*.h)
libInternalHeaders = impl/*.h
libTests = tests/sonLib*.c tests/st*
quickTreeObjects = ../externalTools/quicktree_1.1/obj/buildtree.o ../externalTools/quicktree_1.1/obj/cluster.o ../externalTools/quicktree_1.1/obj/distancemat.o ../externalTools/quicktree_1.1/obj/options.o ../externalTools/quicktree_1.1/obj/sequence.o ../externalTools/quicktree_1.1/obj/tree.o ../externalTools/quicktree_1.1/obj/util.o
quickTreeLibPath = ../externalTools/quicktree_1.1/include/

testProgs = ${binPath}/sonLibTests ${binPath}/sonLib_kvDatabaseTest ${binPath}/sonLib_cigarTest ${binPath}/sonLib_fastaCTest

cflags += ${tokyoCabinetIncl} ${kyotoTycoonIncl} ${tokyoTyrantIncl} ${mysqlIncl} ${pgsqlIncl} -I${quickTreeLibPath} $(CFLAGS)
cppflags += ${kyotoTycoonIncl}

all : ${libPath}/sonLib.a ${testProgs}

clean : 
	rm -rf  ${binPath} ${libPath}

${binPath}/sonLibTests : ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/allTests.c
	@mkdir -p $(dir $@)
	${cxx} $(LDFLAGS) $(CPPFLAGS) ${cflags} -I inc -I ${libPath} -o $@.tmp tests/allTests.c ${libTests} ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs} -lm -lstdc++ -lpthread
	mv $@.tmp $@

${binPath}/sonLib_kvDatabaseTest : ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a ${libPath}/cuTest.a tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c
	@mkdir -p $(dir $@)
	${cxx} $(LDFLAGS) $(CPPFLAGS) ${cflags} -I inc -I ${libPath} -I tests -o $@.tmp tests/kvDatabaseTest.c tests/kvDatabaseTestCommon.c ${libPath}/sonLib.a ${libPath}/cuTest.a ${dblibs} ${mysqlLibs} -lm
	mv $@.tmp $@

${binPath}/sonLib_cigarTest : tests/cigarsTest.c ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a 
	@mkdir -p $(dir $@)
	${cxx} $(LDFLAGS) $(CPPFLAGS) ${cflags} -I inc -I ${libPath} -o $@.tmp tests/cigarsTest.c ${libPath}/sonLib.a -lm
	mv $@.tmp $@

${binPath}/sonLib_fastaCTest : tests/fastaCTest.c ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a
	@mkdir -p $(dir $@)
	${cxx} $(LDFLAGS) $(CPPFLAGS) ${cflags} -I inc -I ${libPath} -o $@.tmp tests/fastaCTest.c ${libPath}/sonLib.a -lm
	mv $@.tmp $@

${binPath}/kt_connect_test : tests/kt_connect_test.cpp ${libTests} ${libInternalHeaders} ${libPath}/sonLib.a
	@mkdir -p $(dir $@)
	${cpp} $(LDFLAGS) $(CPPFLAGS) ${cppflags} -I inc -I ${libPath}/ -o $@.tmp tests/kt_connect_test.cpp ${libPath}/sonLib.a ${dblibs} -lm
	mv $@.tmp $@

${libPath}/%.h: inc/%.h
	@mkdir -p $(dir $@)
	cp $< $@.tmp
	mv $@.tmp $@

${libPath}/sonLib.a : ${libSources} ${cppSources} $(foreach l, ${libHeaders}, ${libPath}/$(notdir $l)) ${libInternalHeaders}
	@mkdir -p $(dir $@)
	${cxx} $(CPPFLAGS) ${cflags} $(CXXFLAGS) -I inc -I ${libPath}/  -c ${libSources}
	${cpp} $(CPPFLAGS) ${cppflags} $(CXXFLAGS) $(LDFLAGS) -I inc -I ${libPath}/ -c ${cppSources}
	ar rc sonLib.a *.o ${quickTreeObjects}
	ranlib sonLib.a 
	rm *.o
	mv sonLib.a ${libPath}/

test:
	python allTests.py --testLength=SHORT --logLevel CRITICAL
