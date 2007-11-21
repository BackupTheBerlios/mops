#include "filelistwidget.h"
FileListWidget::FileListWidget(QWidget *)
{
}

void FileListWidget::focusInEvent(QFocusEvent *event)
{
	if (event->type()==QEvent::FocusIn) {
		printf("Got focus\n");
		emit iGotFocus();
	}
}

CmdComboBox::CmdComboBox(QWidget *)
{
}
/*
void CmdComboBox::keyPressEvent(QKeyEvent *event)
{
	if (event->key()==Qt::Key_Enter) emit execCmd();
}*/
