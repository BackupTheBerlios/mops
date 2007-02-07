/*     Mockup for QT face of mpkg
 *     $Id: main.cpp,v 1.1 2007/02/07 14:54:10 i27249 Exp $
 *     */

#include <QApplication>
 #include <QPushButton>
 #include <QListWidget>
 #include <QWidget>
 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);

     QWidget window;
     window.resize(200,200);

     QPushButton quit("Bye!", &window);
     QListWidget list(&window);
     list.setGeometry(0,0,200,140);
     quit.resize(100, 30);
     quit.setGeometry(0,140,200,60);
     QObject::connect(&quit, SIGNAL(clicked()), &list, SLOT(hide()));
     QString tt="Text 0";
     list.insertItem(1, new QListWidgetItem("true"));
     list.insertItem(2, new QListWidgetItem("true1"));

     window.show();
     quit.show();
     return app.exec();
 } 
