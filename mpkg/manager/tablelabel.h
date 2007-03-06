/*
 * MOPSLinux Packaging system
 * Table item redefinition
 * $Id: tablelabel.h,v 1.1 2007/03/06 15:14:57 i27249 Exp $
 */
#ifndef TABLELABEL_H_
#define TABLELABEL_H_

#include <QLabel>
#include <QTableWidget>
class TableLabel: public QLabel
{

	Q_OBJECT
	protected:
		void mousePressEvent(QMouseEvent *event);
	public:
		TableLabel(QTableWidget *table);
		int row;
		QTableWidget *packageTable;
};

#endif
