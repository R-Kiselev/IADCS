#include "Lab5Widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QTextEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardItemModel>
#include <QDebug>
#include <QRegularExpression>

#include <initguid.h>
#include <usbiodef.h>
#include <Cfgmgr32.h>

#ifndef DBT_DEVICEQUERYREMOVEFAILED
#define DBT_DEVICEQUERYREMOVEFAILED 0x8002
#endif


Lab5Widget::Lab5Widget(QWidget *parent)
    : QWidget(parent), hDeviceNotify(nullptr)
{
    setupUi();
    populateExistingDevices();
    registerDeviceNotifications();
}

Lab5Widget::~Lab5Widget()
{
    unregisterDeviceNotifications();
}

void Lab5Widget::setupUi()
{
    this->setStyleSheet("background-color: #3c3c3c;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setAlignment(Qt::AlignTop);

    titleLabel = new QLabel("Лабораторная работа №5: Мониторинг USB", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    // Модель для таблицы
    devicesModel = new QStandardItemModel(0, 3, this);
    devicesModel->setHorizontalHeaderLabels({"Устройство", "Производитель", "ID Устройства"});

    // Таблица
    devicesTableView = new QTableView(this);
    devicesTableView->setModel(devicesModel);
    devicesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    devicesTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    devicesTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    devicesTableView->verticalHeader()->setVisible(false);

    // ================== ИЗМЕНЕНИЯ ЗДЕСЬ ==================
    // Устанавливаем режим изменения размера для каждого столбца,
    // чтобы обеспечить полное отображение содержимого.
    QHeaderView *header = devicesTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // "Устройство" по содержимому
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // "Производитель" по содержимому
    header->setSectionResizeMode(2, QHeaderView::Stretch);          // "ID Устройства" растягивается
    // ======================================================

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
    mainLayout->addWidget(devicesTableView, 1); // Растягиваем таблицу

    // Лог
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(150); // Ограничиваем высоту лога
    logOutput->setStyleSheet(
        "QTextEdit { background-color: #2b2b2b; color: #ccc; border: 1px solid #555; border-radius: 8px; font-family: 'Consolas', monospace; }"
    );
    mainLayout->addWidget(logOutput);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    backButton = new QPushButton("Назад", this);
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet("QPushButton { font-size: 16px; background-color: #dc3545; color: white; border-radius: 10px; border: none; } QPushButton:hover { background-color: #c82333; }");

    ejectButton = new QPushButton("Извлечь устройство", this);
    ejectButton->setFixedSize(250, 40);
    ejectButton->setStyleSheet("QPushButton { font-size: 16px; background-color: #007bff; color: white; border-radius: 10px; border: none; } QPushButton:hover { background-color: #0056b3; }");

    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(ejectButton);
    mainLayout->addLayout(buttonLayout);

    connect(backButton, &QPushButton::clicked, this, &Lab5Widget::backToMainScreen);
    connect(ejectButton, &QPushButton::clicked, this, &Lab5Widget::onEjectDeviceClicked);

    appendToLog("Готов к работе. Ожидание подключения USB-устройств...", "gray");
}


void Lab5Widget::appendToLog(const QString& message, const QString& color) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logOutput->append(QString("[%1] <font color='%2'>%3</font>").arg(timestamp, color, message));
}

void Lab5Widget::registerDeviceNotifications()
{
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
    ZeroMemory(&notificationFilter, sizeof(notificationFilter));
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notificationFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

    hDeviceNotify = RegisterDeviceNotification((HANDLE)this->winId(), &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

    if (!hDeviceNotify) {
        appendToLog("Ошибка: Не удалось зарегистрировать уведомления об устройствах.", "red");
    } else {
        appendToLog("Мониторинг USB-портов запущен.", "gray");
    }
}

void Lab5Widget::unregisterDeviceNotifications()
{
    if (hDeviceNotify) {
        UnregisterDeviceNotification(hDeviceNotify);
        hDeviceNotify = nullptr;
    }
}

bool Lab5Widget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_DEVICECHANGE) {
        PDEV_BROADCAST_HDR hdr = (PDEV_BROADCAST_HDR)msg->lParam;
        if (hdr && hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
            PDEV_BROADCAST_DEVICEINTERFACE devInterface = (PDEV_BROADCAST_DEVICEINTERFACE)hdr;
            QString devicePath = QString::fromWCharArray(devInterface->dbcc_name).toLower();

            switch (msg->wParam) {
                case DBT_DEVICEARRIVAL:
                    addDevice(devicePath);
                    break;
                case DBT_DEVICEREMOVECOMPLETE: {
                    bool wasSafe = pendingRemovalDevices.contains(devicePath);
                    removeDevice(devicePath, wasSafe);
                    pendingRemovalDevices.remove(devicePath);
                    break;
                }
                case DBT_DEVICEQUERYREMOVE:
                    appendToLog("Запрос на безопасное извлечение...", "#DAA520"); // goldenrod
                    pendingRemovalDevices.insert(devicePath);
                    break;
                case DBT_DEVICEQUERYREMOVEFAILED:
                    appendToLog("ОТКАЗ: Безопасное извлечение отменено системой.", "red");
                    pendingRemovalDevices.remove(devicePath);
                    break;
            }
        }
        *result = TRUE;
        return true;
    }
    return QWidget::nativeEvent(eventType, message, result);
}


void Lab5Widget::addDevice(const QString &devicePath)
{
    HDEVINFO devInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfo == INVALID_HANDLE_VALUE) return;

    SP_DEVICE_INTERFACE_DATA devInterfaceData;
    devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    if (!SetupDiOpenDeviceInterfaceW(devInfo, devicePath.toStdWString().c_str(), 0, &devInterfaceData)) {
        SetupDiDestroyDeviceInfoList(devInfo);
        return;
    }

    SP_DEVINFO_DATA devData;
    devData.cbSize = sizeof(SP_DEVINFO_DATA);
    SetupDiGetDeviceInterfaceDetailW(devInfo, &devInterfaceData, NULL, 0, NULL, &devData);

    QString description = getDeviceProperty(devInfo, devData, SPDRP_DEVICEDESC);
    QString manufacturer = getDeviceProperty(devInfo, devData, SPDRP_MFG);
    QString instanceId = getDeviceInstanceId(devInfo, devData);

    for (int i = 0; i < devicesModel->rowCount(); ++i) {
        if (devicesModel->item(i, 2)->data(Qt::UserRole).toString().toLower() == devicePath) {
            SetupDiDestroyDeviceInfoList(devInfo);
            return;
        }
    }

    appendToLog(QString("ПОДКЛЮЧЕНО: %1").arg(description), "#28a745"); // green

    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(description));
    rowItems.append(new QStandardItem(manufacturer));
    QStandardItem* pathItem = new QStandardItem(instanceId);
    pathItem->setData(devicePath, Qt::UserRole);
    rowItems.append(pathItem);

    devicesModel->appendRow(rowItems);
    SetupDiDestroyDeviceInfoList(devInfo);
}

void Lab5Widget::removeDevice(const QString &devicePath, bool isSafeRemoval)
{
    for (int i = 0; i < devicesModel->rowCount(); ++i) {
        if (devicesModel->item(i, 2)->data(Qt::UserRole).toString().toLower() == devicePath) {
            QString desc = devicesModel->item(i, 0)->text();
            if(isSafeRemoval) {
                 appendToLog(QString("ИЗВЛЕЧЕНО (безопасно): %1").arg(desc), "#00BFFF"); // deep sky blue
            } else {
                 appendToLog(QString("ОТКЛЮЧЕНО (небезопасно): %1").arg(desc), "orange");
            }
            devicesModel->removeRow(i);
            break;
        }
    }
}

void Lab5Widget::onEjectDeviceClicked()
{
    QModelIndexList selection = devicesTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Внимание");
        msgBox.setText("Пожалуйста, выберите устройство для извлечения.");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #3c3c3c; }"
            "QMessageBox QLabel { color: #e0e0e0; font-size: 14px; min-width: 250px;}"
            "QPushButton { background-color: #007bff; color: white; border-radius: 10px; border: none; min-width: 80px; padding: 8px;}"
            "QPushButton:hover { background-color: #0056b3; }"
        );
        msgBox.exec();
        return;
    }

    int row = selection.first().row();
    QString devicePath = devicesModel->item(row, 2)->data(Qt::UserRole).toString();
    QString deviceName = devicesModel->item(row, 0)->text();

    appendToLog(QString("Попытка безопасного извлечения '%1'...").arg(deviceName), "#DAA520");

    pendingRemovalDevices.insert(devicePath);

    if (!ejectDevice(devicePath)) {
        pendingRemovalDevices.remove(devicePath);
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Ошибка");
        msgBox.setText("Не удалось безопасно извлечь устройство.\nВозможно, оно используется другой программой.");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #3c3c3c; }"
            "QMessageBox QLabel { color: #e0e0e0; font-size: 14px; min-width: 300px; }"
            "QPushButton { background-color: #dc3545; color: white; border-radius: 10px; border: none; min-width: 80px; padding: 8px;}"
            "QPushButton:hover { background-color: #c82333; }"
        );
        msgBox.exec();
        appendToLog(QString("Ошибка извлечения '%1'. Устройство занято.").arg(deviceName), "red");
    }
}

// ... (остальные методы: getDeviceProperty, getDeviceInstanceId, ejectDevice, populateExistingDevices и т.д. остаются без изменений) ...

QString Lab5Widget::getDeviceProperty(HDEVINFO devInfo, SP_DEVINFO_DATA &devData, DWORD property)
{
    DWORD dataType;
    DWORD bufferSize = 0;
    SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, property, &dataType, NULL, 0, &bufferSize);
    if (bufferSize == 0) return "N/A";

    QVector<BYTE> buffer(bufferSize);
    if (SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, property, &dataType, buffer.data(), bufferSize, NULL)) {
        return QString::fromWCharArray((const wchar_t*)buffer.data());
    }
    return "N/A";
}

QString Lab5Widget::getDeviceInstanceId(HDEVINFO devInfo, SP_DEVINFO_DATA &devData)
{
    WCHAR instanceId[MAX_DEVICE_ID_LEN];
    if (SetupDiGetDeviceInstanceIdW(devInfo, &devData, instanceId, MAX_DEVICE_ID_LEN, NULL)) {
        return QString::fromWCharArray(instanceId);
    }
    return "N/A";
}

QString Lab5Widget::getVolumeInfo(const QString &instanceId) { return ""; } // В данной версии не используется

bool Lab5Widget::ejectDevice(const QString &devicePath)
{
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) return false;

    SP_DEVICE_INTERFACE_DATA oInterface;
    oInterface.cbSize = sizeof(oInterface);
    if (!SetupDiOpenDeviceInterfaceW(hDevInfo, devicePath.toStdWString().c_str(), 0, &oInterface)) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    SP_DEVINFO_DATA oDevInfo;
    oDevInfo.cbSize = sizeof(oDevInfo);
    SetupDiGetDeviceInterfaceDetailW(hDevInfo, &oInterface, NULL, 0, NULL, &oDevInfo);

    PNP_VETO_TYPE VetoType = PNP_VetoTypeUnknown;
    WCHAR VetoNameW[MAX_PATH] = {0};
    CONFIGRET cr = CM_Request_Device_EjectW(oDevInfo.DevInst, &VetoType, VetoNameW, MAX_PATH, 0);

    SetupDiDestroyDeviceInfoList(hDevInfo);

    if (cr == CR_SUCCESS) {
        return true;
    } else {
        qDebug() << "CM_Request_Device_EjectW failed with code:" << cr;
        return false;
    }
}

void Lab5Widget::populateExistingDevices() {
    devicesModel->removeRows(0, devicesModel->rowCount());
    HDEVINFO devInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devInfo == INVALID_HANDLE_VALUE) return;

    SP_DEVICE_INTERFACE_DATA devInterfaceData;
    devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(devInfo, NULL, &GUID_DEVINTERFACE_USB_DEVICE, i, &devInterfaceData); ++i) {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailW(devInfo, &devInterfaceData, NULL, 0, &requiredSize, NULL);

        SP_DEVICE_INTERFACE_DETAIL_DATA_W *devInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA_W *)malloc(requiredSize);
        if(!devInterfaceDetailData) continue;
        devInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        if (SetupDiGetDeviceInterfaceDetailW(devInfo, &devInterfaceData, devInterfaceDetailData, requiredSize, NULL, NULL)) {
            QString devicePath = QString::fromWCharArray(devInterfaceDetailData->DevicePath);
            addDevice(devicePath.toLower());
        }
        free(devInterfaceDetailData);
    }
    SetupDiDestroyDeviceInfoList(devInfo);
}