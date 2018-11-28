#include "AddPlaneMouseHandler.h"
#include "MyDataManager.h"

AddPlaneMouseHandler::AddPlaneMouseHandler(osgEarth::MapNode* mapNode)
	: _mapNode(mapNode), _mouseDown(-1), _lastPoint(-500.0, 0.0, 0.0)
{

}

AddPlaneMouseHandler::~AddPlaneMouseHandler()
{
}

bool AddPlaneMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	osgViewer::View* view = static_cast<osgViewer::View*>(aa.asView());

	if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH  && _mouseDown == -1)
	{
		_mouseDown = ea.getButton();
		_xDown = ea.getX();
		_yDown = ea.getY();
	}
	else if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE && _lastPoint.x() != -500.0)
	{

	}
	else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
	{
		if (ea.getButton() == _mouseDown)
		{
			if (_xDown == ea.getX() && _yDown == ea.getY())
			{
				osg::Vec3d world;
				if (_mapNode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world))
				{
					osgEarth::GeoPoint mapPoint;
					mapPoint.fromWorld(_mapNode->getMapSRS(), world);
					//_mapNode->getMap()->worldPointToMapPoint( world, mapPoint );
					_currentPoint = mapPoint.vec3d();
					_lastPoint = mapPoint.vec3d();

					double dLon = mapPoint.vec3d().x();
					double dLat = mapPoint.vec3d().y();

					double dTime = view->getFrameStamp()->getSimulationTime();

					GeoPoint geoPoint(_mapNode->getMapSRS(), dLon, dLat, 10000.0);
					MyDataManager::Instance()->CreateProductNode(geoPoint, dTime);
					//MyDataManager::Instance()->UpdateOneNewTargetNode(geoPoint, 0.0, dTime);
				}
			}

			_mouseDown = -1;
		}
	}
	// 		else if (ea.getEventType() == osgGA::GUIEventAdapter::DOUBLECLICK)
	// 		{
	// 			QMetaObject::invokeMethod(_dialog, "accept", Qt::QueuedConnection);
	// 			return true;
	// 		}

	return false;
}