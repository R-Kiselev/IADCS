#include "Lab2Widget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

const QString VBOX_MANAGE_PATH = "C:/VirtualBox/VBoxManage.exe";
const QString VM_NAME = "winxp";
const QString GUEST_EXECUTABLE_PATH = "Z:/lab2/lab2.exe";
const QString GUEST_USERNAME = "Administrator";
const QString GUEST_PASSWORD = "pass";
const QString PCI_LIST_FILE_PATH = "D:/Study/ThirdCourse/IIUVM/dropbox/pci_list.txt";
const int VM_BOOT_WAIT_MS = 40000;


Lab2Widget::Lab2Widget(QWidget *parent) : QWidget(parent), isVmRunning(false) {
    vboxProcess = new QProcess(this);
    setupUi();
    this->setStyleSheet("background-color: #3c3c3c;");
}

Lab2Widget::~Lab2Widget() {
    if (isVmRunning) {
        stopVm();
    }
}

void Lab2Widget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setAlignment(Qt::AlignTop);

    QLabel *titleLabel = new QLabel("Лабораторная работа №2: Шина PCI", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    pciTable = new QTableWidget(this);
    pciTable->setColumnCount(3);
    pciTable->setHorizontalHeaderLabels({"Vendor ID", "Device ID", "Device Class"});
    pciTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #2b2b2b;"
        "   color: #e0e0e0;"
        "   border: 1px solid #555;"
        "   border-radius: 8px;"
        "   gridline-color: #444;"
        "   font-size: 14px;"
        "}"
        "QTableWidget::item { padding: 5px; }"
        "QHeaderView::section {"
        "   background-color: #007bff;"
        "   color: white; font-weight: bold; padding: 8px;"
        "   border: none; border-right: 1px solid #0056b3;"
        "}"
        "QTableWidget::item:selected { background-color: #0056b3; }"
        "QTableCornerButton::section { background-color: #007bff; }"
    );
    pciTable->setAlternatingRowColors(true);
    pciTable->verticalHeader()->setVisible(false);
    pciTable->horizontalHeader()->setStretchLastSection(true);
    
    mainLayout->addWidget(pciTable, 3);

    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);
    logDisplay->setStyleSheet(
        "QTextEdit {"
        "   background-color: #2b2b2b; color: #ccc;"
        "   border: 1px solid #555; border-radius: 8px;"
        "   font-family: 'Consolas', 'Courier New', monospace; font-size: 13px;"
        "}"
    );
    
    mainLayout->addWidget(logDisplay, 1);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    backButton = new QPushButton("Назад", this);
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #dc3545; color: white; border-radius: 10px; border: none; }"
        "QPushButton:hover { background-color: #c82333; }"
        "QPushButton:pressed { background-color: #a71d2a; }"
    );

    runVmScanButton = new QPushButton("Запустить сканирование в VM", this);
    runVmScanButton->setFixedHeight(40);
    runVmScanButton->setStyleSheet(
        "QPushButton { font-size: 16px; background-color: #28a745; color: white; border-radius: 10px; border: none; }"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:pressed { background-color: #1e7e34; }"
    );

    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(runVmScanButton);
    mainLayout->addLayout(buttonLayout);

    connect(backButton, &QPushButton::clicked, this, &Lab2Widget::onBackButtonClicked);
    connect(runVmScanButton, &QPushButton::clicked, this, &Lab2Widget::onRunVmScanClicked);

    addLog("Готов к запуску. Нажмите 'Запустить сканирование в VM'.");
}


void Lab2Widget::onRunVmScanClicked() {
    runVmScanButton->setEnabled(false);
    pciTable->setRowCount(0);
    addLog("--- Начало операции ---");

    if (isVmRunning) {
        addLog("ВМ уже запущена. Запускаю сканирование...");
        onVmReadyToExecute();
        return;
    }

    addLog("Шаг 1: Запуск виртуальной машины '" + VM_NAME + "'...");
    vboxProcess->start(VBOX_MANAGE_PATH, {"startvm", VM_NAME, "--type", "headless"});
    vboxProcess->waitForFinished(10000);

    if (vboxProcess->exitCode() != 0) {
        addLog("ВМ уже запущена. Запускаю сканирование...");
    }

    isVmRunning = true;
    addLog("ВМ успешно запущена.");
    addLog(QString("Шаг 2: Ожидание загрузки ОС (%1 секунд)...").arg(VM_BOOT_WAIT_MS / 1000));
    QTimer::singleShot(VM_BOOT_WAIT_MS, this, &Lab2Widget::onVmReadyToExecute);
}

void Lab2Widget::onVmReadyToExecute() {
    addLog("Шаг 3: Запуск " + GUEST_EXECUTABLE_PATH + " внутри ВМ...");
    QStringList args = {"guestcontrol", VM_NAME, "run",
                        "--exe", GUEST_EXECUTABLE_PATH,
                        "--username", GUEST_USERNAME,
                        "--password", GUEST_PASSWORD,
                        "--wait-stdout"};

    vboxProcess->start(VBOX_MANAGE_PATH, args);
    vboxProcess->waitForFinished(60000);

    if (vboxProcess->exitCode() != 0) {
        addLog("ОШИБКА: Не удалось выполнить команду в гостевой ОС.");

        QString stdErr = vboxProcess->readAllStandardError();
        QString stdOut = vboxProcess->readAllStandardOutput();

        if (!stdErr.isEmpty()) {
            addLog("Standard Error: " + stdErr);
        }
        if (!stdOut.isEmpty()) {
            addLog("Standard Output: " + stdOut);
        }

        runVmScanButton->setEnabled(true);
        return;
    }

    addLog("Команда в ВМ выполнена успешно.");
    addLog("Шаг 4: Чтение файла результатов...");
    readFileAndPopulateTable();

    addLog("--- Сканирование завершено ---");
    addLog("ВМ остается запущенной. Нажмите 'Назад' для выхода и выключения.");
    runVmScanButton->setEnabled(true);
}

void Lab2Widget::readFileAndPopulateTable() {
    QFile file(PCI_LIST_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        addLog("ОШИБКА: Не удалось открыть файл результатов: " + PCI_LIST_FILE_PATH);
        return;
    }

    addLog("Файл " + PCI_LIST_FILE_PATH + " успешно открыт.");
    pciTable->setRowCount(0);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.startsWith('|') || line.contains("VendorID")) {
            continue;
        }
        QStringList parts = line.split('|', Qt::SkipEmptyParts);
        if (parts.size() >= 6) {
            QString vendorId = parts[3].trimmed();
            QString deviceId = parts[4].trimmed();
            QString deviceClass = parts[5].trimmed();

            int currentRow = pciTable->rowCount();
            pciTable->insertRow(currentRow);
            pciTable->setItem(currentRow, 0, new QTableWidgetItem(vendorId));
            pciTable->setItem(currentRow, 1, new QTableWidgetItem(deviceId));
            pciTable->setItem(currentRow, 2, new QTableWidgetItem(deviceClass));
        }
    }
    file.close();
    addLog(QString("Данные загружены в таблицу. Найдено устройств: %1").arg(pciTable->rowCount()));
}

void Lab2Widget::stopVm() {
    if (!isVmRunning) return;

    addLog("Выключение виртуальной машины...");
    QProcess stopper;
    stopper.execute(VBOX_MANAGE_PATH, {"controlvm", VM_NAME, "poweroff"});
    addLog("Виртуальная машина выключена.");
    isVmRunning = false;
}

void Lab2Widget::onBackButtonClicked() {
    if (isVmRunning) {
        stopVm();
    }
    emit backToMainScreen();
}

void Lab2Widget::addLog(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logDisplay->append(QString("[%1] %2").arg(timestamp, message));
}