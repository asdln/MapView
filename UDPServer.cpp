#include "udpserver.h"
#include "osgEarth/SpatialReference"
#include "osgEarthUtil/EarthManipulator"
#include <osg/LineStipple>
#include <osg/LineWidth>
#include "MyDataManager.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;

extern osg::Node* g_earthNode;

double g_dPlanePosLon = 0.0;
double g_dPlanePosLat = 0.0;
double g_dPlanePosAngle = 0.0;

double g_dTargetPosLon = 0.0;
double g_dTargetPosLat = 0.0;

bool g_bPlaneMove = true;

UDPServer::UDPServer(int nPort
	, osgViewer::ViewerBase* pViewer, QObject *parent)
	: QObject(parent)
{
	m_pViewer = pViewer;
	m_nPort = nPort;

	receiver = new QUdpSocket(this);
	receiver->bind(QHostAddress::LocalHost, nPort);
	connect(receiver, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

UDPServer::~UDPServer()
{

}

void UDPServer::readPendingDatagrams()
{
	osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(g_earthNode);
	SpatialReference* pwgs84 = osgEarth::SpatialReference::get("wgs84");
	const SpatialReference* mapSRS = mapNode->getMapSRS();

	while (receiver->hasPendingDatagrams()) 
	{
		QByteArray datagram;
		datagram.resize(receiver->pendingDatagramSize());
		receiver->readDatagram(datagram.data(), datagram.size());
		//数据接收在datagram里
		/* readDatagram 函数原型
		qint64 readDatagram(char *data,qint64 maxSize,QHostAddress *address=0,quint16 *port=0)
		*/

		int nSize = datagram.size();
		double dLon, dLat, dAngle, dDistance, dSpeed;
		int nID;
		memcpy(&dLon, datagram.data() + 1, 8);
		memcpy(&dLat, datagram.data() + 9, 8);
		memcpy(&dAngle, datagram.data() + 17, 8);
		memcpy(&nID, datagram.data() + 25, 4);
		memcpy(&dDistance, datagram.data() + 29, 8);
		memcpy(&dSpeed, datagram.data() + 37, 8);

		osgViewer::ViewerBase::Views views;
		m_pViewer->getViews(views);
		double dTime = views[0]->getFrameStamp()->getSimulationTime();
		GeoPoint geoPoint(osgEarth::SpatialReference::get("wgs84"), dLon, dLat, 10000.0);
		MyDataManager::Instance()->UpdateOneNewTargetNode(geoPoint, dAngle, dTime);

		MyDataManager::Instance()->m_mapTargetInfo[nID] = TargetInfo(dDistance, dSpeed);
	}
}