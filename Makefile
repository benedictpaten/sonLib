include ../../include.mk
binPath = ../../bin
libPath = ../../lib

libSources = avl.c bioioC.c commonC.c fastCMaths.c hashTableC.c hashTableC_itr.c heapC.c substitutionC.c pairwiseAlignment.c sonLib*.c
libHeaders = avl.h bioioC.h commonC.h fastCMaths.h hashTableC.h hashTableC_itr.h heapC.h substitutionC.h pairwiseAlignment.h hashTablePrivateC.h sonLib*.h
cflags += ${tokyoCabinetIncl}

libPlusSources = Argument_helper.cc XMLTools.cc substitutionIO.cc
libPlusHeaders = Argument_helper.h XMLTools.h substitutionIO.h

all : ${libPath}/sonLib.a ${libPath}/sonLibPlus.a ${binPath}/sonLib_binTest ${binPath}/sonLib_cigarsTest ${binPath}/sonLib_fastaCTest eVDM ${binPath}/sonLibTests

clean : 
#Removing test binaries
	rm -f ${binPath}/sonLib_binTest ${binPath}/sonLib_cigarsTest ${binPath}/sonLib_fastaCTest ${binPath}/sonLibTests \
	${libPath}/sonLib.a ${libPath}/avl.h ${libPath}/bioioC.h ${libPath}/chains.h ${libPath}/commonC.h ${libPath}/fastCMaths.h ${libPath}/hashTableC.h ${libPath}/heapC.h ${libPath}/substitutionC.h ${libPath}/pairwiseAlignment.h \
	${libPath}/sonLibPlus.a ${libPath}/Argument_helper.h ${libPath}/XMLTools.h ${libPath}/substitutionIO.h
	cd EVD && make clean

${binPath}/sonLib_binTest : ${libPath}/sonLib.a binTreeTest.c
	${cxx} ${cflags} -I ../sonLib -o ${binPath}/sonLib_binTest binTreeTest.c ${libPath}/sonLib.a

${binPath}/sonLib_cigarsTest : ${libPath}/sonLib.a cigarsTest.c
	${cxx} ${cflags} -I ../sonLib -o ${binPath}/sonLib_cigarsTest cigarsTest.c ${libPath}/sonLib.a

${binPath}/sonLib_fastaCTest : ${libPath}/sonLib.a fastaCTest.c
	${cxx} ${cflags} -I ../sonLib -o ${binPath}/sonLib_fastaCTest fastaCTest.c ${libPath}/sonLib.a
	
${binPath}/sonLibTests : ${libSources} ${libHeaders} allTests.c ${libPath}/sonLib.a ${libPath}/cuTest.a
	${cxx} ${cflags} -I ${libPath} -o ${binPath}/sonLibTests allTests.c ${libPath}/sonLib.a ${libPath}/cuTest.a ${tokyoCabinetLib}


eVDM : ${libPath}/sonLib.a ${libPath}/sonLibPlus.a
#Making EVD 
	cd EVD && make all

${libPath}/sonLib.a : ${libSources} ${libHeaders}
	${cxx} ${cflags} -I ${libPath}/ -c ${libSources}
	ar rc sonLib.a *.o
	ranlib sonLib.a 
	rm *.o
	mv sonLib.a ${libPath}/
	cp ${libHeaders} ${libPath}/

${libPath}/sonLibPlus.a : ${libPlusSources} ${libPlusHeaders}
	${cxx} ${cflags} -I ${libPath}/ -c ${libPlusSources}
	ar rc sonLibPlus.a *.o
	ranlib sonLibPlus.a
	rm *.o
	mv sonLibPlus.a ${libPath}/
	cp ${libPlusHeaders} ${libPath}/
