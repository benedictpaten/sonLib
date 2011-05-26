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
else
    cxx = gcc -std=c99
    cpp = g++ 
# -Wno-unused-result
endif



#Release compiler flags
cflags_opt = -O3 -g -Wall -Werror --pedantic -funroll-loops -lm

#Debug flags (slow)
cflags_dbg = -Wall -Werror --pedantic -g -fno-inline -DBEN_DEBUG -lm

#Ultra Debug flags (really slow)
cflags_ultraDbg = -Wall -Werror --pedantic -g -fno-inline -DBEN_DEBUG -BEN_ULTRA_DEBUG -lm

#Profile flags
cflags_prof = -Wall -Werror --pedantic -pg -O3 -g -lm

#  for cpp code: don't use pedantic, or Werror
cppflags = -g -Wall -funroll-loops -lm 

#Flags to use
cflags = ${cflags_opt} 

# location of Tokyo cabinet
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
endif

# location of Tokyo Tyrant
ifneq ($(wildcard /hive/groups/recon/local/include/tcrdb.h),)
   # hgwdev hive install
   ttPrefix = /hive/groups/recon/local
   tokyoTyrantIncl = -I${ttPrefix}/include -DHAVE_TOKYO_TYRANT=1
   tokyoTyrantLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -ltokyotyrant -lz -lbz2 -lpthread -lm
else ifneq ($(wildcard /opt/local/include/tcrdb.h),)
   # OS/X with TC installed from MacPorts
   ttPrefix = /opt/local
   tokyoTyrantIncl = -I${ttPrefix}/include -DHAVE_TOKYO_TYRANT=1
   tokyoTyrantLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -ltokyotyrant -lz -lbz2 -lpthread -lm
else ifneq ($(wildcard /usr/local/include/tcrdb.h),)
   # /usr/local install (FreeBSD, etc)
   ttPrefix = /usr/local
   tokyoTyrantIncl = -I${ttPrefix}/include -DHAVE_TOKYO_TYRANT=1
   tokyoTyrantLib = -L${ttPrefix}/lib -Wl,-rpath,${ttPrefix}/lib -ltokyotyrant -lz -lbz2 -lpthread -lm
endif

# location of Kyoto Tycoon
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

# location of mysql
ifeq ($(shell mysql_config --version >/dev/null 2>&1 && echo ok),ok)
    mysqlIncl = $(shell mysql_config --include) -DHAVE_MYSQL=1
    mysqlLibs = $(shell mysql_config --libs)
endif

# location of PostgreSQL
#### just disable this for now, since we are having problems with not
#### having postgres share libraries on the cluster.
# ifeq ($(shell pg_config --version >/dev/null 2>&1 && echo ok),ok)
#     #pgsqlIncl = -I$(shell pg_config --includedir) -DHAVE_POSTGRESQL=1
#     #pgsqlLibs = $(shell pg_config --ldflags) -lpq
# endif

dblibs = ${tokyoCabinetLib} ${tokyoTyrantLib} ${kyotoTycoonLib} ${mysqlLibs} ${pgsqlLibs} -lz

