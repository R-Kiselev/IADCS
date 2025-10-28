#ifndef LAB3WIDGET_H
#define LAB3WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QProcess>
#include <QTableWidget> // <--- Добавлено

class Lab3Widget : public QWidget {
    Q_OBJECT

public:
    explicit Lab3Widget(QWidget *parent = nullptr);
    ~Lab3Widget() override;

    signals:
        void backToMainScreen();

    private slots:
        void onBackButtonClicked();
    void onRunVmScanClicked();
    void onVmReadyToExecute();

private:
    void setupUi();
    void parseDiskInfoFile();
    void stopVm();
    void addLog(const QString &message);

    // UI Elements
    QTextEdit *logDisplay;
    QPushButton *backButton;
    QPushButton *runVmScanButton;
    QTableWidget *infoTable; // <--- Заменено

    // Process Management
    QProcess *vboxProcess;
    bool isVmRunning;
};

#endif // LAB3WIDGET_H