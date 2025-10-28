#ifndef LAB2WIDGET_H
#define LAB2WIDGET_H

#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QProcess>

class Lab2Widget : public QWidget {
    Q_OBJECT

public:
    explicit Lab2Widget(QWidget *parent = nullptr);
    ~Lab2Widget() override;

    signals:
        void backToMainScreen();

    private slots:
        void onBackButtonClicked();
    void onRunVmScanClicked();
    void onVmReadyToExecute();

private:
    void setupUi();
    void runVmCommand(const QStringList &args);
    void readFileAndPopulateTable();
    void stopVm();

    void addLog(const QString &message);

    QTableWidget *pciTable;
    QTextEdit *logDisplay;
    QPushButton *backButton;
    QPushButton *runVmScanButton;

    QProcess *vboxProcess;
    bool isVmRunning;
};

#endif