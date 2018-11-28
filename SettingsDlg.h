#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <QDialog>
#include "ui_SettingsDlg.h"

class SettingsDlg : public QDialog
{
	Q_OBJECT

public:
	SettingsDlg(QWidget *parent = 0);
	~SettingsDlg();

public slots:

	void accept() override;
	void reject() override;

private:
	Ui::SettingsDlg ui;
};

#endif // SETTINGSDLG_H
