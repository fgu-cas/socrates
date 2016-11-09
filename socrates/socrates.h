#ifndef SOCRATES_H
#define SOCRATES_H

#include <QtWidgets/QMainWindow>
#include "ui_socrates.h"

class socrates : public QMainWindow
{
	Q_OBJECT

public:
	socrates(QWidget *parent = 0);
	~socrates();

private:
	Ui::socratesClass ui;
};

#endif // SOCRATES_H
