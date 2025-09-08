#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QEvent>
#include <QRegularExpression>
#include <QTransform>
#include <QStackedWidget> // Добавляем QStackedWidget

class MyMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MyMainWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    private slots: // Добавляем слоты для кнопок
        void onLab1ButtonClicked();
    // Добавьте сюда слоты для других лабораторных работ по мере их реализации

private:
    QLabel *perryLabel;
    QPixmap originalPerryPixmap;
    QPixmap kickPerryPixmap;
    QStackedWidget *stackedWidget; // Объявляем QStackedWidget

    QPixmap getTransformedKickPerry(QPushButton *button, bool reflectHorizontal);
    void updatePerryState(QPushButton *button, bool hovered);

    // Добавим приватные указатели на виджеты ЛР, чтобы управлять их жизненным циклом
    QWidget *mainScreenWidget; // Виджет для главного экрана с кнопками и Перри
};

#endif // MYMAINWINDOW_H