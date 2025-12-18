#include <QApplication>
#include "mainwindow.h"
#include <iostream>
#include <QDebug>

int main(int argc, char *argv[]) {
    std::cout << "=== ЗАПУСК КУЛИНАРНОЙ КНИГИ ===" << std::endl;
    
    // Отладочная информация
    std::cout << "argc: " << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    }
    
    qDebug() << "QT Version:" << QT_VERSION_STR;
    qDebug() << "QApplication создается...";
    
    if (!qEnvironmentVariableIsSet("XDG_RUNTIME_DIR")) {
        qputenv("XDG_RUNTIME_DIR", "/tmp/runtime-cookbookuser");
    }
    
    setlocale(LC_ALL, "C.UTF-8");
    
    try {
        QApplication app(argc, argv);
        qDebug() << "QApplication создан";
        
        qDebug() << "Создание MainWindow...";
        MainWindow window;
        qDebug() << "MainWindow создан";
        
        qDebug() << "Показ окна...";
        window.show();
        qDebug() << "Окно показано, запуск event loop...";
        
        return app.exec();
    } catch (const std::exception& e) {
        std::cerr << "ОШИБКА: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "НЕИЗВЕСТНАЯ ОШИБКА" << std::endl;
        return 1;
    }
}