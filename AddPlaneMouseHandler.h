#pragma once

#include "osgEarth/MapNode"
#include <osgEarthQt/Common>

#include <osgEarthAnnotation/Draggers>
#include <osgEarth/GeoMath>
#include <osgEarthAnnotation/AnnotationEditing>
#include <osgEarthAnnotation/EllipseNode>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthFeatures/Feature>
#include <osgEarthSymbology/Geometry>
#include <QtCore/qstring.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <osgEarthQt/DataManager>
#include "MyPlaceNode.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;

class AddPlaneMouseHandler : public osgGA::GUIEventHandler
{
public:
	AddPlaneMouseHandler(osgEarth::MapNode* mapNode);

	~AddPlaneMouseHandler();

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:

	osg::ref_ptr<osgEarth::MapNode>  _mapNode;
	osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode;
	osgEarth::Symbology::Style _lineStyle;

	bool _drawLead;
	int _mouseDown;
	float _xDown, _yDown;
	osg::Vec3d _lastPoint;
	osg::Vec3d _currentPoint;
};

