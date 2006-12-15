# $Id: Makefile,v 1.1 2006/12/15 09:40:40 adiakin Exp $
#makefile for the xmlParser library
#
all: xmlParser.h xmlParser.cpp PackageConfig.h PackageConfig.cpp test.cpp
	g++ -g -Wall -o testConf \
			xmlParser.cpp xmlParser.h PackageConfig.cpp PackageConfig.h test.cpp

clean:
	-@rm -f testConf				  >/dev/null 2>/dev/null
	-@rm -f *.o				  >/dev/null 2>/dev/null
	-@rm -f *~                   >/dev/null 2>/dev/null
	@echo "Clean OK."
