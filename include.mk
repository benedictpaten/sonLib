# we do specific stuff for specific host for now.
HOSTNAME = $(shell hostname)
MACH = $(shell uname -m)
SYS =  $(shell uname -s)

#C compiler
ifeq (${SYS},FreeBSD)
    # default FreeBSD gcc (4.2.1) has warning bug
    #cxx = gcc46 -std=c99 -Wno-unused-but-set-variable
    cxx = gcc34 -std=c99 -Wno-unused-but-set-variable
else
    cxx = gcc -std=c99
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

#Flags to use
cflags = ${cflags_opt} 
#cflags = ${cflags_dbg}

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

dblibs = ${tokyoCabinetLib} ${mysqlLibs} ${pgsqlLibs} -lz

