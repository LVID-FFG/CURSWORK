#pragma once
#include <QMainWindow>
#include <memory>
#include <QListWidgetItem>
#include "cookbookdatabase.h"
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddRecipeClicked();
    void onEditRecipeClicked();
    void onDeleteRecipeClicked();
    void onRecipeSelected(QListWidgetItem* item);
    void onSearchTextChanged(const QString& text);
    void onTagFilterChanged(int index);

private:
    void loadRecipes();
    void loadTags();
    void applyFilters();
    void createDefaultRecipes();

    Ui::MainWindow *ui;
    unique_ptr<CookBookDatabase> database;
};