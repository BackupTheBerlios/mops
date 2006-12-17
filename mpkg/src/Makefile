# MPKG - MOPSLinux packaging system
# $Id: Makefile,v 1.2 2006/12/17 19:34:57 i27249 Exp $
#


all: xmlParser.h xmlParser.cpp PackageConfig.h PackageConfig.cpp installpkg-ng.cpp local_package.cpp
	g++ -g -lsqlite3 -o installpkg-ng \
			xmlParser.cpp xmlParser.h PackageConfig.cpp PackageConfig.h mpkg.cpp debug.cpp core.cpp dependencies.cpp conditions.cpp dataunits.cpp sql_pool.cpp local_package.cpp installpkg-ng.cpp 

clean:
	-@rm -f installpkg-ng				  >/dev/null 2>/dev/null
	-@rm -f *.o				  >/dev/null 2>/dev/null
	-@rm -f *~                   >/dev/null 2>/dev/null
	@echo "Clean OK."
