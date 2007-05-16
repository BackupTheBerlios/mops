/******************************************
 * MOPSLinux package system
 * Update monitor - processing thread header
 * $Id: monitor.h,v 1.2 2007/05/16 01:15:59 i27249 Exp $
 */

#ifndef MONITOR_H__
#define MONITOR_H__
#include <QThread>
#include <mpkg/libmpkg.h>
#include <map>
typedef enum
{
	MST_ENABLE=0,
	MST_DISABLE,
	MST_SHUTDOWN
} ThreadAction;


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
		


class monitorThread: public QThread
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
		int updateThreshold;
		int idleTime;
		ThreadAction action;
		void checkUpdates();
		HashDatabase hDatabase;

};

#endif
