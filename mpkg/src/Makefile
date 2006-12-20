# MPKG - MOPSLinux packaging system
# $Id: Makefile,v 1.7 2006/12/20 20:00:38 i27249 Exp $
#

CC=gcc
CXX=g++
CFLAGS=-c -Wall -std=c99 -g
CXXFLAGS=-c -g
LDFLAGS=-lsqlite3

SOURCES=installpkg-ng.cpp mpkg.cpp core.cpp dataunits.cpp conditions.cpp \
	   	dependencies.cpp xmlParser.cpp PackageConfig.cpp debug.cpp \
	   	local_package.cpp sql_pool.cpp file_routines.cpp

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=installpkg-ng

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

.cpp.o:
	$(CC) $(CXXFLAGS) $< -o $@

.PHONY : clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	rm -f installpkg-ng
	rm -f *.o
	rm -f *~
	

