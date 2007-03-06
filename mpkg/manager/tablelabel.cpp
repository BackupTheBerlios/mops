#include "tablelabel.h"

TableLabel::TableLabel(QTableWidget *table)
{
	packageTable=table;
}

void TableLabel::mousePressEvent(QMouseEvent *event)
{
	printf("Mouse pressed, row = %d\n",row);
	packageTable->setCurrentCell(row, 0);
	
}
