#include "socrates.h"

#include <QSettings>
#include <QSerialPortInfo>
#include <QFileDialog>

socrates::socrates(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//QMetaObject::connectSlotsByName(this);

	for (QSerialPortInfo port : QSerialPortInfo::availablePorts()) {
		ui.portCombo->addItem(port.portName());
	}

	loadSettings();

	ui.statusBar->showMessage(u8"Připraven."); 
}

void socrates::loadSettings() {
	QSettings settings("FGU AV", "Socrates");

	int res = ui.portCombo->findText(settings.value("port").toString());
	ui.portCombo->setCurrentIndex(res);

	QString file = settings.value("file").toString();
	ui.fileLine->setText(file);
}

void socrates::saveSettings() {
	QSettings settings("FGU AV", "Socrates");

	settings.setValue("port", ui.portCombo->currentText());
	settings.setValue("file", ui.fileLine->text());
}

void socrates::closeEvent(QCloseEvent* event) {
	saveSettings();
	event->accept();
}

void socrates::on_fileBrowseButton_clicked() {
	QString file = QFileDialog::getSaveFileName(this, u8"Soubor záznamu", "", "*.log");
	if (!file.isEmpty()) {
		ui.fileLine->setText(file);
	}
}

void socrates::on_startStopButton_clicked() {
	if (serialPort.get() == 0) {
		ui.startStopButton->setText("Stop");
		startLog();
	}
	else {
		ui.startStopButton->setText("Start");
		stopLog();
	}
}

void socrates::startLog() {
	reset();

	serialPort.reset(new QSerialPort(ui.portCombo->currentText()));

	connect(serialPort.get(), SIGNAL(readyRead()), this, SLOT(handleData()));
	//connect(serialPort.get(), SIGNAL(error()), this, SLOT(handleError()));

	ui.statusBar->showMessage(u8"Byl zahájen záznam.");
}

void socrates::stopLog() {
	serialPort.reset(0);
	ui.statusBar->showMessage(u8"Byl ukončen záznam.");
}

void socrates::handleData() {
	buffer.append(serialPort->readAll());

	int index = buffer.indexOf("\n");
	if (index != -1){
		QString line = buffer.remove(0, index + 1);
		QStringList split = line.split(" ");
		if (split.size() == 2) {
			int gate = split[0].trimmed().toInt();
			int length = split[0].trimmed().toInt();

			switch (gate) {
			case 1:
				gate1Total += length;
				ui.gate1label->setText(QString("%1 s").arg(gate1Total/1000.0));
			case 2:
				gate2Total += length;
				ui.gate2label->setText(QString("%1 s").arg(gate2Total / 1000.0));
			case 3:
				gate3Total += length;
				ui.gate3label->setText(QString("%1 s").arg(gate3Total / 1000.0));
			case 4:
				gate4Total += length;
				ui.gate4label->setText(QString("%1 s").arg(gate4Total / 1000.0));
			}
			
			ui.resultTableWidget->setRowCount(count + 1);
			QTableWidgetItem* gateWidget = new QTableWidgetItem(QString(gate));
			QTableWidgetItem* lengthWidget = new QTableWidgetItem(QString("%1 ms").arg(length));
			ui.resultTableWidget->setItem(count, 0, gateWidget);
			ui.resultTableWidget->setItem(count, 1, lengthWidget);
			count++;
		}
	}
}

void socrates::reset() {
	count = 0;
	ui.resultTableWidget->clear();

	gate1Total = 0;
	gate2Total = 0;
	gate3Total = 0;
	gate4Total = 0;

	ui.gate1label->setText("0 s");
	ui.gate2label->setText("0 s");
	ui.gate3label->setText("0 s");
	ui.gate4label->setText("0 s");
}