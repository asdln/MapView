/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
* Copyright 2015 Pelican Mapping
* http://osgearth.org
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include <osg/Notify>
#include <osg/Version>
#include <osgEarth/ImageUtils>
#include <osgEarth/MapNode>
#include <osgEarthAnnotation/AnnotationData>
#include <osgEarthAnnotation/AnnotationNode>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthAnnotation/ScaleDecoration>
#include <osgEarthAnnotation/TrackNode>
#include <osgEarthQt/ViewerWidget>
#include <osgEarthQt/LayerManagerWidget>
#include <osgEarthQt/MapCatalogWidget>
#include <osgEarthQt/DataManager>
#include <osgEarthQt/AnnotationListWidget>
#include <osgEarthQt/LOSControlWidget>
#include <osgEarthQt/TerrainProfileWidget>
#include <osgEarthUtil/AnnotationEvents>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/Sky>
#include <osgEarthUtil/Ocean>
#include <osgEarthAnnotation/LocalGeometryNode>

#include <osgEarthDrivers/bing/bingoptions>

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QApplication>
#include "MainWindow.h"
#include "UDPServer.h"
#include <osg/LineWidth>

#include <osg/PointSprite>
#include <osg/BlendFunc>
#include <osg/StateAttribute>
#include <osg/Point>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/GLExtensions>
#include <osg/TexEnv>

#include <osgDB/ReadFile>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
#include "MyPlaceNode.h"
#include "MyManipulator.h"
#include "MyDataManager.h"
#include "TargetInfoWidget.h"

#include <osg/Program>
#include <osgDB/ReadFile>
#include <osg/Shader>
//#include <osgViewer/ViewerEventHandlers>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

using namespace osgEarth;
using namespace osgEarth::Features;
using namespace osgEarth::Drivers;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;

osg::ref_ptr<osg::Group> s_annoGroup;
osg::ref_ptr<osg::Group> s_annoGroupProduct;

osg::Camera* g_hudCamera = nullptr;
osgEarth::MapNode* g_MapNode = nullptr;
osg::Geometry* g_GeoScaleLine = nullptr;

osg::Group* g_root = nullptr;
osg::Node* g_earthNode = nullptr;
osgViewer::Viewer* g_viewerMain = nullptr;
osgText::Text* g_pText = nullptr;

double g_dOriginHeight = 5000000.0;

osgEarth::QtGui::DataManager* g_DataManager = nullptr;

int
usage(const std::string& msg)
{
	OE_NOTICE << msg << std::endl;
	OE_NOTICE << std::endl;
	OE_NOTICE << "USAGE: osgearth_qt [options] file.earth" << std::endl;
	OE_NOTICE << "   --multi n               : use a multi-pane viewer with n initial views" << std::endl;
	OE_NOTICE << "   --stylesheet filename   : optional Qt stylesheet" << std::endl;
	OE_NOTICE << "   --run-on-demand         : use the OSG ON_DEMAND frame scheme" << std::endl;
	OE_NOTICE << "   --tracks                : create some moving track data" << std::endl;

	return -1;
}

//------------------------------------------------------------------

osg::Node* createScaleBarHUD(osgText::Text* updateText)
{
	// create the hud. derived from osgHud.cpp
	// adds a set of quads, each in a separate Geode - which can be picked individually
	// eg to be used as a menuing/help system!
	// Can pick texts too!

	osg::Camera* hudCamera = new osg::Camera;
	g_hudCamera = hudCamera;
	hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	hudCamera->setProjectionMatrixAsOrtho2D(0, 1024, 0, 1024);
	hudCamera->setViewport(0, 0, 1024, 1024);
	hudCamera->setViewMatrix(osg::Matrix::identity());
	hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
	hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);

	std::string timesFont("fonts/times.ttf");

	double dx = 100.0;
	double dy = 50.0;

	double dHeight = 15.0;
	double dWidth = 100.0;

	//绘制比例尺的线
	{
		osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry();
		g_GeoScaleLine = linesGeom.get();
		// pass the created vertex array to the points geometry object.

		osg::Vec3dArray* vertices = new osg::Vec3dArray();

		vertices->push_back(osg::Vec3d(dx, dy + dHeight, 0.0));
		vertices->push_back(osg::Vec3d(dx, dy, 0.0));
		vertices->push_back(osg::Vec3d(dx + dWidth, dy, 0.0));
		vertices->push_back(osg::Vec3d(dx + dWidth, dy + dHeight, 0.0));
		vertices->push_back(osg::Vec3d(dx + dWidth, dy, 0.0));
		vertices->push_back(osg::Vec3d(dx + dWidth * 2.0, dy, 0.0));
		vertices->push_back(osg::Vec3d(dx + dWidth * 2.0, dy + dHeight, 0.0));

		linesGeom->setVertexArray(vertices);
		linesGeom->setDataVariance(osg::Object::DYNAMIC);

		// set the colors as before, plus using the above
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		linesGeom->setColorArray(colors);
		linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

		// set the normal in the same way color.
		osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
		linesGeom->setNormalArray(normals);
		linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

		int nn = vertices->size();

		// This time we simply use primitive, and hardwire the number of coords to use 
		// since we know up front,
		linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertices->size()));
		linesGeom->getOrCreateStateSet()->setMode(GL_LINE_STIPPLE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		//linesGeom->getOrCreateStateSet()->setAttribute(new osg::LineStipple(2, 0x00FF));
		linesGeom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(2.0));

		osg::Geode* geode = new osg::Geode();
		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		geode->setName("simple");
		geode->addDrawable(linesGeom);

		hudCamera->addChild(geode);
	}

	{
		osg::Geode* geode = new osg::Geode();
		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		geode->setName("simple");

		osgText::Text* text = new  osgText::Text;
		geode->addDrawable(text);

		text->setCharacterSize(40.0f);
		text->setFont(timesFont);
		text->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		text->setText("0");
		text->setPosition(osg::Vec3d(dx - 8, dy + dHeight + 8, 0.0));

		hudCamera->addChild(geode);
	}

	{ // this displays what has been selected
		osg::Geode* geode = new osg::Geode();
		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		geode->setName("The text label");
		geode->addDrawable(updateText);
		hudCamera->addChild(geode);

		updateText->setCharacterSize(40.0f);
		updateText->setFont(timesFont);
		updateText->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		updateText->setText("km");
		updateText->setPosition(osg::Vec3d(dx + dWidth * 2.0, dy + dHeight + 4, 0.0));
		updateText->setDataVariance(osg::Object::DYNAMIC);

	}

	return hudCamera;
}

int
main(int argc, char** argv)
{

#ifdef Q_WS_X11
	XInitThreads();
#endif

	QApplication app(argc, argv);

	QString strResourcePath = QApplication::applicationFilePath();
	strResourcePath = QFileInfo(strResourcePath).absolutePath();
	strResourcePath += "/data/";

	QString strResPath = strResourcePath + "base.earth";
	QByteArray arrayTemp = strResPath.toLocal8Bit();
	char* pPath = arrayTemp.data();
	int nSize = strlen(pPath);

	int nArgC = 2;
	char** pArg = new char*[nArgC];
	pArg[0] = "abc.exe";
	pArg[1] = new char[nSize + 1];
	memcpy(pArg[1], pPath, nSize);
	pArg[1][nSize] = 0;

	osg::ArgumentParser arguments(&nArgC, pArg);
	osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

	// load the .earth file from the command line.
	osg::Node* earthNode = osgDB::readNodeFiles(arguments);
	if (!earthNode)
		return usage("Unable to load earth model.");

	g_earthNode = earthNode;

	osg::Group* root = new osg::Group();
	g_root = root;
	root->addChild(earthNode);

	s_annoGroup = new osg::Group();
	s_annoGroupProduct = new osg::Group();
	root->addChild(s_annoGroup);
	root->addChild(s_annoGroupProduct);

	osg::ref_ptr<osgText::Text> updateText = new osgText::Text;
	g_pText = updateText.get();
	root->addChild(createScaleBarHUD(updateText.get()));

	osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(earthNode);

	g_MapNode = mapNode.get();
	QString strFilePath = QApplication::applicationFilePath();
	strFilePath = QFileInfo(strFilePath).absolutePath();

	osg::ref_ptr<osgEarth::QtGui::DataManager> dataManager = new osgEarth::QtGui::DataManager(mapNode.get());
	g_DataManager = dataManager.get();
	osgEarth::Drivers::BingOptions bing;
	QByteArray arrayTemp1 = QString(strFilePath + "/data/1").toLocal8Bit();
	bing.key() = arrayTemp1.data();
	dataManager->map()->addImageLayer(new osgEarth::ImageLayer("TileImage", bing));

	DemoMainWindow appWin(dataManager.get(), mapNode.get(), s_annoGroup);

	osgEarth::QtGui::ViewVector views;
	osg::ref_ptr<osgViewer::ViewerBase> viewer;

	osgEarth::QtGui::ViewerWidget* viewerWidget = 0L;

	// tests: implicity creating a viewer.
	viewerWidget = new osgEarth::QtGui::ViewerWidget(root);
	osgViewer::ViewerBase* pViewBase = viewerWidget->getViewer();

	osgViewer::Viewer* pViewer = dynamic_cast<osgViewer::Viewer*>(pViewBase);
	MyManipulator* pCameraManipulator = new MyManipulator;
	pViewer->setCameraManipulator(pCameraManipulator);

	//设置初始的高度
	if (0)
	{
		osgEarth::Viewpoint viewPoint = pCameraManipulator->getViewpoint();
		double dRange = viewPoint.getRange();
		viewPoint.setRange(g_dOriginHeight);

		pCameraManipulator->setViewpoint(viewPoint);
	}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
	// Enable touch events on the viewer
	viewerWidget->getGraphicsWindow()->setTouchEventsEnabled(true);
#endif

	viewerWidget->getViews(views);

	for (osgEarth::QtGui::ViewVector::iterator i = views.begin(); i != views.end(); ++i)
	{
		i->get()->getCamera()->addCullCallback(new osgEarth::Util::AutoClipPlaneCullCallback(mapNode));
	}
	appWin.setViewerWidget(viewerWidget);

	if (0)
	{
		// create and dock an annotation list widget
		QDockWidget *annoDock = new QDockWidget;
		annoDock->setWindowTitle(QString::fromLocal8Bit("产品/目标"));
		annoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		osgEarth::QtGui::AnnotationListWidget* annoList = new osgEarth::QtGui::AnnotationListWidget(dataManager.get());
		annoList->setActiveViews(views);
		annoDock->setWidget(annoList);
		appWin.addDockWidget(Qt::LeftDockWidgetArea, annoDock);
	}

	QDockWidget* pTargetInfoDock = new QDockWidget;
	pTargetInfoDock->setWindowTitle(QString::fromLocal8Bit("目标信息"));

	TargetInfoWidget* pTargetInfoWidget = new TargetInfoWidget;
	pTargetInfoDock->setWidget(pTargetInfoWidget);
	appWin.addDockWidget(Qt::LeftDockWidgetArea, pTargetInfoDock);

	viewer = viewerWidget->getViewer();
	g_viewerMain = dynamic_cast<osgViewer::Viewer*>(viewer.get());

	if (viewer.valid())
		viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

	MyDataManager::Instance()->Initialize();
	UDPServer server(6665, pViewBase);

	appWin.setGeometry(100, 100, 1280, 800);
	appWin.show();

	return app.exec();
}
