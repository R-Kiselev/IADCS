#ifndef LAB4WIDGET_H
#define LAB4WIDGET_H

#include <QWidget>
#include <QCamera>
#include <QImageCapture>
#include <QMediaRecorder>
#include <QVideoWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QMediaCaptureSession>
#include <QTableWidget>
#include <QSystemTrayIcon>
#include <QMenu>

// Определения для глобальных горячих клавиш
#define HOTKEY_ID_F1 1
#define HOTKEY_ID_F2 2
#define HOTKEY_ID_F3 3
#define HOTKEY_ID_F4 4

class Lab4Widget : public QWidget {
    Q_OBJECT

public:
    explicit Lab4Widget(QWidget *parent = nullptr);
    ~Lab4Widget() override;

    signals:
        void backToMainScreen();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    private slots:
        void takePhoto();
    void startVideoRecording();
    void stopVideoRecording();
    void toggleStealthMode();
    void onBackButtonClicked();
    void showWindowFromTray();

private:
    void setupUi();
    void setupCamera();
    void registerHotkeys();
    void unregisterHotkeys();
    void addLog(const QString &message);
    void updateCameraInfo();
    QString getOutputPath();

    QLabel *titleLabel;
    QTableWidget *cameraInfoTable;
    QVideoWidget *videoWidget;
    QTextEdit *logDisplay;
    QPushButton *backButton;
    QPushButton *photoButton;
    QPushButton *startVideoButton;
    QPushButton *stopVideoButton;
    QPushButton *stealthModeButton;
    QLabel* stealthStatusLabel;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QCamera *camera = nullptr;
    QMediaCaptureSession *captureSession = nullptr;
    QImageCapture *imageCapture = nullptr;
    QMediaRecorder *mediaRecorder = nullptr;

    bool isStealthMode = false;
    Qt::WindowFlags originalFlags;
};

#endif // LAB4WIDGET_H