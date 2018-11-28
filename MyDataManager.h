#pragma once
#include "MyPlaceNode.h"
#include <QtCore/QString>

struct TargetInfo
{
	double m_dDistance;
	double m_dSpeed;

	TargetInfo()
	{
		m_dDistance = 0.0;
		m_dSpeed = 0.0;
	}

	TargetInfo(double dDistance, double dSpeed)
	{
		m_dDistance = dDistance;
		m_dSpeed = dSpeed;
	}
};

class MyDataManager
{
public:
	
	~MyDataManager();

	static MyDataManager* Instance()
	{
		static MyDataManager s_Manager;
		return &s_Manager;
	}

	void Initialize();

	static void SetTargetRemainedTime(double dTime);

	static double GetTargetRemainedTime();

	static void CreateProductNode(const osgEarth::GeoPoint& geoPoint, double dTime);

	static void ClearProductNode();

	void UpdateOneNewTargetNode(const osgEarth::GeoPoint& geoPoint, double dAngle, double dTime);

protected:

	static osg::Switch* CreateTargetNode(const osgEarth::GeoPoint& geoPoint, double dRotate, double dTime);

	static osgEarth::Annotation::MyPlaceNode* CreateNode(const osgEarth::GeoPoint& geoPoint, const QString& strIcon, double dTime);

	MyDataManager();
	MyDataManager(const MyDataManager&);
	MyDataManager& operator = (const MyDataManager&);

public:

	std::list<osg::ref_ptr<osg::Switch>> m_vecTargetNodeInvisible;

	std::map<int, TargetInfo> m_mapTargetInfo;

protected:

	bool m_bInitialized;

	static double s_dTargetRemainedTime;
};

