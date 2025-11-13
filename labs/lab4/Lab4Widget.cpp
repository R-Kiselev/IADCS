#include "Lab4Widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMediaDevices>
#include <QAudioInput>
#include <QAudioDevice>
#include <QCameraDevice>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QHeaderView>
#include <QAction>
#include <QCameraFormat>
#include <QVideoSink>
#include <QVideoFrame>
#include <QApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Lab4Widget::Lab4Widget(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupCamera();
    registerHotkeys();
    this->setStyleSheet("background-color: #3c3c3c;");
}

Lab4Widget::~Lab4Widget() {
    unregisterHotkeys();
    if (camera) {
        camera->stop();
    }
    if(trayIcon) {
        trayIcon->hide();
    }
}

void Lab4Widget::setupUi() {
    originalFlags = this->window()->windowFlags();
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    titleLabel = new QLabel("Лабораторная работа №4: Веб-камера", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    cameraInfoTable = new QTableWidget(this);
    cameraInfoTable->setColumnCount(2);
    cameraInfoTable->setHorizontalHeaderLabels({"Параметр", "Значение"});
    cameraInfoTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #2b2b2b; color: #e0e0e0; border: 1px solid #555;"
        "   border-radius: 8px; gridline-color: #444; font-size: 14px;"
        "}"
        "QTableWidget::item { padding: 5px; }"
        "QHeaderView::section {"
        "   background-color: #007bff; color: white; font-weight: bold; padding: 6px;"
        "   border: none; border-bottom: 1px solid #0056b3;"
        "}"
        "QTableWidget::item:selected { background-color: #0056b3; }"
        "QTableCornerButton::section { background-color: #007bff; }"
    );
    cameraInfoTable->verticalHeader()->setVisible(false);
    cameraInfoTable->horizontalHeader()->setStretchLastSection(true);
    cameraInfoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cameraInfoTable->setColumnWidth(0, 200);
    cameraInfoTable->setMaximumHeight(160);
    mainLayout->addWidget(cameraInfoTable);
    mainLayout->addSpacing(10);

    QHBoxLayout* videoLayout = new QHBoxLayout();
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumSize(480, 360);
    videoWidget->setMaximumSize(1280, 720);
    videoWidget->setStyleSheet("background-color: black; border-radius: 8px;");
    videoLayout->addStretch();
    videoLayout->addWidget(videoWidget);
    videoLayout->addStretch();
    mainLayout->addLayout(videoLayout);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    photoButton = new QPushButton("Фото (F1)", this);
    startVideoButton = new QPushButton("Начать видео (F2)", this);
    stopVideoButton = new QPushButton("Остановить видео (F3)", this);
    stealthModeButton = new QPushButton("Скрытый режим (F4)", this);

    QString buttonStyle = "QPushButton { font-size: 15px; background-color: #007bff; color: white; border-radius: 10px; border: none; padding: 10px; }"
                          "QPushButton:hover { background-color: #0056b3; }"
                          "QPushButton:disabled { background-color: #555; }";
    photoButton->setStyleSheet(buttonStyle);
    startVideoButton->setStyleSheet(buttonStyle);
    stopVideoButton->setStyleSheet(buttonStyle.replace("#007bff", "#28a745").replace("#0056b3", "#218838"));
    stealthModeButton->setStyleSheet(buttonStyle.replace("#007bff", "#ffc107").replace("#0056b3", "#e0a800"));
    stopVideoButton->setEnabled(false);

    controlsLayout->addWidget(photoButton);
    controlsLayout->addWidget(startVideoButton);
    controlsLayout->addWidget(stopVideoButton);
    controlsLayout->addWidget(stealthModeButton);
    mainLayout->addLayout(controlsLayout);

    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);
    logDisplay->setMaximumHeight(100);
    logDisplay->setStyleSheet("QTextEdit { background-color: #2b2b2b; color: #ccc; border: 1px solid #555; border-radius: 8px; font-family: 'Consolas', monospace; }");
    mainLayout->addWidget(logDisplay);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    backButton = new QPushButton("Назад", this);
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet("QPushButton { font-size: 16px; background-color: #dc3545; color: white; border-radius: 10px; border: none; } QPushButton:hover { background-color: #c82333; }");

    stealthStatusLabel = new QLabel("Скрытый режим: ВЫКЛ", this);
    stealthStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffc107;");

    bottomLayout->addWidget(backButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(stealthStatusLabel);
    mainLayout->addLayout(bottomLayout);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icon.png"));
    trayIcon->setToolTip("Камера-шпион активна");

    QAction *showAction = new QAction("Показать окно", this);
    QAction *quitAction = new QAction("Выход", this);
    connect(showAction, &QAction::triggered, this, &Lab4Widget::showWindowFromTray);
    connect(quitAction, &QAction::triggered, this, &Lab4Widget::onBackButtonClicked);

    trayMenu = new QMenu(this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason){
        if (reason == QSystemTrayIcon::DoubleClick) {
            showWindowFromTray();
        }
    });

    connect(photoButton, &QPushButton::clicked, this, &Lab4Widget::takePhoto);
    connect(startVideoButton, &QPushButton::clicked, this, &Lab4Widget::startVideoRecording);
    connect(stopVideoButton, &QPushButton::clicked, this, &Lab4Widget::stopVideoRecording);
    connect(stealthModeButton, &QPushButton::clicked, this, &Lab4Widget::toggleStealthMode);
    connect(backButton, &QPushButton::clicked, this, &Lab4Widget::onBackButtonClicked);
}

void Lab4Widget::setupCamera() {
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Веб-камеры не найдены!");
        addLog("Ошибка: Веб-камеры не найдены.");
        return;
    }

    camera = new QCamera(cameras.first(), this);
    captureSession = new QMediaCaptureSession(this);

    imageCapture = new QImageCapture(this);
    mediaRecorder = new QMediaRecorder(this);

    captureSession->setCamera(camera);
    captureSession->setImageCapture(imageCapture);
    captureSession->setRecorder(mediaRecorder);
    captureSession->setVideoOutput(videoWidget);

    const QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
    if (!audioInputs.isEmpty()) {
        captureSession->setAudioInput(new QAudioInput(audioInputs.first(), this));
    } else {
        addLog("Предупреждение: Аудиоустройства для записи не найдены.");
    }

    connect(imageCapture, &QImageCapture::imageSaved, this, [this](int id, const QString &fileName) {
        Q_UNUSED(id);
        addLog(QString("Фото сохранено: %1").arg(QDir::toNativeSeparators(fileName)));
    });

    connect(mediaRecorder, &QMediaRecorder::recorderStateChanged, this, [this](QMediaRecorder::RecorderState state) {
        if (state == QMediaRecorder::RecordingState) {
            startVideoButton->setEnabled(false);
            stopVideoButton->setEnabled(true);
            addLog("Запись видео начата...");
        } else {
            startVideoButton->setEnabled(true);
            stopVideoButton->setEnabled(false);
        }
    });

    connect(mediaRecorder, &QMediaRecorder::errorChanged, this, [this](){
        addLog("Ошибка записи: " + mediaRecorder->errorString());
    });

    camera->start();

    QTimer::singleShot(1000, this, &Lab4Widget::updateCameraInfo);
}

void Lab4Widget::updateCameraInfo() {
    cameraInfoTable->setRowCount(0);
    if (!camera) return;

    const auto allCameras = QMediaDevices::videoInputs();

    auto addInfoRow = [&](const QString &param, const QString &value) {
        int currentRow = cameraInfoTable->rowCount();
        cameraInfoTable->insertRow(currentRow);
        cameraInfoTable->setItem(currentRow, 0, new QTableWidgetItem(param));
        cameraInfoTable->setItem(currentRow, 1, new QTableWidgetItem(value));
    };

    const QCameraDevice &device = camera->cameraDevice();
    if (!device.isNull()) {
        addInfoRow("Описание", device.description());

        QString positionStr;
        switch (device.position()) {
            case QCameraDevice::Position::FrontFace:
                positionStr = "Фронтальная (встроенная)";
                break;
            case QCameraDevice::Position::BackFace:
                positionStr = "Тыльная";
                break;
            default:
                positionStr = "Не указано";
                break;
        }
        addInfoRow("Расположение", positionStr);

        bool isEffectivelyDefault = (allCameras.size() == 1) || device.isDefault();
        addInfoRow("Устройство по умолчанию", isEffectivelyDefault ? "Да" : "Нет");

        addInfoRow("ID устройства", QString(device.id().toHex()));
    }

    if (captureSession && captureSession->videoSink()) {
        QSize resolution = captureSession->videoSink()->videoFrame().size();
        if (resolution.isValid() && !resolution.isEmpty()) {
            addInfoRow("Активное разрешение", QString("%1x%2").arg(resolution.width()).arg(resolution.height()));
        } else {
            addInfoRow("Активное разрешение", "Не определено");
        }
    } else {
        addInfoRow("Активное разрешение", "Недоступно");
    }
}


void Lab4Widget::addLog(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logDisplay->append(QString("[%1] %2").arg(timestamp, message));
}

QString Lab4Widget::getOutputPath() {
    QDir exeDir(QApplication::applicationDirPath());
    exeDir.cdUp();
    QString outputPath = exeDir.absolutePath() + "/Lab4_Output";

    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
        addLog(QString("Создана папка для сохранения: %1").arg(QDir::toNativeSeparators(outputPath)));
    }
    return outputPath;
}

void Lab4Widget::takePhoto() {
    if (!imageCapture || !imageCapture->isReadyForCapture()) {
        addLog("Ошибка: Камера не готова для съемки фото.");
        return;
    }
    QString savePath = getOutputPath() + "/photo_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".jpg";
    imageCapture->captureToFile(savePath);
    addLog("Захват фото...");
}

void Lab4Widget::startVideoRecording() {
    if (!mediaRecorder) return;
    QString savePath = getOutputPath() + "/video_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".mp4";
    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(savePath));
    mediaRecorder->record();
}

void Lab4Widget::stopVideoRecording() {
    if (!mediaRecorder) return;
    mediaRecorder->stop();
    addLog("Запись видео остановлена. Файл сохранен.");
}

void Lab4Widget::toggleStealthMode() {
    isStealthMode = !isStealthMode;
    if (isStealthMode) {
        addLog("Активирован скрытый режим. Окно будет скрыто.");
        stealthStatusLabel->setText("Скрытый режим: ВКЛ");
        trayIcon->show();
        QTimer::singleShot(200, this, [this](){
            this->window()->hide();
        });
    } else {
        showWindowFromTray();
    }
}

void Lab4Widget::showWindowFromTray() {
    isStealthMode = false;
    trayIcon->hide();
    this->window()->show();
    addLog("Скрытый режим отключен.");
    stealthStatusLabel->setText("Скрытый режим: ВЫКЛ");
}

void Lab4Widget::onBackButtonClicked() {
    trayIcon->hide();
    emit backToMainScreen();
}


void Lab4Widget::registerHotkeys() {
#ifdef Q_OS_WIN
    if (!RegisterHotKey((HWND)this->winId(), HOTKEY_ID_F1, 0, VK_F1))
        addLog("Ошибка регистрации горячей клавиши F1.");
    if (!RegisterHotKey((HWND)this->winId(), HOTKEY_ID_F2, 0, VK_F2))
        addLog("Ошибка регистрации горячей клавиши F2.");
    if (!RegisterHotKey((HWND)this->winId(), HOTKEY_ID_F3, 0, VK_F3))
        addLog("Ошибка регистрации горячей клавиши F3.");
    if (!RegisterHotKey((HWND)this->winId(), HOTKEY_ID_F4, 0, VK_F4))
        addLog("Ошибка регистрации горячей клавиши F4.");
#endif
}

void Lab4Widget::unregisterHotkeys() {
#ifdef Q_OS_WIN
    UnregisterHotKey((HWND)this->winId(), HOTKEY_ID_F1);
    UnregisterHotKey((HWND)this->winId(), HOTKEY_ID_F2);
    UnregisterHotKey((HWND)this->winId(), HOTKEY_ID_F3);
    UnregisterHotKey((HWND)this->winId(), HOTKEY_ID_F4);
#endif
}

bool Lab4Widget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY) {
            switch (msg->wParam) {
                case HOTKEY_ID_F1:
                    takePhoto();
                    *result = 1;
                    return true;
                case HOTKEY_ID_F2:
                    if (mediaRecorder->recorderState() != QMediaRecorder::RecordingState)
                        startVideoRecording();
                    *result = 1;
                    return true;
                case HOTKEY_ID_F3:
                    if (mediaRecorder->recorderState() == QMediaRecorder::RecordingState)
                        stopVideoRecording();
                    *result = 1;
                    return true;
                case HOTKEY_ID_F4:
                    toggleStealthMode();
                    *result = 1;
                    return true;
                default:
                    break;
            }
        }
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}