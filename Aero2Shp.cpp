#include "Aero2Shp.h"
#include "ogr_api.h"
#include "ogrsf_frmts.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>

Aero2Shp::Aero2Shp()
{
}

Aero2Shp::~Aero2Shp()
{
	//OGRCleanupAll();
}

struct ptd
{
	double dx;
	double dy;
};


void Aero2Shp::TranslateFile(const QString& strAeroFile, const QStringList& strShpFileList)
{
	OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES");
	OGRSFDriverRegistrar* poR = OGRSFDriverRegistrar::GetRegistrar();
	OGRSFDriver* poDriver = poR->GetDriverByName("ESRI Shapefile");

	if (poDriver == nullptr)
		return;

	QFile file(strAeroFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QList<QList<ptd> > listLines;

	QList<ptd> currentLine;

	//读取航线文件
	{
		while (!file.atEnd())
		{
			QByteArray line = file.readLine();
			QTextStream textLine(line);

			char spe = 'a';
			ptd point;
			textLine >> point.dy >> spe >> point.dx;

			if (spe == 0)
			{
				if (!currentLine.isEmpty())
				{
					listLines.append(currentLine);
					currentLine.clear();
				}

				continue;
			}

			currentLine.push_back(point);
		}
	}

	if (!currentLine.isEmpty())
	{
		listLines.append(currentLine);
		currentLine.clear();
	}

	OGRDataSource* poDSLine = poDriver->CreateDataSource(strShpFileList[0].toUtf8().data());
	OGRDataSource* poDSPoint = poDriver->CreateDataSource(strShpFileList[1].toUtf8().data());

	QString strWKT = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AUTHORITY[\"EPSG\",\"4326\"]]";
	OGRSpatialReference ogrSRF;
	QByteArray byteArray = strWKT.toUtf8();
	char* pButes = byteArray.data();
	ogrSRF.importFromWkt(&pButes);

	OGRLayer* poLayerLine = poDSLine->CreateLayer(strShpFileList[0].toUtf8().data(), &ogrSRF, wkbLineString);
	OGRLayer* poLayerPoint = poDSPoint->CreateLayer(strShpFileList[1].toUtf8().data(), &ogrSRF, wkbPoint);

	OGRFeatureDefn* pDefnLine = poLayerLine->GetLayerDefn();
	OGRFeatureDefn* pDefnPoint = poLayerPoint->GetLayerDefn();

	//创建线
	for (QList<QList<ptd> >::iterator itr = listLines.begin(); itr != listLines.end();itr ++)
	{
		QList<ptd>& line = *itr;
		OGRLineString lineString;
		
		for (QList<ptd>::iterator itrLine = line.begin(); itrLine != line.end(); itrLine ++)
		{
			ptd& point = *itrLine;
			lineString.addPoint(point.dx, point.dy);
		}

		OGRFeature* poFeature = OGRFeature::CreateFeature(pDefnLine);
		poFeature->SetGeometry(&lineString);
		poLayerLine->CreateFeature(poFeature);

		OGRFeature::DestroyFeature(poFeature);
	}

	//创建点

	OGRFieldDefn oField("Name", OFTString);
	oField.SetWidth(32);
	poLayerPoint->CreateField(&oField);

	char strName[33];
	char lineId = 'a';
	int nPointTag = 1;
	for (QList<QList<ptd> >::iterator itr = listLines.begin(); itr != listLines.end(); itr++)
	{
		QList<ptd>& line = *itr;
		
		for (QList<ptd>::iterator itrLine = line.begin(); itrLine != line.end(); itrLine++)
		{
			ptd& point = *itrLine;
			OGRPoint ogrPoint(point.dx, point.dy);

			OGRFeature* poFeature = OGRFeature::CreateFeature(pDefnPoint);
			poFeature->SetGeometry(&ogrPoint);

			sprintf(strName, "%c%d", lineId, nPointTag);
			poFeature->SetField("Name", strName);
			poLayerPoint->CreateFeature(poFeature);

			OGRFeature::DestroyFeature(poFeature);
			nPointTag++;
		}

		lineId += 1;
		nPointTag = 1;
	}

	OGRDataSource::DestroyDataSource(poDSLine);
	OGRDataSource::DestroyDataSource(poDSPoint);
}
