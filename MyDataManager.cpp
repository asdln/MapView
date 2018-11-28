#include "MyDataManager.h"
#include <QtCore/QString>
#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <osgEarthQt/DataManager>
#include "osgUtil/UpdateVisitor"

using namespace osgEarth;
using namespace osgEarth::Annotation;

extern osg::ref_ptr<osg::Group> s_annoGroup;
extern osg::ref_ptr<osg::Group> s_annoGroupProduct;
extern osgEarth::MapNode* g_MapNode;
extern osgEarth::QtGui::DataManager* g_DataManager;

static char * vertexShader = {
	"void main(void ){\n"
	"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"
	"}\n"
};
static char * fragShader = {
	"uniform sampler2D sampler0;\n"
	"uniform vec4 mcolor;\n"
	"void main(void){\n"
	"gl_FragColor = texture2D(sampler0, gl_TexCoord[0].st);\n"
	"if(gl_FragColor.a >= 0.001)\n"
	"gl_FragColor.a = mcolor.a;\n"
	"}\n"
};

class MColorCallback : public osg::Uniform::Callback
{
public:
	MColorCallback(double dOriginTime) : m_dOriginTime(dOriginTime)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		osg::NodeVisitor::VisitorType eType = nv->getVisitorType();
		if (eType == osg::NodeVisitor::UPDATE_VISITOR)
		{
			osgUtil::UpdateVisitor* pUpdateVisitor = dynamic_cast<osgUtil::UpdateVisitor*>(nv);
			const osg::FrameStamp* pStamp = pUpdateVisitor->getFrameStamp();
			double dTime = pStamp->getSimulationTime();
			dTime -= m_dOriginTime;

			double dCircleTime = MyDataManager::GetTargetRemainedTime();
			dTime /= dCircleTime;
			double dValue = dTime - (int)dTime;
			uniform->set(osg::Vec4(0.0, 0.0, 0.0, 1.0 - dValue));
		}
	}

	void SetOriginTime(double dTime)
	{
		m_dOriginTime = dTime;
	}

private:

	double m_dOriginTime;
	
};

class UserData : public osg::Referenced
{
public: 
	UserData(double dTime) : m_dTime(dTime){}
	~UserData(){}

	double GetTime(){ return m_dTime; }

private:

	double m_dTime;
};

class AutoHideCallback : public osg::NodeCallback
{
public:

	AutoHideCallback(){}
	~AutoHideCallback(){}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) override
	{
		traverse(node, nv);

		osg::Switch* pSwitch = node->asSwitch();
		if (!pSwitch)
			return;

		UserData* pUserData = (UserData*)pSwitch->getUserData();
		double dTime = pUserData->GetTime();

		const osg::FrameStamp* pStamp = nv->getFrameStamp();
		double dCurrentTime = pStamp->getSimulationTime();

		if ((dCurrentTime - dTime) >= MyDataManager::GetTargetRemainedTime())
		{
			pSwitch->setValue(0, false);
			MyDataManager::Instance()->m_vecTargetNodeInvisible.push_back(pSwitch);
			node->setUpdateCallback(nullptr);
		}
	}
};

 double MyDataManager::s_dTargetRemainedTime = 5.0;

MyDataManager::MyDataManager()
{
	m_bInitialized = false;

}

MyDataManager::~MyDataManager()
{
}

void MyDataManager::SetTargetRemainedTime(double dTime)
{
	s_dTargetRemainedTime = dTime;
}

double MyDataManager::GetTargetRemainedTime()
{
	return s_dTargetRemainedTime;
}

void MyDataManager::Initialize()
{
	if (m_bInitialized)
		return;

	m_bInitialized = true;
	for (int i = 0; i < 50; i++) //预加载多个目标。收到信号后只需更新位置，以缩短响应时间。
	{
		osg::Switch* pSwitch = CreateTargetNode(GeoPoint(osgEarth::SpatialReference::get("wgs84"), 0.0, 0.0, 10000.0), 0.0, 0.0);
		m_vecTargetNodeInvisible.push_back(pSwitch);
	}
}

osgEarth::Annotation::MyPlaceNode* MyDataManager::CreateNode(const osgEarth::GeoPoint& geoPoint
	, const QString& strIcon, double dTime)
{
	QString strResourcePath = QCoreApplication::applicationFilePath();
	strResourcePath = QFileInfo(strResourcePath).absolutePath();
	strResourcePath += "/data/";

	//创建并加载飞机
	QString strPlanePath = strResourcePath + strIcon/*"product.png"*/;
	QByteArray arrayPlane = strPlanePath.toLocal8Bit();
	char* pPathPlanePNG = arrayPlane.data();

	Style pin;
	pin.getOrCreate<IconSymbol>()->url()->setLiteral(pPathPlanePNG);
	pin.getOrCreate<IconSymbol>()->alignment() = osgEarth::Symbology::IconSymbol::ALIGN_CENTER_CENTER;
	//PlaceNode* pPlaneTag = new PlaceNode(mapNode, GeoPoint(osgEarth::SpatialReference::get("wgs84"), 0.0, 0.0, 10000.0), "", pin);
	MyPlaceNode* pPlaneTag = new MyPlaceNode(g_MapNode, geoPoint, "", pin);
	//pPlaneTag->RotateHeading(0.0);
	pPlaneTag->getOrCreateAnnotationData()->setName("Product");


	return pPlaneTag;
}

void MyDataManager::CreateProductNode(const osgEarth::GeoPoint& geoPoint, double dTime)
{
	osgEarth::Annotation::MyPlaceNode*pPlaneTag = CreateNode(geoPoint, "product.png", dTime);
	g_DataManager->addAnnotation(pPlaneTag, s_annoGroupProduct);
}

void MyDataManager::ClearProductNode()
{
	int nChildCount = s_annoGroupProduct->getNumChildren();
	s_annoGroupProduct->removeChild(0, nChildCount);
}

void MyDataManager::UpdateOneNewTargetNode(const osgEarth::GeoPoint& geoPoint, double dAngle, double dTime)
{
	osg::Switch* pSwitch = nullptr;
	if (m_vecTargetNodeInvisible.empty())
	{
		pSwitch = CreateTargetNode(geoPoint, dAngle, dTime);
	}
	else
	{
		pSwitch = m_vecTargetNodeInvisible.front().get();
		m_vecTargetNodeInvisible.pop_front();
		osg::Node* pChild = pSwitch->getChild(0);
		MyPlaceNode* placeNode = dynamic_cast<MyPlaceNode*>(pChild);
		if (placeNode)
		{
			placeNode->setPosition(geoPoint);
			placeNode->RotateHeading(dAngle);

			osg::StateSet * ss = placeNode->getOrCreateStateSet();
			osg::ref_ptr<osg::Uniform> mcolor = ss->getUniform("mcolor");// new osg::Uniform("mcolor", osg::Vec4(1.0, 0.0, 1.0, 0.5));

			MColorCallback* pCallback = (MColorCallback*)mcolor->getUpdateCallback();
			pCallback->SetOriginTime(dTime);
		}
	}

	pSwitch->setValue(0, true);
	pSwitch->setUserData(new UserData(dTime));
	pSwitch->setUpdateCallback(new AutoHideCallback);
}

osg::Switch* MyDataManager::CreateTargetNode(const osgEarth::GeoPoint& geoPoint, double dRotate, double dTime)
{
	osgEarth::Annotation::MyPlaceNode*pPlaneTag = CreateNode(geoPoint, "plane.png", dTime);
	pPlaneTag->RotateHeading(dRotate);

	//添加着色器
	if (1)
	{
		osg::StateSet * ss = pPlaneTag->getOrCreateStateSet();
		osg::Program * program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
		program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShader));

		osg::ref_ptr<osg::Uniform> sampler0 = new osg::Uniform("sampler0", 0);
		ss->addUniform(sampler0.get());

		osg::ref_ptr<osg::Uniform> mcolor = new osg::Uniform("mcolor", osg::Vec4(1.0, 0.0, 1.0, 0.5));
		mcolor->setUpdateCallback(new MColorCallback(dTime));
		ss->addUniform(mcolor.get());

		ss->setAttributeAndModes(program, osg::StateAttribute::OVERRIDE);
	}

	osg::Switch* pSwitch = new osg::Switch;
	pSwitch->addChild(pPlaneTag);
	pSwitch->setValue(0, false);
	pSwitch->setUserData(new UserData(dTime));
	//pSwitch->setUpdateCallback(new AutoHideCallback);

	s_annoGroup->addChild(pSwitch);
	//g_DataManager->addAnnotation(pPlaneTag, s_annoGroup);
	//s_annoGroup->setUpdateCallback(new AutoDeleteCallback());

	return pSwitch;
}