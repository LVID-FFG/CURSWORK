#pragma once
#include <QDialog>
#include <QStandardItemModel>
#include "recipe.h"
#include "cookbookdatabase.h"

namespace Ui {
class RecipeDialog;
}

class RecipeDialog : public QDialog {
    Q_OBJECT

public:
    enum Mode { Create, Edit };
    
    explicit RecipeDialog(CookBookDatabase* db, Mode mode = Create, QWidget *parent = nullptr);
    ~RecipeDialog();
    
    void setRecipe(const Recipe& recipe);
    Recipe getRecipe() const;
    
private slots:
    void onAddIngredientClicked();
    void onRemoveIngredientClicked();
    void onAddStepClicked();
    void onRemoveStepClicked();
    void onAddTagClicked();
    void onRemoveTagClicked();
    void onNewTagClicked();
    void accept() override;
    
private:
    void setupConnections();
    void loadAvailableTags();
    bool validateForm();
    
    Ui::RecipeDialog *ui;
    CookBookDatabase* database_;
    Mode mode_;
    int currentRecipeId_;
    QStandardItemModel* ingredientsModel_;
    QStandardItemModel* stepsModel_;
};