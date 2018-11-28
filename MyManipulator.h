#ifndef MYMANIPULATOR_H
#define MYMANIPULATOR_H

#include "osgEarthUtil/EarthManipulator"
#include "ScaleBarRefresh.h"

class MyManipulator : public osgEarth::Util::EarthManipulator
{
public:
	MyManipulator();
	~MyManipulator();

	virtual void rotate(double dx, double dy) override;

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

private:

	ScaleBarRefresh m_scaleBarRefresh;
};

#endif // MYMANIPULATOR_H
