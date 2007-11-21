#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H
#include <QtGui>
class FileListWidget: public QListWidget
{
	Q_OBJECT
	public:
		FileListWidget(QWidget *);
		int focusIndex;
		void focusInEvent(QFocusEvent *focus);
	signals:
		void iGotFocus();
};

class CmdComboBox: public QComboBox
{
	Q_OBJECT
	public:
		CmdComboBox(QWidget *);
		//void keyPressEvent(QKeyEvent *event);
	signals:
		void execCmd();
};
#endif
