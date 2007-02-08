# MPKG - MOPSLinux packaging system
# $Id: Makefile,v 1.20 2007/02/08 18:57:56 adiakin Exp $
#

CC=gcc
CXX=g++
CFLAGS=-c -Wall -std=c99 -g

# VERY BAD HACK!!!
CXXFLAGS=-c -DDEP_NORMALIZE -Wall -I/sw/include 
LDFLAGS=-L/sw/lib -lsqlite3 -lintl

SOURCES=config.cpp installpkg-ng.cpp mpkg.cpp core.cpp dataunits.cpp conditions.cpp \
	   	dependencies.cpp xmlParser.cpp PackageConfig.cpp debug.cpp \
	   	local_package.cpp sql_pool.cpp file_routines.cpp repository.cpp syscommands.cpp \
		string_operations.cpp converter.cpp constants.cpp

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=installpkg-ng

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

sqlinit:
	rm ../sql/packages.db
	sqlite3 -init ../sql/create_database.sql ../sql/packages.db

sqlclean:
	cp ../sql/packages.db /var/log/mpkg/

