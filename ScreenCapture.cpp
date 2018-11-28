#include "ScreenCapture.h"

#include <osgDB/WriteFile>

#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtWidgets/QMessageBox>

extern osgViewer::Viewer* g_viewerMain;
extern QMutex g_ErrorLogmutex;

ScreenCapture::ScreenCapture()
{

}

ScreenCapture::~ScreenCapture()
{

}

CScreenCapture::CScreenCapture(CaptureOperation* defaultOperation /*= 0*/, int numFrames/* = 1*/):
ScreenCaptureHandler( defaultOperation, numFrames )
{
	
}

bool CScreenCapture::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	osgViewer::ViewerBase* viewer = dynamic_cast<osgViewer::View*>(&aa)->getViewerBase();
	if (!viewer) return false;

	switch(ea.getEventType())
	{
		case (osgGA::GUIEventAdapter::FRAME):
		{
			// Booleans aren't the best way of doing this, but I want to do
			// the actual adding here because I don't want to require
			// startCapture() take a viewer as argument, which could not be
			// the right one.
			if (_startCapture)
			{
				// Start capturing with the currently set number of frames.
				// If set to -1 it will capture continuously, if set to >0
				// it will capture that number of frames.
				_startCapture = false;
				addCallbackToViewer(*viewer);
			}
			else if (_stopCapture)
			{
				_stopCapture = false;
				removeCallbackFromViewer(*viewer);
			}

			break;
		}

		case (osgGA::GUIEventAdapter::USER) :
		{
			const CaptureEvent* pEvent = dynamic_cast<const CaptureEvent*>(ea.getUserData());
			if (pEvent)
			{
				if (getFramesToCapture() == 0)
				{
					setFramesToCapture(1);
				}
				addCallbackToViewer(*viewer);
				return true;
			}
		}

		default:
			break;
	}
	return false;
}

void CScreenCapture::WriteToImageFile::operator()( const osg::Image& image, const unsigned int context_id )//仿函数实现图片保存
{
	if (!_filename.empty())
	{
		bool bTag = image.isDataContiguous();
		osgDB::writeImageFile(image, _filename);

		const unsigned char* pData = image.data();
	}
}

CScreenCapture::WriteToImageFile::WriteToImageFile( const std::string& filename, const std::string& extension, SavePolicy savePolicy /*= SEQUENTIAL_NUMBER*/ )
: _filename( filename ),
_extension( extension ),
_savePolicy( savePolicy )
{

}