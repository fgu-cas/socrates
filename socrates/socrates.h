#ifndef SOCRATES_H
#define SOCRATES_H

#include <memory>
#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QSerialPort>
#include <QTextStream>
#include <QElapsedTimer>
#include "ui_socrates.h"

class socrates : public QMainWindow
{
	Q_OBJECT

public:
	socrates(QWidget *parent = 0);

private:
	Ui::socratesClass ui;
	void loadSettings();
	void saveSettings();

	std::unique_ptr<QSerialPort> serialPort;
	std::unique_ptr<QFile> logFile;
	std::unique_ptr<QTextStream> logStream;
	QString buffer;
	QElapsedTimer timer;
	int count = 0;
	int gate1Total;
	int gate2Total;
	int gate3Total;
	int gate4Total;
	void startLog();
	void stopLog();
	void reset();

	void closeEvent(QCloseEvent* event);

private slots:
	void on_fileBrowseButton_clicked();
	void on_startStopButton_clicked();

	void handleData();
};

#endif // SOCRATES_H
