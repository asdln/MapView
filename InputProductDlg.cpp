#include "InputProductDlg.h"
#include <QtGui/QDoubleValidator>

InputProductDlg::InputProductDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QDoubleValidator* pValidator = new QDoubleValidator;
	pValidator->setRange(-180.0, 180.0);
	ui.lineEdit_Lon->setValidator(pValidator);

	pValidator = new QDoubleValidator;
	pValidator->setRange(-90.0, 90.0);
	ui.lineEdit_Lat->setValidator(pValidator);
}

InputProductDlg::~InputProductDlg()
{

}
