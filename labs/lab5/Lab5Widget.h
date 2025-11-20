#ifndef Lab5Widget_H
#define Lab5Widget_H

#include <QWidget>
#include <QSet>

#include <windows.h>
#include <dbt.h>
#include <SetupAPI.h>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QTableView;
class QTextEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE


class Lab5Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Lab5Widget(QWidget *parent = nullptr);
    ~Lab5Widget();

    signals:
        void backToMainScreen(); // <--- ПЕРЕИМЕНОВАНО ДЛЯ СООТВЕТСТВИЯ

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    private slots:
        void onEjectDeviceClicked();

private:
    // --- Методы ---
    void setupUi();
    void registerDeviceNotifications();
    void unregisterDeviceNotifications();
    void populateExistingDevices();
    void addDevice(const QString &devicePath);
    void removeDevice(const QString &devicePath, bool isSafeRemoval);
    QString getDeviceProperty(HDEVINFO devInfo, SP_DEVINFO_DATA &devData, DWORD property);
    QString getDeviceInstanceId(HDEVINFO devInfo, SP_DEVINFO_DATA &devData);
    QString getVolumeInfo(const QString &instanceId);
    bool ejectDevice(const QString &devicePath);
    void appendToLog(const QString& message, const QString& color = "#cccccc"); // Цвет по умолчанию изменен

    // --- UI Элементы ---
    QLabel *titleLabel;
    QTableView *devicesTableView;
    QStandardItemModel *devicesModel;
    QPushButton *ejectButton;
    QTextEdit *logOutput;
    QPushButton *backButton;

    // --- Специфичные для Windows переменные ---
    HDEVNOTIFY hDeviceNotify;
    QSet<QString> pendingRemovalDevices;
};

#endif // Lab5Widget_H