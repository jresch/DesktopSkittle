#ifndef MDI_CHILD_SKITTLE
#define MDI_CHILD_SKITTLE

#include <QMdiArea>
#include <QFrame>
#include "UiVariables.h"

class GLWidget;
class MainWindow;
class QHBoxLayout;

class MdiChildWindow : public QFrame
{
	Q_OBJECT

public:

	QScrollBar* horizontalScrollBar;
	QScrollBar* verticalScrollBar;
	QFrame* subFrame;
	GLWidget* glWidget;	
	QFrame* largeFrame;
	UiVariables ui;
	
	MdiChildWindow(UiVariables gui);
	//createConnections();
	void connectWidget();
	
public slots:
	void setHorizontalWidth(int val);
	void setPageSize();

	
};	
#endif