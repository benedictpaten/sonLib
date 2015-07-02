# we do specific stuff for specific host for now.
HOSTNAME = $(shell hostname)
MACH = $(shell uname -m)
SYS =  $(shell uname -s)

#C compiler
ifeq (${SYS},FreeBSD)
    # default FreeBSD gcc (4.2.1) has warning bug
    #cxx = gcc46 -std=c99 -Wno-unused-but-set-variable
    cxx = gcc34 -std=c99 -Wno-unused-but-set-variable
    cpp = g++
else ifeq ($(SYS),Darwin) #This is to deal with the Mavericks replacing gcc with clang fully
	cxx = clang -std=c99 
	cpp = clang++ 
else
    cxx = gcc -std=c99
    cpp = g++ 
endif

# -Wno-unused-result

# Compiler flags.
# DO NOT put static library -l options here. Those must be specified *after*
# linker input files. See <http://stackoverflow.com/a/8266512/402891>.

#Release compiler flags
cflags_opt = -O3 -g -Wall --pedantic -funroll-loops -DNDEBUG 
#-fopenmp
cppflags_opt = -O3 -g -Wall -funroll-loops -DNDEBUG

#Debug flags (slow)
cflags_dbg = -Wall -Werror --pedantic -g -fno-inline
cppflags_dbg = -Wall -g -O0 -fno-inline 

#Ultra Debug flags (really slow)
cflags_ultraDbg = -Wall -Werror --pedantic -g -fno-inline

#Profile flags
cflags_prof = -Wall -Werror --pedantic -pg -O3 -g

#for cpp code: don't use pedantic, or Werror
cppflags = ${cppflags_opt} 

#Flags to use
cflags = ${cflags_dbg} 

# location of Tokyo cabinet
ifndef tokyoCabinetLib
ifneq ($(wildcard /hive/groups/recon/local/include/tcbdb.h),)
   # hgwdev hive install
   tcPrefix = /hive/groups/recon/local
   tokyoCabinetIncl = -I${tcPrefix}/include -DHAVE_TOKYO_CABINET=1
   tokyoCabinetLib = -L${tcPrefix}/lib -Wl,-rpath,${tcPrefix}/lib -ltokyocabinet -lz -lbz2 -lpthread -lm
else ifneq ($(wildcard /opt/local/include/tcbdb.h),)
   # OS/X with TC installed from MacPorts
   tcPrefix = /opt/local
   tokyoCabinetIncl = -I${tcPrefix}/include -DHAVE_TOKYO_CABINET=1
   tokyoCabinetLib = -L${tcPrefix}/lib -Wl,-rpath,${tcPrefix}/lib -ltokyocabinet -lz -lbz2 -lpthread -lm
else ifneq ($(wildcard /usr/local/include/tcbdb.h),)
   # /usr/local install (FreeBSD, etc)
   tcPrefix = /usr/local
   tokyoCabinetIncl = -I${tcPrefix}/include -DHAVE_TOKYO_CABINET=1
   tokyoCabinetLib = -L${tcPrefix}/lib -Wl,-rpath,${tcPrefix}/lib -ltokyocabinet -lz -lbz2 -lpthread -lm
else ifneq ($(wildcard /usr/include/tcbdb.h),)
   # /usr install (Ubuntu, and probably most Debain-based systems)
   tcPrefix = /usr
   tokyoCabinetIncl = -I${tcPrefix}/include -DHAVE_TOKYO_CABINET=1
   tokyoCabinetLib = -L${tcPrefix}/lib -Wl,-rpath,${tcPrefix}/lib -ltokyocabinet -lz -lbz2 -lpthread -lm
endif
endif

# location of Kyoto Tycoon
ifndef kyotoTycoonLib
ifneq ($(wildcard /hive/groups/recon/local/include/ktcommon.h),)
   # hgwdev hive install
   ttPrefix = /hive/groups/recon/local
   kyotoTycoonIncl = -I${ttPrefix}/include -DHAVE_KYOTO_TYCOON=1
   kyotoTycoonLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -lkyototycoon -lkyotocabinet -lz -lbz2 -lpthread -lm -lstdc++
else ifneq ($(wildcard /opt/local/include/ktcommon.h),)
   # OS/X with TC installed from MacPorts
   ttPrefix = /opt/local
   kyotoTycoonIncl = -I${ttPrefix}/include -DHAVE_KYOTO_TYCOON=1 
   kyotoTycoonLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -lkyototycoon -lkyotocabinet -lz -lbz2 -lpthread -lm -lstdc++ 
else ifneq ($(wildcard /usr/local/include/ktcommon.h),)
   # /usr/local install (FreeBSD, etc)
   ttPrefix = /usr/local
   kyotoTycoonIncl = -I${ttPrefix}/include -DHAVE_KYOTO_TYCOON=1 
   kyotoTycoonLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -lkyototycoon -lkyotocabinet -lz -lbz2 -lpthread -lm -lstdc++
endif
endif

# location of mysql
#ifndef mysqlLibs
#ifeq ($(shell mysql_config --version >/dev/null 2>&1 && echo ok),ok)
#    mysqlIncl = $(shell mysql_config --include) -DHAVE_MYSQL=1
#    mysqlLibs = $(shell mysql_config --libs)
#endif
#endif

dblibs = ${tokyoCabinetLib} ${kyotoTycoonLib} ${mysqlLibs} -lz -lm

