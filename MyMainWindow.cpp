#include "MyMainWindow.h"
#include "labs/lab1/Lab1Widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QTransform>
#include <cmath> // Для M_PI

// --- КОНСТАНТЫ ДЛЯ НАСТРОЙКИ ---
// Размеры оригинального perry_kick.png (важно для расчетов pivot points)
// Установите ТОЧНЫЕ размеры вашего файла perry_kick.png
// Эти константы теперь менее критичны, так как вращение будет вокруг центра,
// но все еще хорошая практика их иметь для масштабирования.
#define ORIGINAL_KICK_PERRY_WIDTH 348.0
#define ORIGINAL_KICK_PERRY_HEIGHT 266.0

// Координаты точки вращения (пятки) и точки прицеливания (носка)
// Эти константы больше НЕ ИСПОЛЬЗУЮТСЯ для новой упрощенной логики.
// Вы можете их удалить или оставить закомментированными.
// #define KICK_PIVOT_X 333.0
// #define KICK_PIVOT_Y 111.0
// #define KICK_AIM_X 248.0
// #define KICK_AIM_Y 61.0
// --- КОНЕЦ КОНСТАНТ ---

MyMainWindow::MyMainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Лабораторные работы: Интерфейсы и устройства ПК");
    setFixedSize(1024, 768);
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
    overallLayout->addStretch();

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

// УПРОЩЕННАЯ ВЕРСИЯ getTransformedKickPerry
QPixmap MyMainWindow::getTransformedKickPerry(QPushButton *button, bool reflectHorizontal) {
    if (kickPerryPixmap.isNull()) {
        qDebug() << "getTransformedKickPerry: kickPerryPixmap is null!";
        return QPixmap();
    }

    QTransform transform;

    // Точка вращения теперь будет центр изображения kickPerryPixmap
    // QTransform::rotate() по умолчанию вращает вокруг (0,0),
    // чтобы вращать вокруг центра, нужно сначала транслировать центр в (0,0),
    // потом повернуть, потом транслировать обратно.

    // Но для простого отражения и небольшого поворота, можно обойтись без сложных трансляций
    // если изображение уже отцентровано или поворот небольшой.

    // 1. Применяем горизонтальное отражение, если нужно
    if (reflectHorizontal) {
        // Отражаем изображение, затем немного поворачиваем, чтобы казалось, что он "целится" влево
        // Угол поворота (в градусах)
        qreal rotationAngle = -15.0; // Например, 15 градусов против часовой стрелки
        transform.scale(-1, 1);
        // Применяем поворот. По умолчанию QTransform.rotate() вращает вокруг (0,0).
        // Чтобы вращать вокруг центра изображения, нужно сдвинуть, повернуть, сдвинуть обратно.
        // Или можно использовать QPixmap::transformed с QTransform.
        // Самый простой способ: сначала отразить, потом повернуть вокруг центра (0,0),
        // а потом отмасштабировать для QLabel.
        transform.rotate(rotationAngle);
    } else {
        // Если не отражаем, немного поворачиваем, чтобы казалось, что он "целится" вправо
        qreal rotationAngle = 15.0; // Например, 15 градусов по часовой стрелке
        transform.rotate(rotationAngle);
    }

    // Применяем трансформацию к оригинальному Pixmap
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
        if (buttonNumber >= 1 && buttonNumber <= 3) { // Кнопки слева
            reflectHorizontal = true;
        }

        QPixmap transformedPerry = getTransformedKickPerry(button, reflectHorizontal);
        if (!transformedPerry.isNull()) {
            // Важно: масштабируем transformedPerry уже после всех трансформаций
            perryLabel->setPixmap(transformedPerry.scaled(perryLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            perryLabel->setText("Kick Perry не найден!");
        }
    } else {
        // Мышь ушла, возвращаем Перри в исходное состояние
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