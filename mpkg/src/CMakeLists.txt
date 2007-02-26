#
# MOPSLinux packaging system core
# $Id: CMakeLists.txt,v 1.2 2007/02/26 12:00:37 i27249 Exp $
#

PROJECT(LIBMPKG)
SET(BUILD_SHARED_LIBS ON)
ADD_DEFINITIONS(-DUSE_SYSTEM_SED -DDEP_NORMALIZE)
INCLUDE_DIRECTORIES($LIBMPKG_SOURCE_DIR /sw/include)
LINK_DIRECTORIES(/sw/lib)
LINK_LIBRARIES(mpkg intl sqlite3 dl)
ADD_LIBRARY(mpkg-http libmpkg-http HttpDownload)
TARGET_LINK_LIBRARIES(mpkg-http dl curl)
ADD_LIBRARY(mpkg DownloadFactory DownloadManager PackageConfig conditions config constants converter core dataunits debug dependencies file_routines libmpkg local_package mpkg mpkgsys repository sql_pool string_operations syscommands xmlParser)
ADD_EXECUTABLE(installpkg-ng-static DownloadFactory DownloadManager PackageConfig conditions config constants converter core dataunits debug dependencies file_routines installpkg-ng libmpkg local_package mpkg mpkgsys repository sql_pool string_operations syscommands xmlParser)
TARGET_LINK_LIBRARIES(installpkg-ng-static intl mpkg sqlite3 dl)
