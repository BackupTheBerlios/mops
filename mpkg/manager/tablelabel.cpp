#include "tablelabel.h"

TableLabel::TableLabel(QTableWidget *table)
{
	packageTable=table;
}

void TableLabel::mousePressEvent(QMouseEvent *event)
{
	packageTable->setCurrentCell(row,1);
	
}
