# we do specific stuff for specific host for now.
HOSTNAME = $(shell hostname)
MACH = $(shell uname -m)
SYS =  $(shell uname -s)

CC ?=gcc
#C compiler. FIXME: for some reason the cxx variable is used, which
#typically means C++ compiler.
ifeq (${CC},cc)
  ifeq (${SYS},FreeBSD)
    # default FreeBSD gcc (4.2.1) has warning bug
    #cxx = gcc46 -std=c99 -Wno-unused-but-set-variable
    cxx = gcc34 -std=c99 -Wno-unused-but-set-variable
  else ifeq ($(SYS),Darwin) #This is to deal with the Mavericks replacing gcc with clang fully
	cxx = clang -std=c99 
  else
    cxx = ${CC} -std=c99
  endif
else
  cxx = ${CC} -std=c99
endif

# C++ compiler. FIXME: for some reason the cpp variable is used, which
# typically refers to the C preprocessor.
ifndef CXX
  ifeq (${SYS},FreeBSD)
    # default FreeBSD gcc (4.2.1) has warning bug
    cpp = g++
  else ifeq ($(SYS),Darwin) #This is to deal with the Mavericks replacing gcc with clang fully
    cpp = clang++
  else
    cpp = g++ 
  endif
else
  cpp = ${CXX} -std=gnu++98
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
cflags_dbg = -Wall -O0 -Werror --pedantic -g -fno-inline -UNDEBUG -Wno-error=unused-result
cppflags_dbg = -Wall -g -O0 -fno-inline -UNDEBUG

#Ultra Debug flags (really slow, checks for memory issues)
cflags_ultraDbg = -Wall -Werror --pedantic -g -O1 -fno-inline -fno-omit-frame-pointer -fsanitize=address
cppflags_ultraDbg = -g -O1 -fno-inline -fno-omit-frame-pointer -fsanitize=address

#Profile flags
cflags_prof = -Wall -Werror --pedantic -pg -O3 -g -Wno-error=unused-result

#Flags to use
ifndef CGL_DEBUG
  cppflags = ${cppflags_opt}
  cflags = ${cflags_opt}
else
  ifeq (${CGL_DEBUG},ultra)
    cppflags = ${cppflags_ultraDbg}
    cflags = ${cflags_ultraDbg}
  else
    cppflags = ${cppflags_dbg}
    cflags = ${cflags_dbg}
  endif
endif
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
else ifneq ($(wildcard /usr/include/ktcommon.h),)
   # /usr install (Ubuntu)
   ttPrefix = /usr
   kyotoTycoonIncl = -I${ttPrefix}/include -DHAVE_KYOTO_TYCOON=1 
   kyotoTycoonLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -lkyototycoon -lkyotocabinet -lz -lbz2 -lpthread -lm -lstdc++
endif
endif

dblibs = ${tokyoCabinetLib} ${kyotoTycoonLib} -lz -lm

