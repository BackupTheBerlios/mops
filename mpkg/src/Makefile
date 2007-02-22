# MPKG - MOPSLinux packaging system
# $Id: Makefile,v 1.34 2007/02/22 12:51:19 adiakin Exp $
#

CC=gcc
CXX=g++
CFLAGS=-c -Wall -std=c99 -g

# VERY BAD HACK!!!
CXXFLAGS=-c -g -DDEBUG -DDEP_NORMALIZE -DUSE_SYSTEM_SED -Wall -I/sw/include 
LDFLAGS=-L/sw/lib -lsqlite3 -ldl

SOURCES=config.cpp installpkg-ng.cpp mpkg.cpp core.cpp dataunits.cpp conditions.cpp \
	   	dependencies.cpp xmlParser.cpp PackageConfig.cpp debug.cpp \
	   	local_package.cpp sql_pool.cpp file_routines.cpp repository.cpp syscommands.cpp \
		string_operations.cpp converter.cpp constants.cpp mpkgsys.cpp libmpkg.cpp \
		DownloadManager.cpp DownloadFactory.cpp

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=installpkg-ng-static

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY : clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	rm -f installpkg-ng
	rm -f *.o
	rm -f *~
	rm -f libmpkg.so
	rm -f libmpkg.a

sqlinit:
	rm ../sql/packages.db
	sqlite3 -init ../sql/create_database.sql ../sql/packages.db

sqlclean:
	cp ../sql/packages.db /var/log/mpkg/

lib:
	make -f Makefile.lib

shared:
	make -f Makefile.installpkg

install:
	cp libmpkg.so /usr/local/lib/
	mkdir -p /usr/local/include/mpkg
	cp *.h /usr/local/include/mpkg/
	ldconfig

http-lib:
	$(CXX) -fPIC -c -ldl -DDEBUG -lcurl HttpDownload.cpp libmpkg-http.cxx
	$(CXX) -shared -fPIC -ldl -lcurl -g libmpkg-http.o HttpDownload.o -o libmpkg-http.so

