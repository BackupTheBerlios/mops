/* Miscellaneous constants
    $Id: constants.h,v 1.6 2007/01/31 15:47:33 i27249 Exp $
*/


#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// Package status definitions
#define PKGSTATUS_UNKNOWN 0 		// This means error
#define PKGSTATUS_AVAILABLE 1		// Package is not installed but available in enabled repositories
#define PKGSTATUS_UNAVAILABLE 2		// Package is not installed and not available in any of enabled repositories
#define PKGSTATUS_INSTALL 3		// Package is not installed, available and marked for install
#define PKGSTATUS_UPDATE 4		// Package is installed, and marked to be updated with newer version
#define PKGSTATUS_REMOVE 5		// Package is installed and marked to be removed (uninstalled)
#define PKGSTATUS_INSTALLED 6		// Package is installed and no actions will be performed with him
#define PKGSTATUS_REMOVE_PURGE 7	// Package is installed, marked to remove and purge configuration files
#define PKGSTATUS_PURGE 8		// Package was removed, and his configuration files are marked to purge
#define PKGSTATUS_REMOVED_AVAILABLE 9		// Package was removed, but config files are in touch, and package is available to install again.
#define PKGSTATUS_REMOVED_UNAVAILABLE 10	// Package was removed, but config files are in touch, but package is unavailable to install back.

#define CHKINSTALL_NOTFOUND 1
#define CHKINSTALL_DBERROR 2
#define CHKINSTALL_FILECONFLICT 3
#define CHKINSTALL_UNAVAILABLE 4
#define CHKINSTALL_REMOVE 5
#define CHKINSTALL_REMOVE_PURGE 6
#define CHKINSTALL_INSTALL 7
#define CHKINSTALL_INSTALLED 8
#define CHKINSTALL_PURGE 9
#define CHKINSTALL_AVAILABLE 10

#define ALREADY_INSTALLED 10
#define ERR_FILE_CONFLICT 11
#define ERR_UNKNOWN_STATUS 12
#define ALREADY_MARKED 13
#define ERR_UNAVAILABLE 14
#define ERR_UNKNOWN 15
#define ERR_NOTIMPLEMENTED 16

// Bitmask status definitions
#define ST_INSTALLED 	110000
#define ST_REMOVE	101000
#define ST_INSTALL	100100
#define ST_PURGE	100010
#define ST_AVAILABLE	100001

bool IsAvailable(int status);

/*bool stInstalled(int status);
bool stRemove(int status);
bool stInstall(int status);
bool stPurge(int status);
bool stAvailable(int status);
*/
// File type definitions
#define FTYPE_PLAIN	0
#define FTYPE_CONFIG	1
// Special file definitions: not used now, but maybe in future
#define FTYPE_SYMLINK	2
#define FTYPE_DIR	3
#define FTYPE_DEVICE	4





#endif
