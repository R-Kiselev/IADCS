#include <QApplication>
#include "MyMainWindow.h" // Включаем наш новый класс MyMainWindow

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    MyMainWindow window; // Создаем экземпляр нашего главного окна
    window.show();       // Отображаем окно

    return a.exec();     // Запускаем цикл обработки событий приложения
}