#include "ScaleBarRefresh.h"
#include "osgViewer/Viewer"
#include <osgEarthSymbology/Geometry>
#include <osgEarth/MapNode>
#include <QtCore/QtMath>
#include "osgText/Text"

extern osgViewer::Viewer* g_viewerMain;
extern osgEarth::MapNode* g_MapNode;
extern osg::Geometry* g_GeoScaleLine;
extern osgText::Text* g_pText;

ScaleBarRefresh::ScaleBarRefresh(QObject *parent)
	: QObject(parent)
{
	QObject::connect(&m_timerRefreshScaleBar, SIGNAL(timeout()), this, SLOT(RefreshScaleBar()));
	m_timerRefreshScaleBar.start(500);
}

ScaleBarRefresh::~ScaleBarRefresh()
{

}

void ScaleBarRefresh::RefreshScaleBar()
{
	m_timerRefreshScaleBar.stop();

	osg::Vec3d world;
	osgEarth::GeoPoint mapPoint;

	std::vector<osgViewer::View*> vecViews;
	g_viewerMain->getViews(vecViews);
	osgViewer::View* pView = vecViews[0];

	osg::Vec3d vec3d1;
	osg::Vec3d vec3d2;

	bool b1 = false;
	bool b2 = false;

	if (g_MapNode->getTerrain()->getWorldCoordsUnderMouse(pView->asView(), 0.0, 0.0, world))
	{
		mapPoint.fromWorld(g_MapNode->getMapSRS(), world);
		vec3d1 = mapPoint.vec3d();
		b1 = true;
	}

	if (g_MapNode->getTerrain()->getWorldCoordsUnderMouse(pView->asView(), 10.0, 0.0, world))
	{
		mapPoint.fromWorld(g_MapNode->getMapSRS(), world);
		vec3d2 = mapPoint.vec3d();
		b2 = true;
	}

	if (b1 == false || b2 == false)
		return;

	double dRes = qSqrt((vec3d1.x() - vec3d2.x()) * (vec3d1.x() - vec3d2.x()) + (vec3d1.y() - vec3d2.y()) * (vec3d1.y() - vec3d2.y()));
	dRes /= 10.0;

	//根据分辨率计算一个最合适的比例尺数值,总共19个级别
	int pScale[] = { 1, 20, 50, 100, 200, 500, 1000, 2000,
		5000, 10000, 20000, 25000, 50000, 100000, 200000, 500000, 1000000,
		2000000, 5000000 }; 

	char* pScaleText[] = { "1 m", "20 m", "50 m", "100 m", "200 m", "500 m", "1 km", "2 km",
		"5 km", "10 km", "20 km", "25 km", "50 km", "100 km", "200 km", "500 km", "1000 km",
		"2000 km", "5000 km" };

	//以200个像素的宽度为基准进行计算
	int nDefaultWidth = 400;

	double dTempValue = nDefaultWidth * dRes;
	int nIndex = 0;
	for (int i = 1; i < 19; i ++)
	{
		if (dTempValue > pScale[i - 1] && dTempValue < pScale[i])
		{
			nIndex = i - 1;
		}
	}

	if (dTempValue >= 5000000 && nIndex == 0)
	{
		nIndex = 18;
	}

	int nFinalGeoWidth = pScale[nIndex];
	int nDeviceWidth = nFinalGeoWidth / dRes;

	double dx = 100.0;
	double dy = 50.0;

	double dHeight = 10.0;
	double dWidth = nDeviceWidth / 2.0;

	osg::Vec3dArray* vertices = new osg::Vec3dArray();

	vertices->push_back(osg::Vec3d(dx, dy + dHeight, 0.0));
	vertices->push_back(osg::Vec3d(dx, dy, 0.0));
	vertices->push_back(osg::Vec3d(dx + dWidth, dy, 0.0));
	vertices->push_back(osg::Vec3d(dx + dWidth, dy + dHeight, 0.0));
	vertices->push_back(osg::Vec3d(dx + dWidth, dy, 0.0));
	vertices->push_back(osg::Vec3d(dx + dWidth * 2.0, dy, 0.0));
	vertices->push_back(osg::Vec3d(dx + dWidth * 2.0, dy + dHeight, 0.0));

	g_GeoScaleLine->setVertexArray(vertices);

	g_pText->setText(pScaleText[nIndex]);
	g_pText->setPosition(osg::Vec3d(dx + dWidth * 2.0 - 15, dy + dHeight + 8, 0.0));
}

void ScaleBarRefresh::Start()
{
	m_timerRefreshScaleBar.start(500);
}
