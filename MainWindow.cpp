#include "MainWindow.h"
#include "InputProductDlg.h"
#include "MyDataManager.h"
#include "SettingsDlg.h"

using namespace osgEarth;
using namespace osgEarth::QtGui;

CScreenCapture* g_pScreenCapture = nullptr;
CScreenCapture::WriteToImageFile* g_pCaptureOperation = nullptr;

bool DelDir(const QString &path);

DemoMainWindow::DemoMainWindow(osgEarth::QtGui::DataManager* manager, osgEarth::MapNode* mapNode, osg::Group* annotationRoot)
	: _manager(manager), _mapNode(mapNode), _annoRoot(annotationRoot), _layerAdded(false), _viewerWidget(0L)
{
	_annotationToolbar = nullptr;
	m_pCurrentLineModelLayer = nullptr;
	m_pCurrentPointModelLayer = nullptr;

	initUi();
}

void DemoMainWindow::setViewerWidget(osgEarth::QtGui::ViewerWidget* viewerWidget)
{
	setCentralWidget(viewerWidget);
	_viewerWidget = viewerWidget;

	_views.clear();
	_viewerWidget->getViews(_views);

	if (_annotationToolbar)
		_annotationToolbar->setActiveViews(_views);
}

void DemoMainWindow::slotStop()
{
	g_bPlaneMove = !m_pActionStop->isChecked();
	m_pActionCapture->setEnabled(!g_bPlaneMove);
}

void DemoMainWindow::slotAddProduct()
{
	bool bChecked = m_pActionSetProduct->isChecked();

	//添加
	if (bChecked)
	{
		if (_mapNode.valid() && _views.size() > 0)
		{
			m_ptrMouseHandler = new AddPlaneMouseHandler(_mapNode.get());
			for (ViewVector::const_iterator it = _views.begin(); it != _views.end(); ++it)
				(*it)->addEventHandler(m_ptrMouseHandler.get());
		}
	}
	else
	{
		if (_mapNode.valid() && _views.size() > 0)
		{
			for (ViewVector::const_iterator it = _views.begin(); it != _views.end(); ++it)
				(*it)->removeEventHandler(m_ptrMouseHandler.get());
		}
	}
}

void DemoMainWindow::slotRemoveAllProducts()
{
	MyDataManager::ClearProductNode();
}

void DemoMainWindow::slotInputProduct()
{
	InputProductDlg dlg;
	if (dlg.exec())
	{
		double dLon = dlg.ui.lineEdit_Lon->text().toDouble();
		double dLat = dlg.ui.lineEdit_Lat->text().toDouble();

		GeoPoint geoPoint(osgEarth::SpatialReference::get("wgs84"), dLon, dLat, 10000.0);
		MyDataManager::CreateProductNode(geoPoint, 0.0);
	}
}

void DemoMainWindow::slotSettings()
{
	SettingsDlg dlg;
	if (dlg.exec())
	{

	}
}

void DemoMainWindow::slotCapture()
{
	std::vector<osgViewer::View*> Views;
	g_viewerMain->getViews(Views);

	if (Views.empty()) return;

	if (g_pCaptureOperation == nullptr)
	{
		g_pCaptureOperation = new CScreenCapture::WriteToImageFile("c:\\abc.png", "");
		CScreenCapture* pScreenCapture = new CScreenCapture(g_pCaptureOperation);
		g_pScreenCapture = pScreenCapture;

		Views[0]->addEventHandler(pScreenCapture);
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		"/home/jana/untitled.png",
		tr("Images (*.png *.jpg)"));

	if (fileName.isEmpty() || fileName.isNull())
		return;

	g_pCaptureOperation->setFilePath(fileName.toLocal8Bit().data());
	g_pScreenCapture->setFramesToCapture(1);
	Views[0]->getEventQueue()->userEvent(new CaptureEvent);
}

void DemoMainWindow::slotLoadAeroLine()
{
	QFileDialog dlg;
	QString strFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		"/home",
		tr("AeroLine (*.dat)"));

	if (strFileName.isEmpty() || strFileName.isNull())
		return;

	if (m_pCurrentLineModelLayer)
	{
		g_MapNode->getMap()->removeModelLayer(m_pCurrentLineModelLayer);

//		不能显示删除。osgEarth会自动释放
// 		delete m_pCurrentModelLayer;
// 		m_pCurrentModelLayer = nullptr;
	}

	if (m_pCurrentPointModelLayer)
	{
		g_MapNode->getMap()->removeModelLayer(m_pCurrentPointModelLayer);
	}

	QString strResourcePath = QApplication::applicationFilePath();
	strResourcePath = QFileInfo(strResourcePath).absolutePath();
	strResourcePath += "/temp";

	//在程序启动的时候删除临时数据文件夹，不在此处删除
	//DelDir(strResourcePath);

	QDir dir;
	dir.mkdir(strResourcePath);

	QString strLinePath = strResourcePath + "/" + QUuid::createUuid().toString() + ".shp";
	QString strPointPath = strResourcePath + "/" + QUuid::createUuid().toString() + ".shp";
	QStringList strPathList;
	strPathList.push_back(strLinePath);
	strPathList.push_back(strPointPath);

	Aero2Shp::TranslateFile(strFileName, strPathList);

	//加载线shp数据
	{
		osgEarth::Drivers::OGRFeatureOptions featureOptions;
		featureOptions.url() = strLinePath.toUtf8().data();

		osgEarth::Symbology::Style style;

		osgEarth::Symbology::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
		ls->stroke()->color() = osgEarth::Symbology::Color::Red;
		ls->stroke()->width() = 4.0f;

		osgEarth::Drivers::FeatureGeomModelOptions geomOptions;
		geomOptions.featureOptions() = featureOptions;
		geomOptions.styles() = new osgEarth::Symbology::StyleSheet();
		geomOptions.styles()->addStyle(style);
		geomOptions.enableLighting() = false;

		osgEarth::Drivers::ModelLayerOptions layerOptions("line features", geomOptions);
		m_pCurrentLineModelLayer = new osgEarth::ModelLayer(layerOptions);
		g_MapNode->getMap()->addModelLayer(m_pCurrentLineModelLayer);
	}

	//加载点shp数据
	{
		osgEarth::Drivers::OGRFeatureOptions featureOptions;
		featureOptions.url() = strPointPath.toUtf8().data();

		osgEarth::Symbology::Style style;

		osgEarth::Symbology::PointSymbol* ps = style.getOrCreateSymbol<osgEarth::Symbology::PointSymbol>();
		ps->fill()->color() = osgEarth::Symbology::Color::Yellow;
		ps->size() = 8.0;

		osgEarth::Symbology::TextSymbol* text = style.getOrCreateSymbol<osgEarth::Symbology::TextSymbol>();
		text->content() = osgEarth::Symbology::StringExpression("[Name]");
		//text->priority() = osgEarth::Symbology::NumericExpression("[pop_cntry]");
		text->removeDuplicateLabels() = true;
		text->size() = 30.0f;
		text->alignment() = osgEarth::Symbology::TextSymbol::ALIGN_CENTER_CENTER;
		text->fill()->color() = osgEarth::Symbology::Color::Green;
		text->halo()->color() = osgEarth::Symbology::Color::Yellow;

		osgEarth::Drivers::FeatureGeomModelOptions geomOptions;
		geomOptions.featureOptions() = featureOptions;
		geomOptions.styles() = new osgEarth::Symbology::StyleSheet();
		geomOptions.styles()->addStyle(style);
		geomOptions.enableLighting() = false;

		osgEarth::Drivers::ModelLayerOptions layerOptions("point features", geomOptions);
		m_pCurrentPointModelLayer = new osgEarth::ModelLayer(layerOptions);
		g_MapNode->getMap()->addModelLayer(m_pCurrentPointModelLayer);
	}
}

void DemoMainWindow::addRemoveLayer()
{
	if (!_testLayer.valid())
	{
		osgEarth::Drivers::GDALOptions layerOpt;
		layerOpt.url() = osgEarth::URI("../data/nyc-inset-wgs84.tif");
		_testLayer = new osgEarth::ImageLayer(osgEarth::ImageLayerOptions("ny_inset", layerOpt));
	}

	if (!_layerAdded)
	{
		_manager->map()->addImageLayer(_testLayer.get());
		_layerAdded = true;
		_addRemoveLayerAction->setText(tr("&Remove Layer"));
		_addRemoveLayerAction->setToolTip("Remove an image layer");
	}
	else
	{
		_manager->map()->removeImageLayer(_testLayer.get());
		_layerAdded = false;
		_addRemoveLayerAction->setText(tr("&Add Layer"));
		_addRemoveLayerAction->setToolTip("Add an image layer");
	}
}

void DemoMainWindow::addAnnotation()
{
	osgEarth::Annotation::PlaceNode* annotation = new osgEarth::Annotation::PlaceNode(
		_mapNode.get(),
		osgEarth::GeoPoint(_mapNode->getMapSRS(), -74.0, 40.714),
		osgDB::readImageFile("../data/placemark32.png"),
		"New York");

	osgEarth::Annotation::AnnotationData* annoData = new osgEarth::Annotation::AnnotationData();
	annoData->setName("New York");
	annoData->setViewpoint(osgEarth::Viewpoint("New Tork", -74, 40.714, 0, 0.0, -90.0, 1e5));
	annotation->setAnnotationData(annoData);

	annotation->installDecoration("selected", new osgEarth::Annotation::ScaleDecoration(2.0f));

	_manager->addAnnotation(annotation, _annoRoot.get());

	_addAnnotationAction->setDisabled(true);
}

void DemoMainWindow::closeEvent(QCloseEvent *event)
{
	if (_viewerWidget)
	{
		_viewerWidget->getViewer()->setDone(true);
	}

	event->accept();
}

void DemoMainWindow::initUi()
{
	setWindowTitle(tr("map view"));
	//setWindowIcon(QIcon(":/resources/images/pmicon32.png"));

	//createActions();

	QToolBar* pToolBar = addToolBar("tool");

	m_pActionSetProduct = pToolBar->addAction(QString::fromLocal8Bit("添加产品"));
	m_pActionSetProduct->setCheckable(true);
	connect(m_pActionSetProduct, SIGNAL(triggered()), this, SLOT(slotAddProduct()));

	QAction* pActionInputProduct = pToolBar->addAction(QString::fromLocal8Bit("输入产品"));
	connect(pActionInputProduct, SIGNAL(triggered()), this, SLOT(slotInputProduct()));

	QAction* pActionDeleteProduct = pToolBar->addAction(QString::fromLocal8Bit("删除产品"));
	connect(pActionDeleteProduct, SIGNAL(triggered()), this, SLOT(slotRemoveAllProducts()));

	m_pActionStop = pToolBar->addAction(QString::fromLocal8Bit("停止"));
	m_pActionStop->setCheckable(true);
	m_pActionStop->setChecked(!g_bPlaneMove);

	connect(m_pActionStop, SIGNAL(triggered()), this, SLOT(slotStop()));

	m_pActionCapture = pToolBar->addAction(QString::fromLocal8Bit("截图"));
	m_pActionCapture->setEnabled(!g_bPlaneMove);

	connect(m_pActionCapture, SIGNAL(triggered()), this, SLOT(slotCapture()));

	QAction* pActionSettings = pToolBar->addAction(QString::fromLocal8Bit("设置"));
	connect(pActionSettings, SIGNAL(triggered()), this, SLOT(slotSettings()));

// 	QAction* pActionLoadAreoLine = pToolBar->addAction(QString::fromLocal8Bit("加载航线"));
// 	connect(pActionLoadAreoLine, SIGNAL(triggered()), this, SLOT(slotLoadAeroLine()));
}

void DemoMainWindow::createActions()
{
	_addRemoveLayerAction = new QAction(tr("&Add Layer"), this);
	_addRemoveLayerAction->setToolTip(tr("Add an image layer"));
	connect(_addRemoveLayerAction, SIGNAL(triggered()), this, SLOT(addRemoveLayer()));
	_addRemoveLayerAction->setDisabled(!_manager.valid());

	_addAnnotationAction = new QAction(/*QIcon(":/images/open.png"),*/ tr("&Add Annotation"), this);
	_addAnnotationAction->setToolTip(tr("Add an annotation"));
	connect(_addAnnotationAction, SIGNAL(triggered()), this, SLOT(addAnnotation()));
	_addAnnotationAction->setDisabled(!_manager.valid() || !_mapNode.valid() || !_annoRoot.valid());

	_terrainProfileAction = new QAction(QIcon(":/images/terrain_profile.png"), tr(""), this);
	_terrainProfileAction->setToolTip(tr("Terrain Profile Tool"));
	_terrainProfileAction->setCheckable(true);
	connect(_terrainProfileAction, SIGNAL(toggled(bool)), this, SLOT(terrainProfileToggled(bool)));
}