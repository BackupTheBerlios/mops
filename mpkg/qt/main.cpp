/*     Mockup for QT face of mpkg
 *     $Id: main.cpp,v 1.2 2007/02/14 14:33:58 i27249 Exp $
 *     */

#include <QApplication>
 #include <QPushButton>
 #include <QListWidget>
 #include <QWidget>

class SettingsList: public QListWidget
{
	Q_OBJECT
	public slots:
	SettingsList (QWidget *parent = 0);
	~SettingsList();
	void printline();
};

SettingsList::SettingsList(QWidget *parent)
{}

SettingsList::~SettingsList(){}

void SettingsList::printline()
{
	printf("button pressed!!!\n");
}

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);

     QWidget window;
     window.resize(640,480);

     QPushButton quit("Bye!", &window);
     SettingsList list(&window);
     list.setGeometry(0,0,200,140);
     quit.resize(100, 30);
     quit.setGeometry(0,140,200,60);
     QObject::connect(&quit, SIGNAL(clicked()), &list, SLOT(printline()) );
     QString tt="Text 0";
     list.insertItem(1, new QListWidgetItem("true"));
     list.insertItem(2, new QListWidgetItem("true1"));

     window.show();
     quit.show();
     return app.exec();
 } 
