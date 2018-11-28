#include "SettingsDlg.h"
#include "MyDataManager.h"

SettingsDlg::SettingsDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("Setting");
	ui.setupUi(this);
	ui.lineEdit->setText(QString::number(MyDataManager::GetTargetRemainedTime()));

	QDoubleValidator* pValidator = new QDoubleValidator;
	pValidator->setRange(0.0, 20.0);
	ui.lineEdit->setValidator(pValidator);
	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

SettingsDlg::~SettingsDlg()
{

}

void SettingsDlg::accept()
{
	MyDataManager::SetTargetRemainedTime(ui.lineEdit->text().toDouble());
	QDialog::accept();
}

void SettingsDlg::reject()
{
	QDialog::reject();
}
