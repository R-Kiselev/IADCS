#include "MyMainWindow.h"

#include "labs/lab6/Lab6Widget.h" // <--- ДОБАВИТЬ
#include "labs/lab1/Lab1Widget.h"
#include "labs/lab2/Lab2Widget.h"
#include "labs/lab3/Lab3Widget.h"
#include "labs/lab4/Lab4Widget.h"
#include "labs/lab5/Lab5Widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QTransform>
#include <cmath>

#define ORIGINAL_KICK_PERRY_WIDTH 348.0
#define ORIGINAL_KICK_PERRY_HEIGHT 266.0


MyMainWindow::MyMainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Лабораторные работы: Интерфейсы и устройства ПК");
    resize(1024, 768);
    setStyleSheet("background-color: #2b2b2b;");

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    mainScreenWidget = new QWidget(this);
    mainScreenWidget->setStyleSheet("background-color: transparent;");

    QVBoxLayout *overallLayout = new QVBoxLayout(mainScreenWidget);
    overallLayout->addStretch();

    QHBoxLayout *buttonsAndPerryLayout = new QHBoxLayout();
    buttonsAndPerryLayout->setAlignment(Qt::AlignCenter);

    QVBoxLayout *leftColumnLayout = new QVBoxLayout();
    leftColumnLayout->setAlignment(Qt::AlignVCenter);

    QVBoxLayout *rightColumnLayout = new QVBoxLayout();
    rightColumnLayout->setAlignment(Qt::AlignVCenter);

    originalPerryPixmap.load("C:/Users/Raman/CLionProjects/InterfacesAndDevicesOfPC/resources/perry_the_platypus/perry.png");
    kickPerryPixmap.load("C:/Users/Raman/CLionProjects/InterfacesAndDevicesOfPC/resources/perry_the_platypus/perry_kick.png");

    if (originalPerryPixmap.isNull()) {
        qDebug() << "Ошибка: 'perry.png' НЕ ЗАГРУЖЕН!";
    }
    if (kickPerryPixmap.isNull()) {
        qDebug() << "Ошибка: 'perry_kick.png' НЕ ЗАГРУЖЕН!";
    }

    for (int i = 1; i <= 6; ++i) {
        QPushButton *labButton = new QPushButton(QString("Лабораторная работа №%1").arg(i), mainScreenWidget);
        labButton->setFixedSize(350, 50);
        labButton->setStyleSheet("QPushButton {"
                                 "   font-size: 18px;"
                                 "   background-color: rgba(76, 175, 80, 220);"
                                 "   color: white;"
                                 "   border-radius: 10px;"
                                 "   border: none;"
                                 "}"
                                 "QPushButton:hover {"
                                 "   background-color: rgba(69, 160, 73, 220);"
                                 "}"
                                 "QPushButton:pressed {"
                                 "   background-color: rgba(62, 142, 65, 220);"
                                 "}");
        labButton->installEventFilter(this);

        if (i == 1) {
            connect(labButton, &QPushButton::clicked, this, &MyMainWindow::onLab1ButtonClicked);
        } else if (i == 2) {
            connect(labButton, &QPushButton::clicked, this, &MyMainWindow::onLab2ButtonClicked);
        } else if (i == 3) {
            connect(labButton, &QPushButton::clicked, this, &MyMainWindow::onLab3ButtonClicked);
        } else if (i == 4) {
            connect(labButton, &QPushButton::clicked, this, &MyMainWindow::onLab4ButtonClicked);
        } else if (i == 5) { // <-- ДОБАВИТЬ ЭТОТ БЛОК
            connect(labButton, &QPushButton::clicked, this, &MyMainWindow::onLab5ButtonClicked);
        } else if (i == 6) {
            connect(labButton, &QPushButton::clicked, this, &MyMainWindow::onLab6ButtonClicked);
        }



        if (i <= 3) {
            leftColumnLayout->addWidget(labButton);
            if (i < 3) leftColumnLayout->addSpacing(15);
        } else {
            rightColumnLayout->addWidget(labButton);
            if (i < 6) rightColumnLayout->addSpacing(15);
        }
    }

    buttonsAndPerryLayout->addLayout(leftColumnLayout);

    perryLabel = new QLabel(mainScreenWidget);
    if (!originalPerryPixmap.isNull()) {
        perryLabel->setPixmap(originalPerryPixmap.scaled(200, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        perryLabel->setAlignment(Qt::AlignCenter);
    } else {
        perryLabel->setText("Перри не найден!");
        perryLabel->setStyleSheet("color: red; font-size: 16px;");
    }
    perryLabel->setFixedSize(200, 240);

    buttonsAndPerryLayout->addSpacing(50);
    buttonsAndPerryLayout->addWidget(perryLabel);
    buttonsAndPerryLayout->addSpacing(50);

    buttonsAndPerryLayout->addLayout(rightColumnLayout);

    overallLayout->addLayout(buttonsAndPerryLayout);
    overallLayout->addStretch(); // <--- ЭТА СТРОКА ДОБАВЛЕНА ДЛЯ ЦЕНТРИРОВАНИЯ

    stackedWidget->addWidget(mainScreenWidget);
}

bool MyMainWindow::eventFilter(QObject *obj, QEvent *event) {
    QPushButton *button = qobject_cast<QPushButton*>(obj);
    if (button) {
        if (event->type() == QEvent::Enter) {
            updatePerryState(button, true);
        } else if (event->type() == QEvent::Leave) {
            updatePerryState(button, false);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


QPixmap MyMainWindow::getTransformedKickPerry(QPushButton *button, bool reflectHorizontal) {
    if (kickPerryPixmap.isNull()) {
        qDebug() << "getTransformedKickPerry: kickPerryPixmap is null!";
        return QPixmap();
    }

    QTransform transform;
    if (reflectHorizontal) {
        qreal rotationAngle = -15.0;
        transform.scale(-1, 1);
        transform.rotate(rotationAngle);
    } else {
        qreal rotationAngle = 15.0;
        transform.rotate(rotationAngle);
    }

    return kickPerryPixmap.transformed(transform, Qt::SmoothTransformation);
}


void MyMainWindow::updatePerryState(QPushButton *button, bool hovered) {
    if (stackedWidget->currentIndex() != 0) {
        return;
    }

    if (hovered) {
        int buttonNumber = 0;
        QString buttonText = button->text();
        QRegularExpression rx("№(\\d)");
        QRegularExpressionMatch match = rx.match(buttonText);
        if (match.hasMatch()) {
            buttonNumber = match.captured(1).toInt();
        }

        bool reflectHorizontal = false;
        if (buttonNumber >= 1 && buttonNumber <= 3) {
            reflectHorizontal = true;
        }

        QPixmap transformedPerry = getTransformedKickPerry(button, reflectHorizontal);
        if (!transformedPerry.isNull()) {

            perryLabel->setPixmap(transformedPerry.scaled(perryLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            perryLabel->setText("Kick Perry не найден!");
        }
    } else {

        if (!originalPerryPixmap.isNull()) {
            perryLabel->setPixmap(originalPerryPixmap.scaled(perryLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            perryLabel->setText("Перри не найден!");
        }
    }
}

void MyMainWindow::onLab1ButtonClicked() {
    Lab1Widget *lab1 = new Lab1Widget(this);
    stackedWidget->addWidget(lab1);
    stackedWidget->setCurrentWidget(lab1);

    connect(lab1, &Lab1Widget::backToMainScreen, this, [this, lab1]() {
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(lab1);
        lab1->deleteLater();
    });
}

void MyMainWindow::onLab2ButtonClicked() {
    Lab2Widget *lab2 = new Lab2Widget(this);
    stackedWidget->addWidget(lab2);
    stackedWidget->setCurrentWidget(lab2);

    connect(lab2, &Lab2Widget::backToMainScreen, this, [this, lab2]() {
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(lab2);
        lab2->deleteLater();
    });
}


void MyMainWindow::onLab3ButtonClicked() {
    Lab3Widget *lab3 = new Lab3Widget(this);
    stackedWidget->addWidget(lab3);
    stackedWidget->setCurrentWidget(lab3);

    connect(lab3, &Lab3Widget::backToMainScreen, this, [this, lab3]() {
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(lab3);
        lab3->deleteLater();
    });
}

void MyMainWindow::onLab4ButtonClicked() {
    Lab4Widget *lab4 = new Lab4Widget(this);
    stackedWidget->addWidget(lab4);
    stackedWidget->setCurrentWidget(lab4);

    connect(lab4, &Lab4Widget::backToMainScreen, this, [this, lab4]() {
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(lab4);
        lab4->deleteLater();
    });
}

void MyMainWindow::onLab5ButtonClicked() {
    Lab5Widget *lab5 = new Lab5Widget(this);
    stackedWidget->addWidget(lab5);
    stackedWidget->setCurrentWidget(lab5);

    connect(lab5, &Lab5Widget::backToMainScreen, this, [this, lab5]() {
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(lab5);
        lab5->deleteLater();
    });
}

void MyMainWindow::onLab6ButtonClicked() {
    Lab6Widget *lab6 = new Lab6Widget(this);
    stackedWidget->addWidget(lab6);
    stackedWidget->setCurrentWidget(lab6);

    connect(lab6, &Lab6Widget::backToMainScreen, this, [this, lab6]() {
        stackedWidget->setCurrentIndex(0);
        stackedWidget->removeWidget(lab6);
        lab6->deleteLater();
    });
}