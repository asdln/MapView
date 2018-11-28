#ifndef SCALEBARREFRESH_H
#define SCALEBARREFRESH_H

#include <QObject>
#include <QtCore/QTimer>

class ScaleBarRefresh : public QObject
{
	Q_OBJECT

public:
	ScaleBarRefresh(QObject *parent = nullptr);
	~ScaleBarRefresh();

public slots:

	void RefreshScaleBar();

public:

	void Start();

private:
	
	QTimer m_timerRefreshScaleBar;
};

#endif // SCALEBARREFRESH_H
