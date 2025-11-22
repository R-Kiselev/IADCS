#ifndef LAB6WIDGET_H
#define LAB6WIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QProcess>

// ВАЖНО: winsock2.h должен быть включен ПЕРЕД windows.h
#include <winsock2.h>
#include <windows.h>
#include <bluetoothapis.h>

struct BluetoothDeviceInfo {
    QString name;
    QString address;
    BLUETOOTH_DEVICE_INFO deviceInfo;
    bool isConnected;
    QString deviceClass;
};

class Lab6Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Lab6Widget(QWidget *parent = nullptr);
    ~Lab6Widget();

    signals:
        void backToMainScreen();

    private slots:
        void onScanDevices();
    void onSendFile();
    void onConnectDevice();
    void onDisconnectDevice();
    void onBackButtonClicked();

private:
    void setupUi();
    void appendToLog(const QString& message, const QString& color = "#cccccc");

    // Bluetooth функции
    bool initializeBluetooth();
    QList<BluetoothDeviceInfo> scanBluetoothDevices();
    bool connectToDevice(const BluetoothDeviceInfo &device);
    bool disconnectFromDevice(const BluetoothDeviceInfo &device);
    void launchWindowsTransferWizard(); // Запуск fsquirt
    QString getDeviceClassString(DWORD deviceClass);

    // UI элементы
    QLabel *titleLabel;
    QTableView *devicesTableView;
    QStandardItemModel *devicesModel;

    QPushButton *scanButton;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *sendButton;
    QPushButton *backButton;

    QTextEdit *logOutput;

    // Данные
    QList<BluetoothDeviceInfo> discoveredDevices;
    BluetoothDeviceInfo *connectedDevice;
    HANDLE hRadio;
    bool bluetoothInitialized;
};

#endif // LAB6WIDGET_H