#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QEvent>
#include <QRegularExpression>
#include <QStackedWidget>

class MyMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MyMainWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    private slots:
        void onLab1ButtonClicked();
    void onLab2ButtonClicked();
    void onLab3ButtonClicked();
    void onLab4ButtonClicked();
    void onLab5ButtonClicked(); // <-- ДОБАВИТЬ ЭТОТ СЛОТ
    void onLab6ButtonClicked(); // <--- ДОБАВИТЬ


private:
    QLabel *perryLabel;
    QPixmap originalPerryPixmap;
    QPixmap kickPerryPixmap;
    QStackedWidget *stackedWidget;

    QPixmap getTransformedKickPerry(QPushButton *button, bool reflectHorizontal);
    void updatePerryState(QPushButton *button, bool hovered);

    QWidget *mainScreenWidget;
};

#endif