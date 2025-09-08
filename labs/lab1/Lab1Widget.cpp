#include "Lab1Widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <winbase.h>

#define STATUS_UNKNOWN "Неизвестно"
#define STATUS_NOT_APPLICABLE "Неприменимо"

Lab1Widget::Lab1Widget(QWidget *parent) : QWidget(parent) {
    // Инициализация UI-элементов
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *titleLabel = new QLabel("Лабораторная работа №1: Энергопитание", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    // Labels для вывода информации
    powerSourceLabel = new QLabel(QString("Тип энергопитания: %1").arg(STATUS_UNKNOWN), this);
    batteryTypeLabel = new QLabel(QString("Тип батареи: %1").arg(STATUS_UNKNOWN), this);
    batteryLevelLabel = new QLabel(QString("Уровень заряда батареи: %1").arg(STATUS_UNKNOWN), this);
    powerSavingModeLabel = new QLabel(QString("Режим энергосбережения: %1").arg(STATUS_UNKNOWN), this);
    batteryLifeTimeFullLabel = new QLabel(QString("Время работы аккумулятора (осталось): %1").arg(STATUS_UNKNOWN), this);
    // Изменяем текст метки для секундомера
    elapsedTimeSinceDisconnectLabel = new QLabel(QString("Время с момента отключения зарядки: %1").arg(STATUS_NOT_APPLICABLE), this);


    // Стиль для всех информационных меток
    QString infoLabelStyle = "font-size: 16px; color: lightgray; margin-bottom: 5px;";
    powerSourceLabel->setStyleSheet(infoLabelStyle);
    batteryTypeLabel->setStyleSheet(infoLabelStyle);
    batteryLevelLabel->setStyleSheet(infoLabelStyle);
    powerSavingModeLabel->setStyleSheet(infoLabelStyle);
    batteryLifeTimeFullLabel->setStyleSheet(infoLabelStyle);
    elapsedTimeSinceDisconnectLabel->setStyleSheet(infoLabelStyle); // Обновляем стиль для новой метки

    mainLayout->addWidget(powerSourceLabel);
    mainLayout->addWidget(batteryTypeLabel);
    mainLayout->addWidget(batteryLevelLabel);
    mainLayout->addWidget(powerSavingModeLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(batteryLifeTimeFullLabel);
    mainLayout->addWidget(elapsedTimeSinceDisconnectLabel); // Используем новую метку
    mainLayout->addSpacing(30);

    // Кнопки для спящего режима и гибернации
    QHBoxLayout *actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setAlignment(Qt::AlignCenter);

    QPushButton *sleepButton = new QPushButton("Перейти в спящий режим", this);
    QPushButton *hibernateButton = new QPushButton("Перейти в гибернацию", this);
    sleepButton->setFixedSize(250, 40);
    hibernateButton->setFixedSize(250, 40);
    QString actionButtonStyle = "QPushButton {"
                                "   font-size: 16px;"
                                "   background-color: #007bff;"
                                "   color: white;"
                                "   border-radius: 10px;"
                                "   border: none;"
                                "}"
                                "QPushButton:hover {"
                                "   background-color: #0056b3;"
                                "}";
    sleepButton->setStyleSheet(actionButtonStyle);
    hibernateButton->setStyleSheet(actionButtonStyle);

    actionButtonsLayout->addWidget(sleepButton);
    actionButtonsLayout->addSpacing(20);
    actionButtonsLayout->addWidget(hibernateButton);

    mainLayout->addLayout(actionButtonsLayout);
    mainLayout->addStretch();

    // Кнопка "Назад"
    QPushButton *backButton = new QPushButton("Назад", this);
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet("QPushButton {"
                              "   font-size: 16px;"
                              "   background-color: #dc3545;"
                              "   color: white;"
                              "   border-radius: 10px;"
                              "   border: none;"
                              "}"
                              "QPushButton:hover {"
                              "   background-color: #c82333;"
                              "}");
    mainLayout->addWidget(backButton, 0, Qt::AlignBottom | Qt::AlignLeft);

    // Подключаем слоты
    connect(sleepButton, &QPushButton::clicked, this, &Lab1Widget::onSleepButtonClicked);
    connect(hibernateButton, &QPushButton::clicked, this, &Lab1Widget::onHibernateButtonClicked);
    connect(backButton, &QPushButton::clicked, this, &Lab1Widget::onBackButtonClicked);

    // Инициализируем lastPowerStatus, чтобы при первом updatePowerStatus
    // правильно определить состояние, если приложение запущено на батарее.
    lastPowerStatus.ACLineStatus = 1; // Предполагаем, что изначально были на AC.
    lastPowerStatus.BatteryLifeTime = BATTERY_LIFE_UNKNOWN;
    disconnectTimestamp = 0; // Убеждаемся, что сброшено


    // Инициализируем таймер
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Lab1Widget::updatePowerStatus);
    updateTimer->start(1000); // Обновляем каждую секунду

    // Вызываем первое обновление при создании виджета
    updatePowerStatus();
    this->setStyleSheet("background-color: #3c3c3c;");
}

Lab1Widget::~Lab1Widget() {
    updateTimer->stop();
}

QString Lab1Widget::formatSecondsToHMS(qint64 seconds) const {
    if (seconds == -1 || seconds == BATTERY_LIFE_UNKNOWN) {
        return STATUS_UNKNOWN;
    }
    if (seconds == 0) {
        return "00:00:00";
    }

    qint64 hours = seconds / 3600;
    seconds %= 3600;
    qint64 minutes = seconds / 60;
    seconds %= 60;

    return QString("%1:%2:%3")
        .arg(static_cast<int>(hours), 2, 10, QChar('0'))
        .arg(static_cast<int>(minutes), 2, 10, QChar('0'))
        .arg(static_cast<int>(seconds), 2, 10, QChar('0'));
}

QString Lab1Widget::getBatteryChemistry() const {
    QString chemistry = STATUS_UNKNOWN;

    HDEVINFO hdev = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY,
                                        0,
                                        0,
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (INVALID_HANDLE_VALUE != hdev) {
        for (int idev = 0; idev < 100; idev++) {
            SP_DEVICE_INTERFACE_DATA did = {0};
            did.cbSize = sizeof(did);

            if (SetupDiEnumDeviceInterfaces(hdev, 0, &GUID_DEVCLASS_BATTERY, idev, &did)) {
                DWORD cbRequired = 0;
                SetupDiGetDeviceInterfaceDetail(hdev, &did, 0, 0, &cbRequired, 0);

                if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
                    PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
                    if (pdidd) {
                        pdidd->cbSize = sizeof(*pdidd);
                        if (SetupDiGetDeviceInterfaceDetail(hdev, &did, pdidd, cbRequired, &cbRequired, 0)) {
                            HANDLE hBattery = CreateFile(pdidd->DevicePath,
                                                         GENERIC_READ | GENERIC_WRITE,
                                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                         NULL,
                                                         OPEN_EXISTING,
                                                         FILE_ATTRIBUTE_NORMAL,
                                                         NULL);
                            if (INVALID_HANDLE_VALUE != hBattery) {
                                BATTERY_QUERY_INFORMATION bqi = {0};
                                DWORD dwWait = 0;
                                DWORD dwOut;

                                if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,
                                                    &dwWait, sizeof(dwWait),
                                                    &bqi.BatteryTag, sizeof(bqi.BatteryTag),
                                                    &dwOut, NULL) && bqi.BatteryTag) {
                                    BATTERY_INFORMATION bi = {0};
                                    bqi.InformationLevel = BatteryInformation;

                                    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                                                        &bqi, sizeof(bqi),
                                                        &bi, sizeof(bi),
                                                        &dwOut, NULL)) {
                                        chemistry = QString(QByteArray((const char*)bi.Chemistry, 4)).trimmed();
                                        CloseHandle(hBattery);
                                        LocalFree(pdidd);
                                        SetupDiDestroyDeviceInfoList(hdev);
                                        return chemistry;
                                    }
                                }
                                CloseHandle(hBattery);
                            }
                        }
                        LocalFree(pdidd);
                    }
                }
            } else if (ERROR_NO_MORE_ITEMS == GetLastError()) {
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(hdev);
    } else {
        qDebug() << "SetupDiGetClassDevs failed: " << GetLastError();
    }
    return chemistry;
}


void Lab1Widget::updatePowerStatus() {
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        // --- ОТЛАДОЧНЫЕ СООБЩЕНИЯ ---
        qDebug() << "--- Power Status Update ---";
        qDebug() << "Current ACLineStatus:" << status.ACLineStatus;
        qDebug() << "Last ACLineStatus:" << lastPowerStatus.ACLineStatus;
        qDebug() << "Current BatteryLifeTime (OS estimate):" << status.BatteryLifeTime;
        qDebug() << "Disconnect Timestamp:" << disconnectTimestamp;
        qDebug() << "---------------------------";
        // --- КОНЕЦ ОТЛАДОЧНЫХ СООБЩЕНИЙ ---


        // Тип энергопитания
        QString powerSource;
        if (status.ACLineStatus == 0) { // Сейчас работает от батареи
            powerSource = "Автономная работа (от батареи)";
            // Если предыдущее состояние было "от сети" или "неизвестно", значит, только что отключили от зарядки
            if (lastPowerStatus.ACLineStatus == 1 || lastPowerStatus.ACLineStatus == 255) {
                disconnectTimestamp = QDateTime::currentMSecsSinceEpoch(); // Записываем текущее время как начало отсчета
                qDebug() << "ОТКЛЮЧЕНО ОТ ЗАРЯДКИ! timestamp =" << disconnectTimestamp;
            }
        } else if (status.ACLineStatus == 1) { // Сейчас работает от сети
            powerSource = "От сети переменного тока";
            disconnectTimestamp = 0; // Сбрасываем секундомер
            qDebug() << "ПОДКЛЮЧЕНО К ЗАРЯДКЕ! Сброс секундомера.";
        } else {
            powerSource = STATUS_UNKNOWN;
        }
        powerSourceLabel->setText("Тип энергопитания: " + powerSource);


        // Тип батареи (химия)
        QString batteryChemistry = getBatteryChemistry();
        if (batteryChemistry == STATUS_UNKNOWN || batteryChemistry.isEmpty()) {
            if ((status.BatteryFlag & 128) == 128) { // SYSTEM_BATTERY_NO_BATTERY
                 batteryTypeLabel->setText(QString("Тип батареи: Отсутствует"));
            } else if ((status.BatteryFlag & 8) == 8) { // SYSTEM_BATTERY_CHARGING
                 batteryTypeLabel->setText(QString("Тип батареи: Заряжается"));
            } else if (status.BatteryLifePercent != 255) {
                 batteryTypeLabel->setText(QString("Тип батареи: Присутствует (тип неизвестен)"));
            } else {
                batteryTypeLabel->setText(QString("Тип батареи: ") + STATUS_UNKNOWN);
            }
        } else {
            batteryTypeLabel->setText(QString("Тип батареи: ") + batteryChemistry);
        }


        // Уровень заряда батареи
        QString batteryLevel;
        if (status.BatteryLifePercent == 255) {
            batteryLevel = STATUS_UNKNOWN;
        } else {
            batteryLevel = QString("%1%").arg(static_cast<int>(status.BatteryLifePercent));
        }
        batteryLevelLabel->setText("Уровень заряда батареи: " + batteryLevel);

        // Текущий режим энергосбережения
        QString powerSavingMode;
        if (status.SystemStatusFlag == 0) {
            powerSavingMode = "Высокая производительность / Сбалансированный";
        } else if (status.SystemStatusFlag == 1) {
            powerSavingMode = "Режим экономии заряда";
        } else {
            powerSavingMode = STATUS_UNKNOWN;
        }
        powerSavingModeLabel->setText("Режим энергосбережения: " + powerSavingMode);

        // Время работы аккумулятора (осталось) - это прямое значение от ОС
        QString runTimeLeftOS;
        if (status.ACLineStatus == 0 && status.BatteryLifeTime != BATTERY_LIFE_UNKNOWN) {
            runTimeLeftOS = formatSecondsToHMS(status.BatteryLifeTime);
        } else if (status.ACLineStatus == 1) {
            runTimeLeftOS = STATUS_NOT_APPLICABLE;
        } else {
            runTimeLeftOS = STATUS_UNKNOWN;
        }
        batteryLifeTimeFullLabel->setText("Время работы аккумулятора (осталось): " + runTimeLeftOS);


        // ВЫВОДИТЬ ВРЕМЯ С МОМЕНТА ОТКЛЮЧЕНИЯ ЗАРЯДКИ (секундомер)
        if (disconnectTimestamp > 0 && status.ACLineStatus == 0) {
            qint64 elapsedMs = QDateTime::currentMSecsSinceEpoch() - disconnectTimestamp;
            qint64 elapsedSeconds = elapsedMs / 1000;

            elapsedTimeSinceDisconnectLabel->setText(QString("Время с момента отключения зарядки: ") + formatSecondsToHMS(elapsedSeconds));
        } else if (status.ACLineStatus == 1) { // Если подключен к сети
            elapsedTimeSinceDisconnectLabel->setText(QString("Время с момента отключения зарядки: ") + STATUS_NOT_APPLICABLE);
        } else { // Во всех остальных случаях
            elapsedTimeSinceDisconnectLabel->setText(QString("Время с момента отключения зарядки: ") + STATUS_UNKNOWN);
        }

        // Обновляем lastPowerStatus в самом конце, после всех расчетов.
        lastPowerStatus = status;

    } else {
        qDebug() << "Ошибка при получении статуса питания (GetSystemPowerStatus). Код ошибки: " << GetLastError();
        powerSourceLabel->setText(QString("Тип энергопитания: Ошибка получения данных"));
        batteryLevelLabel->setText(QString("Уровень заряда батареи: Ошибка получения данных"));
        powerSavingModeLabel->setText(QString("Режим энергосбережения: Ошибка получения данных"));
        batteryLifeTimeFullLabel->setText(QString("Время работы аккумулятора (осталось): Ошибка получения данных"));
        elapsedTimeSinceDisconnectLabel->setText(QString("Время с момента отключения зарядки: Ошибка получения данных"));
        batteryTypeLabel->setText(QString("Тип батареи: Ошибка получения данных"));
    }
}

void Lab1Widget::onSleepButtonClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите перевести компьютер в спящий режим?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (SetSuspendState(FALSE, FALSE, FALSE)) {
            qDebug() << "Система переходит в спящий режим...";
        } else {
            DWORD error = GetLastError();
            qDebug() << "Не удалось перевести систему в спящий режим. Ошибка:" << error;
            QMessageBox::warning(this, "Ошибка", QString("Не удалось перевести систему в спящий режим. Код ошибки: %1. Возможно, требуются права администратора.").arg(static_cast<unsigned long>(error)));
        }
    }
}

void Lab1Widget::onHibernateButtonClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите перевести компьютер в режим гибернации?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (SetSuspendState(TRUE, FALSE, FALSE)) {
            qDebug() << "Система переходит в режим гибернации...";
        } else {
            DWORD error = GetLastError();
            qDebug() << "Не удалось перевести систему в режим гибернации. Ошибка:" << error;
            QMessageBox::warning(this, "Ошибка", QString("Не удалось перевести систему в режим гибернации. Код ошибки: %1. Возможно, требуются права администратора.").arg(static_cast<unsigned long>(error)));
        }
    }
}

void Lab1Widget::onBackButtonClicked() {
    emit backToMainScreen();
}