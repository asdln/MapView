#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include <osgEarthQt/AnnotationToolbar>
#include <osgEarthQt/Common>
#include <osgEarthQt/DataManager>
#include <osgEarthQt/MapCatalogWidget>
#include <osgEarthQt/TerrainProfileWidget>
#include <osgEarthQt/ViewerWidget>

#include <osgEarthAnnotation/AnnotationNode>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthAnnotation/ScaleDecoration>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarth/GeoData>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>

#include <QAction>
#include <QDockWidget>
#include <QtGui>
#include <QMainWindow>
#include <QToolBar>
#include <QFileDialog>
#include <QUuid>

#include <QtWidgets/QApplication>
#include "ScreenCapture.h"
#include "Aero2Shp.h"
#include "AddPlaneMouseHandler.h"

extern bool g_bPlaneMove;

extern osgViewer::Viewer* g_viewerMain;
extern osgEarth::MapNode* g_MapNode;

extern CScreenCapture* g_pScreenCapture;
extern CScreenCapture::WriteToImageFile* g_pCaptureOperation;

class DemoMainWindow : public QMainWindow
{
	Q_OBJECT

public:

	DemoMainWindow(osgEarth::QtGui::DataManager* manager, osgEarth::MapNode* mapNode, osg::Group* annotationRoot);

	void setViewerWidget(osgEarth::QtGui::ViewerWidget* viewerWidget);

private slots:

	void slotStop();

	void slotCapture();

	void slotLoadAeroLine();

	void slotAddProduct();

	void slotRemoveAllProducts();

	void slotInputProduct();

	void slotSettings();

	void addRemoveLayer();

	void addAnnotation();

protected:

	void closeEvent(QCloseEvent *event);

	void initUi();

	void createActions();

private:

	QAction* m_pActionSetProduct;

	QAction* m_pActionStop;
	QAction* m_pActionCapture;

	osg::ref_ptr<osgEarth::QtGui::DataManager> _manager;
	osg::ref_ptr<osgEarth::MapNode> _mapNode;
	osg::ref_ptr<osg::Group> _annoRoot;
	osg::ref_ptr<osgEarth::ImageLayer> _testLayer;
	osgEarth::QtGui::ViewerWidget* _viewerWidget;
	osgEarth::QtGui::AnnotationToolbar* _annotationToolbar;
	osgEarth::QtGui::ViewVector _views;
	osg::ref_ptr<AddPlaneMouseHandler> m_ptrMouseHandler;
	bool _layerAdded;
	QAction *_addRemoveLayerAction;
	QAction *_addAnnotationAction;
	QAction *_terrainProfileAction;
	QToolBar *_fileToolbar;

	osgEarth::ModelLayer* m_pCurrentLineModelLayer;
	osgEarth::ModelLayer* m_pCurrentPointModelLayer;
};


#endif // MAINWINDOW_H
