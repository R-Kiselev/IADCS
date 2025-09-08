#ifndef LAB1WIDGET_H
#define LAB1WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QTextEdit>
#include <QMessageBox>
#include <QDebug>

// --- Обязательные определения для Windows API ПЕРЕД #include <windows.h> ---
#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
// --- Конец обязательных определений ---

#include <windows.h>
#include <powrprof.h>
#include <setupapi.h>
#include <devguid.h>
#include <batclass.h>


class Lab1Widget : public QWidget {
    Q_OBJECT

public:
    explicit Lab1Widget(QWidget *parent = nullptr);
    ~Lab1Widget();

    signals:
        void backToMainScreen();

    private slots:
        void updatePowerStatus();
    void onSleepButtonClicked();
    void onHibernateButtonClicked();
    void onBackButtonClicked();

private:
    QLabel *powerSourceLabel;
    QLabel *batteryTypeLabel;
    QLabel *batteryLevelLabel;
    QLabel *powerSavingModeLabel;
    QLabel *batteryLifeTimeFullLabel;             // Остается для системной оценки
    QLabel *elapsedTimeSinceDisconnectLabel; // НОВОЕ НАЗВАНИЕ: сколько прошло времени

    QTimer *updateTimer;

    SYSTEM_POWER_STATUS lastPowerStatus;
    qint64 disconnectTimestamp = 0; // Время в мс, когда зарядка была отключена

    QString formatSecondsToHMS(qint64 seconds) const;
    QString getBatteryChemistry() const;
};

#endif // LAB1WIDGET_H