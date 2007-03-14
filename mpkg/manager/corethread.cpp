/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.2 2007/03/14 02:22:17 i27249 Exp $
 * *************************************************************************/

#include "corethread.h"

coreThread::coreThread()
{
	printf("Thread created\n");
	// Basic initializations here...
//	database = new mpkg;	// Database initialization
}

coreThread::~coreThread()
{
	printf("Thread destroyed correctly\n");
	//delete database;
}

void coreThread::run()
{
	printf("Thread started\n");
	// Just entering a loop to receive/emit signals...
	exec();
}

void coreThread::tellAreYouRunning()
{
	printf("Thread is running and received a signal\n");
	emit yesImRunning();
}
