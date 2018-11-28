#ifndef AERO2SHP_H
#define AERO2SHP_H

#include <QtCore/QString>

class Aero2Shp
{
public:
	Aero2Shp();
	~Aero2Shp();

	static void TranslateFile(const QString& strAeroFile, const QStringList& strShpFileList);

private:
	
};

#endif // AERO2SHP_H
