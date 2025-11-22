#include "Lab6Widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QTimer> // <--- ДОБАВИТЬ ЭТУ СТРОКУ

// Подключение библиотек Windows для Bluetooth
#pragma comment(lib, "Bthprops.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")

#ifndef BLUETOOTH_SERVICE_ENABLE
#define BLUETOOTH_SERVICE_ENABLE 0x01
#endif

Lab6Widget::Lab6Widget(QWidget *parent)
    : QWidget(parent),
      hRadio(NULL),
      bluetoothInitialized(false),
      connectedDevice(nullptr)
{
    setupUi();

    if (initializeBluetooth()) {
        appendToLog("Bluetooth инициализирован успешно", "#28a745"); // Green
        bluetoothInitialized = true;
    } else {
        appendToLog("Ошибка инициализации Bluetooth", "#dc3545"); // Red
        appendToLog("Убедитесь, что Bluetooth адаптер включен", "#ffc107"); // Warning
    }
}

Lab6Widget::~Lab6Widget()
{
    if (connectedDevice) {
        delete connectedDevice;
    }
    if (hRadio) {
        CloseHandle(hRadio);
    }
}

void Lab6Widget::setupUi()
{
    this->setStyleSheet("background-color: #3c3c3c;");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setAlignment(Qt::AlignTop);

    // Заголовок
    titleLabel = new QLabel("Лабораторная работа №6: Bluetooth", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    // --- Таблица устройств ---
    devicesModel = new QStandardItemModel(0, 4, this);
    devicesModel->setHorizontalHeaderLabels({
        "Устройство", "MAC-адрес", "Тип", "Статус"
    });

    devicesTableView = new QTableView();
    devicesTableView->setModel(devicesModel);
    devicesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    devicesTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    devicesTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    devicesTableView->verticalHeader()->setVisible(false);

    devicesTableView->setStyleSheet(
        "QTableView {"
        "   background-color: #2b2b2b;"
        "   color: #e0e0e0;"
        "   border: 1px solid #555;"
        "   border-radius: 8px;"
        "   gridline-color: #444;"
        "   font-size: 14px;"
        "}"
        "QTableView::item { padding: 5px; }"
        "QHeaderView::section {"
        "   background-color: #007bff;"
        "   color: white; font-weight: bold; padding: 8px;"
        "   border: none; border-right: 1px solid #0056b3;"
        "}"
        "QTableView::item:selected { background-color: #0056b3; }"
        "QTableCornerButton::section { background-color: #007bff; }"
    );

    QHeaderView *header = devicesTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    mainLayout->addWidget(devicesTableView, 1);

    // --- Кнопки управления ---
    auto* buttonsLayout = new QHBoxLayout();

    QString actionBtnStyle =
        "QPushButton { font-size: 14px; background-color: #007bff; color: white; border-radius: 10px; border: none; padding: 10px; }"
        "QPushButton:hover { background-color: #0056b3; }"
        "QPushButton:disabled { background-color: #555; color: #888; }";

    scanButton = new QPushButton("Поиск устройств");
    scanButton->setStyleSheet(actionBtnStyle.replace("#007bff", "#28a745").replace("#0056b3", "#218838"));
    connect(scanButton, &QPushButton::clicked, this, &Lab6Widget::onScanDevices);

    connectButton = new QPushButton("Подключиться");
    connectButton->setStyleSheet(actionBtnStyle);
    connectButton->setEnabled(false);
    connect(connectButton, &QPushButton::clicked, this, &Lab6Widget::onConnectDevice);

    disconnectButton = new QPushButton("Сброс");
    disconnectButton->setStyleSheet(actionBtnStyle.replace("#007bff", "#ffc107").replace("#0056b3", "#e0a800").replace("white", "black"));
    disconnectButton->setEnabled(false);
    connect(disconnectButton, &QPushButton::clicked, this, &Lab6Widget::onDisconnectDevice);

    buttonsLayout->addWidget(scanButton);
    buttonsLayout->addWidget(connectButton);
    buttonsLayout->addWidget(disconnectButton);
    mainLayout->addLayout(buttonsLayout);

    // --- Кнопка отправки файла ---
    sendButton = new QPushButton("Отправить аудиофайл");
    sendButton->setStyleSheet(
        "QPushButton { font-size: 14px; background-color: #6f42c1; color: white; border-radius: 10px; border: none; padding: 10px; }"
        "QPushButton:hover { background-color: #59359a; }"
        "QPushButton:disabled { background-color: #555; color: #888; }"
    );
    sendButton->setEnabled(false);
    connect(sendButton, &QPushButton::clicked, this, &Lab6Widget::onSendFile);
    mainLayout->addWidget(sendButton);

    // --- Лог ---
    logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(150);
    logOutput->setStyleSheet(
        "QTextEdit { background-color: #2b2b2b; color: #ccc; border: 1px solid #555; border-radius: 8px; font-family: 'Consolas', monospace; }"
    );
    mainLayout->addWidget(logOutput);

    // --- Кнопка Назад ---
    auto* bottomLayout = new QHBoxLayout();
    backButton = new QPushButton("Назад");
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #dc3545; color: white; border-radius: 10px; border: none; }"
        "QPushButton:hover { background-color: #c82333; }"
    );
    connect(backButton, &QPushButton::clicked, this, &Lab6Widget::onBackButtonClicked);

    bottomLayout->addWidget(backButton);
    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    // Сигнал выбора в таблице
    connect(devicesTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, [this]() {
        bool hasSelection = devicesTableView->selectionModel()->hasSelection();
        connectButton->setEnabled(hasSelection && !connectedDevice);
        disconnectButton->setEnabled(connectedDevice != nullptr);
        sendButton->setEnabled(connectedDevice != nullptr);
    });
}

void Lab6Widget::onBackButtonClicked() {
    emit backToMainScreen();
}

void Lab6Widget::appendToLog(const QString& message, const QString& color)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString finalColor = (color == "black") ? "#e0e0e0" : color;

    // Адаптация цветов
    if (color == "blue") finalColor = "#5dade2";
    if (color == "green") finalColor = "#58d68d";
    if (color == "red") finalColor = "#ec7063";
    if (color == "orange") finalColor = "#f5b041";
    if (color == "gray") finalColor = "#aaaaaa";

    logOutput->append(QString("[%1] <font color='%2'>%3</font>")
                      .arg(timestamp, finalColor, message));
}

bool Lab6Widget::initializeBluetooth()
{
    BLUETOOTH_FIND_RADIO_PARAMS radioParams;
    radioParams.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

    HANDLE hRadioFind = BluetoothFindFirstRadio(&radioParams, &hRadio);
    if (hRadioFind == NULL) return false;

    BluetoothFindRadioClose(hRadioFind);
    BLUETOOTH_RADIO_INFO radioInfo;
    radioInfo.dwSize = sizeof(BLUETOOTH_RADIO_INFO);

    if (BluetoothGetRadioInfo(hRadio, &radioInfo) == ERROR_SUCCESS) {
        appendToLog(QString("Адаптер: %1").arg(QString::fromWCharArray(radioInfo.szName)), "blue");
        return true;
    }
    return false;
}

QList<BluetoothDeviceInfo> Lab6Widget::scanBluetoothDevices()
{
    QList<BluetoothDeviceInfo> devices;
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
    ZeroMemory(&searchParams, sizeof(searchParams));
    searchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
    searchParams.fReturnAuthenticated = TRUE;
    searchParams.fReturnRemembered = TRUE;
    searchParams.fReturnUnknown = TRUE;
    searchParams.fReturnConnected = TRUE;
    searchParams.fIssueInquiry = TRUE;
    searchParams.cTimeoutMultiplier = 4;
    searchParams.hRadio = hRadio;

    BLUETOOTH_DEVICE_INFO deviceInfo;
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

    HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

    if (hFind == NULL) {
        appendToLog("Поиск завершен или не дал результатов.", "gray");
        return devices;
    }

    do {
        BluetoothDeviceInfo info;
        info.name = QString::fromWCharArray(deviceInfo.szName);
        info.address = QString("%1:%2:%3:%4:%5:%6")
            .arg(deviceInfo.Address.rgBytes[5], 2, 16, QChar('0'))
            .arg(deviceInfo.Address.rgBytes[4], 2, 16, QChar('0'))
            .arg(deviceInfo.Address.rgBytes[3], 2, 16, QChar('0'))
            .arg(deviceInfo.Address.rgBytes[2], 2, 16, QChar('0'))
            .arg(deviceInfo.Address.rgBytes[1], 2, 16, QChar('0'))
            .arg(deviceInfo.Address.rgBytes[0], 2, 16, QChar('0'))
            .toUpper();
        info.deviceInfo = deviceInfo;
        info.isConnected = deviceInfo.fConnected;
        info.deviceClass = getDeviceClassString(deviceInfo.ulClassofDevice);
        devices.append(info);
    } while (BluetoothFindNextDevice(hFind, &deviceInfo));

    BluetoothFindDeviceClose(hFind);
    return devices;
}

QString Lab6Widget::getDeviceClassString(DWORD deviceClass)
{
    DWORD majorClass = (deviceClass >> 8) & 0x1F;
    switch (majorClass) {
        case 0x01: return "Компьютер";
        case 0x02: return "Телефон";
        case 0x04: return "Аудио";
        case 0x05: return "Периферия";
        case 0x06: return "Изображения";
        case 0x07: return "Носимое";
        default: return "Другое";
    }
}

void Lab6Widget::onScanDevices()
{
    if (!bluetoothInitialized) {
        QMessageBox::critical(this, "Ошибка", "Bluetooth не инициализирован.");
        return;
    }

    scanButton->setEnabled(false);
    appendToLog("Сканирование устройств...", "blue");
    devicesModel->removeRows(0, devicesModel->rowCount());
    discoveredDevices.clear();

    QTimer::singleShot(100, this, [this]() {
        discoveredDevices = scanBluetoothDevices();
        appendToLog(QString("Найдено устройств: %1").arg(discoveredDevices.size()), "green");

        for (const auto& device : discoveredDevices) {
            QList<QStandardItem*> rowItems;
            rowItems.append(new QStandardItem(device.name.isEmpty() ? "Неизвестно" : device.name));
            rowItems.append(new QStandardItem(device.address));
            rowItems.append(new QStandardItem(device.deviceClass));
            QString status = device.deviceInfo.fAuthenticated ? "Сопряжено" : "Не сопряжено";
            rowItems.append(new QStandardItem(status));
            devicesModel->appendRow(rowItems);
        }
        scanButton->setEnabled(true);
    });
}

bool Lab6Widget::connectToDevice(const BluetoothDeviceInfo &device)
{
    appendToLog(QString("Выбор устройства: %1").arg(device.name), "blue");

    if (!device.deviceInfo.fAuthenticated) {
        appendToLog("Устройство не сопряжено!", "orange");
        QMessageBox::warning(this, "Внимание", "Требуется сопряжение в настройках Windows!");
        return false;
    }

    // Здесь мы просто сохраняем устройство как "выбранное".
    // Сервисы Windows активирует сама при передаче.
    appendToLog("Устройство выбрано. Готово к передаче.", "green");
    return true;
}

bool Lab6Widget::disconnectFromDevice(const BluetoothDeviceInfo &device)
{
    appendToLog("Выбор сброшен.", "blue");
    return true;
}

void Lab6Widget::onConnectDevice()
{
    QModelIndexList selection = devicesTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    int row = selection.first().row();
    if (row >= 0 && row < discoveredDevices.size()) {
        BluetoothDeviceInfo device = discoveredDevices[row];
        connectButton->setEnabled(false);

        if (connectToDevice(device)) {
            connectedDevice = new BluetoothDeviceInfo(device);
            disconnectButton->setEnabled(true);
            sendButton->setEnabled(true);
            devicesModel->item(row, 3)->setText("Выбрано");
        } else {
            connectButton->setEnabled(true);
        }
    }
}

void Lab6Widget::onDisconnectDevice()
{
    if (!connectedDevice) return;
    disconnectFromDevice(*connectedDevice);
    delete connectedDevice;
    connectedDevice = nullptr;
    disconnectButton->setEnabled(false);
    sendButton->setEnabled(false);
    connectButton->setEnabled(true);
    appendToLog("Сброс выполнен", "green");
}

void Lab6Widget::launchWindowsTransferWizard()
{
    // К сожалению, fsquirt.exe не принимает аргументов (ни файла, ни адреса устройства).
    // Мы запускаем его с ключом -send, чтобы сразу открыть режим отправки (работает не на всех версиях Windows),
    // но выбирать устройство и файл пользователю все равно придется вручную.

    QProcess *process = new QProcess(this);
    process->startDetached("fsquirt.exe", QStringList() << "-send");
}

void Lab6Widget::onSendFile()
{
    if (!connectedDevice) return;

    // Мы все равно просим выбрать файл, чтобы показать пользователю, что процесс идет,
    // и чтобы залогировать имя файла.
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Выберите аудиофайл",
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
        "Аудио (*.mp3 *.wav *.m4a);;Все (*.*)"
    );

    if (filePath.isEmpty()) return;

    appendToLog(QString("Подготовка к отправке: %1").arg(QFileInfo(filePath).fileName()), "blue");
    appendToLog("Запуск мастера Windows...", "blue");
    appendToLog("⚠️ ВНИМАНИЕ: В открывшемся окне выберите устройство и файл ПОВТОРНО.", "orange");
    appendToLog("(Ограничение Windows: программа fsquirt не принимает параметры)", "gray");

    launchWindowsTransferWizard();
}