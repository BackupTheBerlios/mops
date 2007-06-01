/******************************************
 * MOPSLinux package system
 * Update monitor - processing thread header
 * $Id: monitor.h,v 1.4 2007/06/01 03:38:45 i27249 Exp $
 */

#ifndef MONITOR_H__
#define MONITOR_H__
#include <QtCore>
#include <mpkg/libmpkg.h>
#include <string>
#include <map>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
using namespace std;


typedef enum
{
	MST_ENABLE=0,
	MST_DISABLE,
	MST_SHUTDOWN
} ThreadAction;

typedef enum
{
	LAC_NONE=0,
	LAC_MANAGER,
	LAC_MERGE
} LaunchAction;
class HashDatabase
{
	public:
		HashDatabase();
		~HashDatabase();
		void setControlPoint();
		void setRepositoryData(string url, string md5);
		bool changed();
		void loadHashes();
		void saveHashes();

	private:
		bool changesExist;
		map <string, string> data;
		vector<string> repList;
};

class monitorThread : public QThread
{
	Q_OBJECT

	public:
		monitorThread();
		void run();

	signals:
		void updatesDetected(bool hasUpdates);
		void showStateMessage(QString text);

	public slots:
		void launchManager();
		void mergeUpdates();
		void enable();
		void disable();
		void shutdown();
		void setUpdateInterval(unsigned int sec);
		void forceCheck();

	private:
		bool _forceCheck;
		void _launchManager();
		void _mergeUpdates();
		int updateThreshold;
		int idleTime;
		ThreadAction action;
		LaunchAction launchAction;
		void checkUpdates();
		HashDatabase hDatabase;

};

#endif
