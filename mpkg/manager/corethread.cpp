/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.1 2007/03/14 01:50:24 i27249 Exp $
 * *************************************************************************/

#include "corethread.h"

void coreThread::run()
{
	// The main loop: waiting for actions
	forever {
		switch (actionRequested)
		{
			case CTH_LOAD_PACKAGE_LIST:
				printf("running!...\n");
				xstate = 1;
				loadPackageList();
				break;
			default:
				if (xstate == 1)
				{
					printf("in a loop\n");
					xstate = 0;
				}
				break;
				// Do nothing, wait action
		}
	}
