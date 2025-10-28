#include "Lab3Widget.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QLabel>

const QString VBOX_MANAGE_PATH_LAB3 = "C:/VirtualBox/VBoxManage.exe";
const QString VM_NAME_LAB3 = "winxp";
const QString GUEST_EXECUTABLE_PATH_LAB3 = "Z:/lab3/lab3.exe";
const QString GUEST_USERNAME_LAB3 = "Administrator";
const QString GUEST_PASSWORD_LAB3 = "pass";
const QString DISK_INFO_FILE_PATH = "D:/Study/ThirdCourse/IIUVM/dropbox/disk_info.txt";
const int VM_BOOT_WAIT_MS_LAB3 = 40000;

Lab3Widget::Lab3Widget(QWidget *parent) : QWidget(parent), isVmRunning(false) {
    vboxProcess = new QProcess(this);
    setupUi();
    this->setStyleSheet("background-color: #3c3c3c;");
}

Lab3Widget::~Lab3Widget() {
    if (isVmRunning) {
        stopVm();
    }
}

void Lab3Widget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setAlignment(Qt::AlignTop);

    QLabel *titleLabel = new QLabel("Лабораторная работа №3: Жесткий диск", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    infoTable = new QTableWidget(this);
    infoTable->setColumnCount(2);
    infoTable->setHorizontalHeaderLabels({"Параметр", "Значение"});
    infoTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #2b2b2b;"
        "   color: #e0e0e0;"
        "   border: 1px solid #555;"
        "   border-radius: 8px;"
        "   gridline-color: #444;"
        "   font-size: 14px;"
        "}"
        "QTableWidget::item { padding: 8px; }"
        "QHeaderView::section {"
        "   background-color: #007bff;"
        "   color: white; font-weight: bold; padding: 8px;"
        "   border: none; border-right: 1px solid #0056b3;"
        "}"
        "QTableWidget::item:selected { background-color: #0056b3; }"
        "QTableCornerButton::section { background-color: #007bff; }"
    );
    infoTable->verticalHeader()->setVisible(false);
    infoTable->horizontalHeader()->setStretchLastSection(true);
    infoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    infoTable->setColumnWidth(0, 180);
    infoTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList keys = {"Model", "Manufacturer", "SerialNumber", "Firmware", "TotalGB", "UsedGB", "FreeGB", "Interface", "Modes"};
    infoTable->setRowCount(keys.size());
    for (int i = 0; i < keys.size(); ++i) {
        infoTable->setItem(i, 0, new QTableWidgetItem(keys[i]));
        infoTable->setItem(i, 1, new QTableWidgetItem("N/A"));
    }

    mainLayout->addWidget(infoTable);
    infoTable->setFixedHeight(380);

    mainLayout->addSpacing(20);

    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);
    logDisplay->setStyleSheet(
        "QTextEdit { background-color: #2b2b2b; color: #ccc; border: 1px solid #555; border-radius: 8px; font-family: 'Consolas', monospace; }"
    );
    mainLayout->addWidget(logDisplay, 1);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    backButton = new QPushButton("Назад", this);
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet("QPushButton { font-size: 16px; background-color: #dc3545; color: white; border-radius: 10px; border: none; } QPushButton:hover { background-color: #c82333; }");

    runVmScanButton = new QPushButton("Получить информацию о диске", this);
    runVmScanButton->setFixedHeight(40);
    runVmScanButton->setStyleSheet("QPushButton { font-size: 16px; background-color: #007bff; color: white; border-radius: 10px; border: none; } QPushButton:hover { background-color: #0056b3; }");

    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(runVmScanButton);
    mainLayout->addLayout(buttonLayout);

    connect(backButton, &QPushButton::clicked, this, &Lab3Widget::onBackButtonClicked);
    connect(runVmScanButton, &QPushButton::clicked, this, &Lab3Widget::onRunVmScanClicked);

    addLog("Готов к запуску. Нажмите 'Получить информацию о диске'.");
}

void Lab3Widget::onRunVmScanClicked() {
    runVmScanButton->setEnabled(false);
    addLog("--- Начало операции ---");

    for (int i = 0; i < infoTable->rowCount(); ++i) {
        infoTable->setItem(i, 1, new QTableWidgetItem("..."));
    }

    if (isVmRunning) {
        addLog("ВМ уже запущена. Запускаю сканирование...");
        onVmReadyToExecute();
        return;
    }

    addLog("Шаг 1: Запуск виртуальной машины '" + VM_NAME_LAB3 + "'...");
    vboxProcess->start(VBOX_MANAGE_PATH_LAB3, {"startvm", VM_NAME_LAB3, "--type", "headless"});
    vboxProcess->waitForFinished(10000);

    if (vboxProcess->exitCode() != 0) {
        addLog("Предполагаю, что ВМ уже была запущена. Продолжаю...");
    }

    isVmRunning = true;
    addLog("ВМ успешно запущена (или уже была запущена).");
    addLog(QString("Шаг 2: Ожидание загрузки ОС (%1 секунд)...").arg(VM_BOOT_WAIT_MS_LAB3 / 1000));
    QTimer::singleShot(VM_BOOT_WAIT_MS_LAB3, this, &Lab3Widget::onVmReadyToExecute);
}

void Lab3Widget::onVmReadyToExecute() {
    addLog("Шаг 3: Запуск " + GUEST_EXECUTABLE_PATH_LAB3 + " внутри ВМ...");
    QStringList args = {"guestcontrol", VM_NAME_LAB3, "run",
                        "--exe", GUEST_EXECUTABLE_PATH_LAB3,
                        "--username", GUEST_USERNAME_LAB3,
                        "--password", GUEST_PASSWORD_LAB3,
                        "--wait-stdout"};

    vboxProcess->start(VBOX_MANAGE_PATH_LAB3, args);
    vboxProcess->waitForFinished(60000);

    if (vboxProcess->exitCode() != 0) {
        addLog("ОШИБКА: Не удалось выполнить команду в гостевой ОС.");
        addLog("Stderr: " + vboxProcess->readAllStandardError());
        addLog("Stdout: " + vboxProcess->readAllStandardOutput());
        runVmScanButton->setEnabled(true);
        return;
    }

    addLog("Команда в ВМ выполнена успешно.");
    addLog("Шаг 4: Чтение и обработка файла результатов...");
    parseDiskInfoFile();

    addLog("--- Операция завершена ---");
    addLog("ВМ остается запущенной. Нажмите 'Назад' для выхода и выключения.");
    runVmScanButton->setEnabled(true);
}

void Lab3Widget::parseDiskInfoFile() {
    QFile file(DISK_INFO_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        addLog("ОШИБКА: Не удалось открыть файл результатов: " + DISK_INFO_FILE_PATH);
        return;
    }

    addLog("Файл " + DISK_INFO_FILE_PATH + " успешно открыт.");
    QTextStream in(&file);
    int linesParsed = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        int separatorIndex = line.indexOf(':');
        if (separatorIndex != -1) {
            QString key = line.left(separatorIndex).trimmed();
            QString value = line.mid(separatorIndex + 1).trimmed();

            for (int i = 0; i < infoTable->rowCount(); ++i) {
                if (infoTable->item(i, 0) && infoTable->item(i, 0)->text() == key) {
                    infoTable->setItem(i, 1, new QTableWidgetItem(value));
                    linesParsed++;
                    break;
                }
            }
        }
    }
    file.close();
    addLog(QString("Данные загружены. Обработано строк: %1.").arg(linesParsed));
}

void Lab3Widget::stopVm() {
    if (!isVmRunning) return;

    addLog("Выключение виртуальной машины...");
    QProcess stopper;
    stopper.execute(VBOX_MANAGE_PATH_LAB3, {"controlvm", VM_NAME_LAB3, "poweroff"});
    addLog("Виртуальная машина выключена.");
    isVmRunning = false;
}

void Lab3Widget::onBackButtonClicked() {
    if (isVmRunning) {
        stopVm();
    }
    emit backToMainScreen();
}

void Lab3Widget::addLog(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logDisplay->append(QString("[%1] %2").arg(timestamp, message));
}