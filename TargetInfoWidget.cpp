#include "TargetInfoWidget.h"
#include "MyDataManager.h"

TargetInfoWidget::TargetInfoWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.tableWidget->setColumnCount(3);
	QStringList strList;
	strList << QString::fromLocal8Bit("±àºÅ");
	strList << QString::fromLocal8Bit("¾àÀë");
	strList << QString::fromLocal8Bit("ËÙ¶È");
	ui.tableWidget->setHorizontalHeaderLabels(strList);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(RefreshWidget()));
	m_timer.start(2000);
}

TargetInfoWidget::~TargetInfoWidget()
{

}

void TargetInfoWidget::RefreshWidget()
{
	ui.tableWidget->clearContents();
	std::map<int, TargetInfo>& mapTargetInfo = MyDataManager::Instance()->m_mapTargetInfo;
	int nSize = mapTargetInfo.size();

	ui.tableWidget->setRowCount(nSize);

	int nRow = 0;
	for (std::map<int, TargetInfo>::iterator itr = mapTargetInfo.begin(); itr != mapTargetInfo.end(); itr ++)
	{
		QTableWidgetItem* pItem = new QTableWidgetItem(QString::number(itr->first));
		pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
		ui.tableWidget->setItem(nRow, 0, pItem);

		pItem = new QTableWidgetItem(QString::number(itr->second.m_dDistance));
		pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
		ui.tableWidget->setItem(nRow, 1, pItem);

		pItem = new QTableWidgetItem(QString::number(itr->second.m_dSpeed));
		pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
		ui.tableWidget->setItem(nRow, 2, pItem);
	}
}