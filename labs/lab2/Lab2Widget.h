// Файл: labs/lab2/Lab2Widget.h

#ifndef LAB2WIDGET_H
#define LAB2WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QProcess>

class Lab2Widget : public QWidget {
    Q_OBJECT

public:
    explicit Lab2Widget(QWidget *parent = nullptr);
    ~Lab2Widget() override; // Добавляем деструктор для корректного выключения ВМ

    signals:
        void backToMainScreen();

    private slots:
        void onBackButtonClicked();
    void onRunVmScanClicked(); // Слот для новой кнопки запуска
    void onVmReadyToExecute(); // Слот, который вызовется после 40-секундной задержки

private:
    void setupUi(); // Метод для создания и настройки интерфейса
    void runVmCommand(const QStringList &args); // Общий метод для запуска команд
    void readFileAndPopulateTable(); // Метод для чтения файла и заполнения таблицы
    void stopVm(); // Метод для выключения ВМ

    void addLog(const QString &message); // Утилита для логирования

    // Элементы интерфейса
    QTableWidget *pciTable;
    QTextEdit *logDisplay;
    QPushButton *backButton;
    QPushButton *runVmScanButton; // Новая кнопка

    // Управление процессами
    QProcess *vboxProcess;
    bool isVmRunning; // Флаг, что мы запустили ВМ и должны ее остановить
};

#endif // LAB2WIDGET_H