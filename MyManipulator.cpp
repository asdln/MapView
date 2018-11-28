#include "MyManipulator.h"
#include "osgText/Text"
#include "osgViewer/Viewer"

MyManipulator::MyManipulator()
{

}

MyManipulator::~MyManipulator()
{

}

void MyManipulator::rotate(double dx, double dy)
{
	dy = 0.0;
	osgEarth::Util::EarthManipulator::rotate(dx, dy);
}

bool MyManipulator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	bool bRes = osgEarth::Util::EarthManipulator::handle(ea, aa);

	if (ea.getEventType() == osgGA::GUIEventAdapter::SCROLL)
	{
		m_scaleBarRefresh.Start();
	}

	return bRes;
}