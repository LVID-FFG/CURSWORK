#include "recipedialog.h"
#include "ui_recipedialog.h"
#include <QMessageBox>
#include <QInputDialog>

RecipeDialog::RecipeDialog(CookBookDatabase* db, Mode mode, QWidget *parent)
    : QDialog(parent), ui(new Ui::RecipeDialog), database_(db), mode_(mode), currentRecipeId_(-1) {
    
    ui->setupUi(this);
    
    ingredientsModel_ = new QStandardItemModel(this);
    ingredientsModel_->setColumnCount(3);
    ingredientsModel_->setHorizontalHeaderLabels({"Название", "Количество", "Единица"});
    ui->ingredientsTable->setModel(ingredientsModel_);
    
    stepsModel_ = new QStandardItemModel(this);
    stepsModel_->setColumnCount(1);
    stepsModel_->setHorizontalHeaderLabels({"Описание шага"});
    ui->stepsTable->setModel(stepsModel_);
    ui->stepsTable->horizontalHeader()->setStretchLastSection(true);
    
    ui->difficultyComboBox->addItems({"Легкий", "Средний", "Сложный"});
    ui->categoryComboBox->addItems({"Завтрак", "Обед", "Ужин", "Десерт", "Основное", "Суп", "Салат", "Закуска"});
    
    setupConnections();
    loadAvailableTags();
    
    if (mode == Create) {
        setWindowTitle("Добавить новый рецепт");
    } else {
        setWindowTitle("Редактировать рецепт");
    }
}

RecipeDialog::~RecipeDialog() {
    delete ui;
}

void RecipeDialog::setupConnections() {
    connect(ui->addIngredientButton, &QPushButton::clicked, this, &RecipeDialog::onAddIngredientClicked);
    connect(ui->removeIngredientButton, &QPushButton::clicked, this, &RecipeDialog::onRemoveIngredientClicked);
    connect(ui->addStepButton, &QPushButton::clicked, this, &RecipeDialog::onAddStepClicked);
    connect(ui->removeStepButton, &QPushButton::clicked, this, &RecipeDialog::onRemoveStepClicked);
    connect(ui->addTagButton, &QPushButton::clicked, this, &RecipeDialog::onAddTagClicked);
    connect(ui->removeTagButton, &QPushButton::clicked, this, &RecipeDialog::onRemoveTagClicked);
    connect(ui->newTagButton, &QPushButton::clicked, this, &RecipeDialog::onNewTagClicked);
}

void RecipeDialog::loadAvailableTags() {
    ui->availableTagsList->clear();
    if (database_) {
        auto tags = database_->getAllTags();
        for (const auto& tag : tags) {
            ui->availableTagsList->addItem(QString::fromStdString(tag));
        }
    }
}

void RecipeDialog::setRecipe(const Recipe& recipe) {
    currentRecipeId_ = recipe.getId();
    
    ui->nameEdit->setText(QString::fromStdString(recipe.getName()));
    ui->descriptionEdit->setPlainText(QString::fromStdString(recipe.getDescription()));
    ui->cookingTimeSpinBox->setValue(recipe.getCookingTime());
    
    int difficultyIndex = ui->difficultyComboBox->findText(QString::fromStdString(recipe.getDifficulty()));
    if (difficultyIndex != -1) {
        ui->difficultyComboBox->setCurrentIndex(difficultyIndex);
    }
    
    ui->categoryComboBox->setCurrentText(QString::fromStdString(recipe.getCategory()));
    
    ingredientsModel_->removeRows(0, ingredientsModel_->rowCount());
    for (const auto& ingredient : recipe.getIngredients()) {
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::fromStdString(ingredient.getName()));
        rowItems << new QStandardItem(QString::fromStdString(ingredient.getQuantity()));
        rowItems << new QStandardItem(QString::fromStdString(ingredient.getUnit()));
        ingredientsModel_->appendRow(rowItems);
    }
    
    stepsModel_->removeRows(0, stepsModel_->rowCount());
    for (const auto& step : recipe.getSteps()) {
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::fromStdString(step.getDescription()));
        stepsModel_->appendRow(rowItems);
    }
    
    ui->recipeTagsList->clear();
    for (const auto& tag : recipe.getTags()) {
        ui->recipeTagsList->addItem(QString::fromStdString(tag));
    }
}

Recipe RecipeDialog::getRecipe() const {
    Recipe recipe(ui->nameEdit->text().toStdString(), ui->descriptionEdit->toPlainText().toStdString());
    
    recipe.setId(currentRecipeId_);
    recipe.setCookingTime(ui->cookingTimeSpinBox->value());
    recipe.setDifficulty(ui->difficultyComboBox->currentText().toStdString());
    recipe.setCategory(ui->categoryComboBox->currentText().toStdString());
    
    for (int row = 0; row < ingredientsModel_->rowCount(); ++row) {
        std::string name = ingredientsModel_->item(row, 0)->text().toStdString();
        std::string quantity = ingredientsModel_->item(row, 1)->text().toStdString();
        std::string unit = ingredientsModel_->item(row, 2)->text().toStdString();
        recipe.addIngredient(Ingredient(name, quantity, unit));
    }
    
    for (int row = 0; row < stepsModel_->rowCount(); ++row) {
        int stepNumber = row + 1;
        std::string description = stepsModel_->item(row, 0)->text().toStdString();
        recipe.addStep(CookingStep(stepNumber, description));
    }
    
    for (int i = 0; i < ui->recipeTagsList->count(); ++i) {
        recipe.addTag(ui->recipeTagsList->item(i)->text().toStdString());
    }
    
    return recipe;
}

void RecipeDialog::onAddIngredientClicked() {
    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem("");
    rowItems << new QStandardItem("");
    rowItems << new QStandardItem("");
    ingredientsModel_->appendRow(rowItems);
}

void RecipeDialog::onRemoveIngredientClicked() {
    QModelIndexList selected = ui->ingredientsTable->selectionModel()->selectedRows();
    for (int i = selected.size() - 1; i >= 0; --i) {
        ingredientsModel_->removeRow(selected[i].row());
    }
}

void RecipeDialog::onAddStepClicked() {
    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem("");
    stepsModel_->appendRow(rowItems);
}

void RecipeDialog::onRemoveStepClicked() {
    QModelIndexList selected = ui->stepsTable->selectionModel()->selectedRows();
    for (int i = selected.size() - 1; i >= 0; --i) {
        stepsModel_->removeRow(selected[i].row());
    }
}

void RecipeDialog::onAddTagClicked() {
    QListWidgetItem* selected = ui->availableTagsList->currentItem();
    if (selected) {
        QString tag = selected->text();
        bool exists = false;
        for (int i = 0; i < ui->recipeTagsList->count(); ++i) {
            if (ui->recipeTagsList->item(i)->text() == tag) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            ui->recipeTagsList->addItem(tag);
        }
    }
}

void RecipeDialog::onRemoveTagClicked() {
    QListWidgetItem* selected = ui->recipeTagsList->currentItem();
    if (selected) {
        delete selected;
    }
}

void RecipeDialog::onNewTagClicked() {
    bool ok;
    QString tag = QInputDialog::getText(this, "Новый тег", "Введите название нового тега:", QLineEdit::Normal, "", &ok);
    
    if (ok && !tag.trimmed().isEmpty()) {
        for (int i = 0; i < ui->availableTagsList->count(); ++i) {
            if (ui->availableTagsList->item(i)->text() == tag) {
                return;
            }
        }
        
        ui->availableTagsList->addItem(tag);
        ui->recipeTagsList->addItem(tag);
    }
}

bool RecipeDialog::validateForm() {
    if (ui->nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название рецепта не может быть пустым");
        return false;
    }
    
    if (ingredientsModel_->rowCount() == 0) {
        QMessageBox::warning(this, "Ошибка", "Добавьте хотя бы один ингредиент");
        return false;
    }
    
    if (stepsModel_->rowCount() == 0) {
        QMessageBox::warning(this, "Ошибка", "Добавьте хотя бы один шаг приготовления");
        return false;
    }
    
    return true;
}

void RecipeDialog::accept() {
    if (!validateForm()) {
        return;
    }
    
    QDialog::accept();
}