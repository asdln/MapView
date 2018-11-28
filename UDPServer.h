#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QtNetwork/QtNetwork>
#include "osgViewer/Viewer"
#include <osgEarthAnnotation/LocalGeometryNode>
#include <osgEarthAnnotation/PlaceNode>
#include "MyPlaceNode.h"

class UDPServer : public QObject
{
	Q_OBJECT

public:
	UDPServer(int nPort
		, osgViewer::ViewerBase*,QObject *parent = nullptr);
	~UDPServer();

	QUdpSocket *receiver;

	osgViewer::ViewerBase* m_pViewer;

	int m_nPort;

	//ÐÅºÅ²Û
private slots:
	void readPendingDatagrams();

signals:

	void sigPosChanged(double dLon, double dLat);

private:

};

#endif // UDPSERVER_H
