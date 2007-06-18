#
# MOPSLinux packaging system core
# $Id: CMakeLists.txt,v 1.41 2007/06/18 02:51:10 i27249 Exp $
#

# Project name
PROJECT(LIBMPKG)

# Enabling building of shared libs
SET(BUILD_SHARED_LIBS ON)

# Compilation definitions. They are some differences betweeen Linux and MacOSX...
IF(APPLE)
	ADD_DEFINITIONS(-DNEW_INSTALL_SYSTEM -O2 -funroll-loops -minline-all-stringops -pipe -fomit-frame-pointer -march=prescott -DAPPLE_DEFINED -DUSE_INTERNAL_SED -DDEP_NORMALIZE -DCONFIG_FILE='"/Users/aix/mpkg.xml"')
ELSE(APPLE)
	ADD_DEFINITIONS( -DENABLE_LOGGING -DDL_CLEANUP -Wall -W -DNEW_INSTALL_SYSTEM -O2 -pipe -fomit-frame-pointer -funroll-loops -minline-all-stringops -march=i486 -mtune=i686 -DCONFIG_FILE='"/etc/mpkg.xml"' -DUSE_SYSTEM_SED -DDEP_NORMALIZE -DLINUX_DEFINED)
ENDIF(APPLE)

# Include directories: some diffs between linux and OSX, but doesn't require changes.
INCLUDE_DIRECTORIES($LIBMPKG_SOURCE_DIR /sw/include)

# Linking directories
LINK_DIRECTORIES(/sw/lib)
IF(APPLE)
	LINK_LIBRARIES(mpkg intl sqlite3 dl)
ELSE(APPLE)
	LINK_LIBRARIES(mpkg sqlite3 dl)
ENDIF(APPLE)

# TARGET: building HTTP library (libmpkg-http.so)
ADD_LIBRARY(mpkg-http libmpkg-http HttpDownload debug)
# Link parameters for mpkg-http
TARGET_LINK_LIBRARIES(mpkg-http dl curl)

# TARGET: building core library (libmpkg.so)
ADD_LIBRARY(mpkg dialog bus faststl DownloadFactory DownloadManager PackageConfig conditions config constants converter core dataunits debug dependencies file_routines libmpkg local_package mpkg mpkgsys repository sql_pool string_operations syscommands xmlParser)

# TARGET: building static executable (installpkg-ng-static)
#ADD_EXECUTABLE(installpkg-ng-static DownloadFactory DownloadManager PackageConfig conditions config constants converter core dataunits debug dependencies file_routines installpkg-ng libmpkg local_package mpkg mpkgsys repository sql_pool string_operations syscommands xmlParser)
# Linking...
#IF(APPLE)
#	TARGET_LINK_LIBRARIES(installpkg-ng-static intl mpkg sqlite3 dl)
#ELSE(APPLE)
#	TARGET_LINK_LIBRARIES(installpkg-ng-static mpkg sqlite3 dl)
#ENDIF(APPLE)

# TARGET: building shared executable (installpkg-ng)
ADD_EXECUTABLE(installpkg-ng installpkg-ng2)
TARGET_LINK_LIBRARIES(installpkg-ng mpkg)

