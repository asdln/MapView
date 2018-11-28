#ifndef TARGETINFOWIDGET_H
#define TARGETINFOWIDGET_H

#include <QWidget>
#include "ui_TargetInfoWidget.h"
#include <QtCore/QTimer>

class TargetInfoWidget : public QWidget
{
	Q_OBJECT

public:
	TargetInfoWidget(QWidget *parent = 0);
	~TargetInfoWidget();

public slots:
	void RefreshWidget();

private:
	Ui::TargetInfoWidget ui;

	QTimer m_timer;
};

#endif // TARGETINFOWIDGET_H
