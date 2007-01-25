# MPKG - MOPSLinux packaging system
# $Id: Makefile,v 1.15 2007/01/25 09:51:44 i27249 Exp $
#

CC=gcc
CXX=g++
CFLAGS=-c -Wall -std=c99 -g
CXXFLAGS=-c -DEXTRA_CHECK -O3 -Wall
LDFLAGS=-lsqlite3 

SOURCES=config.cpp installpkg-ng.cpp mpkg.cpp core.cpp dataunits.cpp conditions.cpp \
	   	dependencies.cpp xmlParser.cpp PackageConfig.cpp debug.cpp \
	   	local_package.cpp sql_pool.cpp file_routines.cpp repository.cpp syscommands.cpp \
		string_operations.cpp converter.cpp

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
	

