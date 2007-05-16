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
		void updatesDetected();

	public slots:
		void enable();
		void disable();
		void shutdown();
		void setUpdateInterval(unsigned int sec);

	private:
		unsigned int TIMER_RES;
		ThreadAction action;
		void checkUpdates();
		HashDatabase hDatabase;

};

#endif
