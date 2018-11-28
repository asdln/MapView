#ifndef INPUTPRODUCTDLG_H
#define INPUTPRODUCTDLG_H

#include <QDialog>
#include "ui_InputProductDlg.h"

class InputProductDlg : public QDialog
{
	Q_OBJECT

public:
	InputProductDlg(QWidget *parent = 0);
	~InputProductDlg();

	Ui::InputProductDlg ui;

private:
	
};

#endif // INPUTPRODUCTDLG_H
