include ../../../include.mk
binPath = ../../../bin
libPath = ../../../lib

all : ${libPath}/eVDLib.a ${binPath}/sonLib_histogramTest ${binPath}/sonLib_eVDScript
	
${binPath}/sonLib_histogramTest : histogramTest.c histogram.h histogram.c
	${cxx} ${cflags}  -o ${binPath}/sonLib_histogramTest histogramTest.c histogram.c 
	
${binPath}/sonLib_eVDScript : eVDScript.cc eVD.cc eVD.h histogram.h histogram.c ${libPath}/sonLib.a ${libPath}/sonLibPlus.a ${libPath}/xmlLib.a
	${cyy} ${cpflags} -I ${libPath} -o ${binPath}/sonLib_eVDScript eVDScript.cc eVD.cc histogram.c ${libPath}/sonLib.a ${libPath}/sonLibPlus.a ${libPath}/xmlLib.a

clean :
	rm -f ${binPath}/sonLib_histogramTest
	rm -f ${binPath}/sonLib_eVDScript
	rm -f ${libPath}/histogram.h
	rm -f ${libPath}/eVD.h
	rm -f ${libPath}/eVDLib.a


${libPath}/eVDLib.a : eVD.cc eVD.h histogram.c histogram.h
	${cyy} ${cflags} -I ${libPath}/ -c eVD.cc histogram.c
	ar rc eVDLib.a *.o
	ranlib eVDLib.a
	rm *.o
	mv eVDLib.a ${libPath}/
	cp histogram.h eVD.h ${libPath}/
