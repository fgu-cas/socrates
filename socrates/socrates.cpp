#include "socrates.h"

#include <QSettings>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QDebug>

socrates::socrates(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.resultTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui.resultTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	for (QSerialPortInfo port : QSerialPortInfo::availablePorts()) {
		ui.portCombo->addItem(port.portName());
	}
	
	reset();
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
		startLog();
	}
	else {
		stopLog();
	}
}

void socrates::startLog() {
	reset();

	serialPort.reset(new QSerialPort(ui.portCombo->currentText()));
	bool res = serialPort->open(QIODevice::ReadOnly);

	if (res) {
		connect(serialPort.get(), SIGNAL(readyRead()), this, SLOT(handleData()));
		//connect(serialPort.get(), SIGNAL(error()), this, SLOT(handleError()));

		QString file = ui.fileLine->text();
		if (!file.isEmpty()) {
			logFile.reset(new QFile(file));
			logFile->open(QIODevice::WriteOnly);
			logStream.reset(new QTextStream(logFile.get()));
		}

		ui.startStopButton->setText("Stop");
		ui.statusBar->showMessage(u8"Byl zahájen záznam.");
	}
	else {
		QSerialPort::SerialPortError error = serialPort->error();
		ui.statusBar->showMessage(u8"Nebylo možno zahájit záznam.");
	}

}

void socrates::stopLog() {
	serialPort.reset(0);

	if (!ui.fileLine->text().isEmpty()) {
		(*logStream.get()) << "###\r\n";
		(*logStream.get()) << QString("# %1, %2, %3, %4\r\n").arg(gate1Total).arg(gate2Total).arg(gate3Total).arg(gate4Total);
		logFile->close();
	}

	ui.startStopButton->setText("Start");
	
	ui.statusBar->showMessage(u8"Byl ukončen záznam.");
}

void socrates::handleData() {
	buffer.append(serialPort->readAll());

	int index = buffer.indexOf("\r\n");
	if (index != -1) {
		QString line = buffer.left(index);
		buffer.remove(0, index + 2);
		qDebug() << line;
		QStringList split = line.split(" ");
		qDebug() << split;
		if (split.size() == 2) {
			int gate = split[0].trimmed().toInt();
			int length = split[1].trimmed().toInt();

			switch (gate) {
			case 1:
				gate1Total += length;
				ui.gate1label->setText(QString("%1 s").arg(gate1Total / 1000.0));
				break;
			case 2:
				gate2Total += length;
				ui.gate2label->setText(QString("%1 s").arg(gate2Total / 1000.0));
				break;
			case 3:
				gate3Total += length;
				ui.gate3label->setText(QString("%1 s").arg(gate3Total / 1000.0));
				break;
			case 4:
				gate4Total += length;
				ui.gate4label->setText(QString("%1 s").arg(gate4Total / 1000.0));
				break;
			}

			ui.resultTableWidget->setRowCount(count + 1);
			QTableWidgetItem* gateWidget = new QTableWidgetItem(QString::number(gate));
			gateWidget->setTextAlignment(Qt::AlignCenter);
			QTableWidgetItem* lengthWidget = new QTableWidgetItem(QString("%1 ms").arg(length));
			lengthWidget->setTextAlignment(Qt::AlignCenter);
			ui.resultTableWidget->setItem(count, 0, gateWidget);
			ui.resultTableWidget->setItem(count, 1, lengthWidget);
			ui.resultTableWidget->scrollToBottom();

			if (!ui.fileLine->text().isEmpty()) {
				(*logStream.get()) << QString("%1, %2\r\n").arg(gate).arg(length);
			}

			count++;
		}
	}
}

void socrates::reset() {
	count = 0;
	ui.resultTableWidget->clear();
	ui.resultTableWidget->setHorizontalHeaderLabels(QStringList({ u8"Brána", u8"Délka vstupu" }));

	gate1Total = 0;
	gate2Total = 0;
	gate3Total = 0;
	gate4Total = 0;

	ui.gate1label->setText("0 s");
	ui.gate2label->setText("0 s");
	ui.gate3label->setText("0 s");
	ui.gate4label->setText("0 s");
}