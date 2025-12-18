#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "recipedialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    
    ui->setupUi(this);
    
    qDebug() << "Запуск Кулинарной книги...";
    
    database = std::make_unique<CookBookDatabase>();
    
    if (!database->connect()) {
        QString errorMsg = QString("Не удалось подключиться к базе данных:\n%1\n\n"
                                 "Проверьте что PostgreSQL запущен в контейнере.")
            .arg(QString::fromStdString(database->getLastError()));
        
        QMessageBox::critical(this, "Ошибка", errorMsg);
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }
    
    qDebug() << "База данных подключена!";
    
    // Загружаем рецепты
    loadRecipes();
    
    // Загружаем теги в комбобокс фильтра
    loadTags();
    
    // Если нет рецептов, создаем демо-рецепт
    if (ui->recipesListWidget->count() == 0) {
        createDefaultRecipes();
        loadRecipes();
        loadTags(); // Перезагружаем теги после создания демо-рецептов
    }
    
    // Подключаем сигналы
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddRecipeClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &MainWindow::onEditRecipeClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteRecipeClicked);
    connect(ui->recipesListWidget, &QListWidget::itemClicked, this, &MainWindow::onRecipeSelected);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(ui->tagFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onTagFilterChanged);
    
    // Выбираем первый рецепт если есть
    if (ui->recipesListWidget->count() > 0) {
        ui->recipesListWidget->setCurrentRow(0);
        onRecipeSelected(ui->recipesListWidget->item(0));
    }
    
    ui->statusbar->showMessage("Готово");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::createDefaultRecipes() {
    qDebug() << "Создание демо-рецептов...";
    
    Recipe demo("Добро пожаловать!", "Это ваша кулинарная книга.\n\nДобавляйте свои рецепты, редактируйте их и удаляйте.");
    demo.setCookingTime(5);
    demo.setDifficulty("Легкий");
    demo.setCategory("Информация");
    demo.addIngredient(Ingredient("Идеи", "много", ""));
    demo.addIngredient(Ingredient("Рецепты", "несколько", ""));
    demo.addStep(CookingStep(1, "Нажмите 'Добавить' чтобы создать новый рецепт"));
    demo.addStep(CookingStep(2, "Заполните все поля: название, ингредиенты, шаги приготовления"));
    demo.addStep(CookingStep(3, "Сохраните рецепт и он появится в списке слева"));
    demo.addTag("демо");
    demo.addTag("инструкция");
    
    database->addRecipe(demo);
}

void MainWindow::loadRecipes() {
    ui->recipesListWidget->clear();
    
    auto recipes = database->getAllRecipes();
    
    for (const auto& recipe : recipes) {
        QListWidgetItem* item = new QListWidgetItem(
            QString::fromStdString(recipe->getName()));
        item->setData(Qt::UserRole, recipe->getId());
        
        // Сохраняем теги рецепта в данных элемента
        QStringList tags;
        for (const auto& tag : recipe->getTags()) {
            tags << QString::fromStdString(tag);
        }
        item->setData(Qt::UserRole + 1, tags);
        
        ui->recipesListWidget->addItem(item);
    }
    
    ui->statusbar->showMessage(QString("Рецептов: %1").arg(recipes.size()));
}

void MainWindow::loadTags() {
    ui->tagFilterComboBox->clear();
    ui->tagFilterComboBox->addItem("Все теги", "");
    
    auto tags = database->getAllTags();
    for (const auto& tag : tags) {
        ui->tagFilterComboBox->addItem(QString::fromStdString(tag), 
                                       QString::fromStdString(tag));
    }
}

void MainWindow::onAddRecipeClicked() {
    RecipeDialog dialog(database.get(), RecipeDialog::Create, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        Recipe recipe = dialog.getRecipe();
        int recipeId = database->addRecipe(recipe);
        
        if (recipeId != -1) {
            loadRecipes();
            loadTags(); // Обновляем список тегов
            QMessageBox::information(this, "Успех", "Рецепт добавлен!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить рецепт");
        }
    }
}

void MainWindow::onEditRecipeClicked() {
    QListWidgetItem* item = ui->recipesListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Предупреждение", "Выберите рецепт для редактирования");
        return;
    }
    
    int recipeId = item->data(Qt::UserRole).toInt();
    auto recipe = database->getRecipeById(recipeId);
    
    if (!recipe) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить рецепт");
        return;
    }
    
    RecipeDialog dialog(database.get(), RecipeDialog::Edit, this);
    dialog.setRecipe(*recipe);
    
    if (dialog.exec() == QDialog::Accepted) {
        Recipe updatedRecipe = dialog.getRecipe();
        if (database->updateRecipe(updatedRecipe)) {
            item->setText(QString::fromStdString(updatedRecipe.getName()));
            
            // Обновляем теги в данных элемента
            QStringList tags;
            for (const auto& tag : updatedRecipe.getTags()) {
                tags << QString::fromStdString(tag);
            }
            item->setData(Qt::UserRole + 1, tags);
            
            loadTags(); // Обновляем список тегов
            onRecipeSelected(item);
            QMessageBox::information(this, "Успех", "Рецепт обновлен!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось обновить рецепт");
        }
    }
}

void MainWindow::onDeleteRecipeClicked() {
    QListWidgetItem* item = ui->recipesListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Предупреждение", "Выберите рецепт для удаления");
        return;
    }
    
    int recipeId = item->data(Qt::UserRole).toInt();
    QString recipeName = item->text();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Удаление", 
        QString("Удалить рецепт '%1'?").arg(recipeName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (database->deleteRecipe(recipeId)) {
            delete item;
            // Очищаем отображение
            ui->recipeNameLabel->setText("Кулинарная книга");
            ui->descriptionLabel->setText("Выберите рецепт из списка");
            ui->ingredientsTextEdit->clear();
            ui->stepsTextEdit->clear();
            ui->categoryLabel->clear();
            ui->cookingTimeLabel->clear();
            ui->difficultyLabel->clear();
            ui->tagsLabel->clear();
            
            loadTags(); // Обновляем список тегов
            QMessageBox::information(this, "Успех", "Рецепт удален!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить рецепт");
        }
    }
}

void MainWindow::onRecipeSelected(QListWidgetItem* item) {
    if (!item) return;
    
    int recipeId = item->data(Qt::UserRole).toInt();
    auto recipe = database->getRecipeById(recipeId);
    
    if (!recipe) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить рецепт");
        return;
    }
    
    // Отображаем рецепт
    ui->recipeNameLabel->setText(QString::fromStdString(recipe->getName()));
    ui->descriptionLabel->setText(QString::fromStdString(recipe->getDescription()));
    ui->categoryLabel->setText(QString("Категория: %1").arg(QString::fromStdString(recipe->getCategory())));
    ui->cookingTimeLabel->setText(QString("Время: %1 мин").arg(recipe->getCookingTime()));
    ui->difficultyLabel->setText(QString("Сложность: %1").arg(QString::fromStdString(recipe->getDifficulty())));
    
    // Теги
    QString tagsText;
    auto tags = recipe->getTags();
    for (const auto& tag : tags) {
        tagsText += QString("#%1 ").arg(QString::fromStdString(tag));
    }
    ui->tagsLabel->setText(tagsText.isEmpty() ? "Теги не указаны" : QString("Теги: %1").arg(tagsText));
    
    // Ингредиенты
    QString ingredients;
    for (const auto& ing : recipe->getIngredients()) {
        ingredients += QString("• %1 - %2 %3\n")
            .arg(QString::fromStdString(ing.getName()))
            .arg(QString::fromStdString(ing.getQuantity()))
            .arg(QString::fromStdString(ing.getUnit()));
    }
    ui->ingredientsTextEdit->setPlainText(ingredients);
    
    // Шаги
    QString steps;
    int stepNumber = 1;
    for (const auto& step : recipe->getSteps()) {
        steps += QString("<b>Шаг %1:</b> %2<br><br>")
            .arg(stepNumber)
            .arg(QString::fromStdString(step.getDescription()));
        stepNumber++;
    }
    ui->stepsTextEdit->setHtml(steps);
}

void MainWindow::onSearchTextChanged(const QString& text) {
    applyFilters();
}

void MainWindow::onTagFilterChanged(int index) {
    applyFilters();
}

void MainWindow::applyFilters() {
    QString searchText = ui->searchEdit->text();
    QString selectedTag = ui->tagFilterComboBox->currentData().toString();
    
    for (int i = 0; i < ui->recipesListWidget->count(); ++i) {
        QListWidgetItem* item = ui->recipesListWidget->item(i);
        
        bool matchesSearch = item->text().contains(searchText, Qt::CaseInsensitive);
        bool matchesTag = true;
        
        if (!selectedTag.isEmpty()) {
            // Проверяем, содержит ли рецепт выбранный тег
            QStringList tags = item->data(Qt::UserRole + 1).toStringList();
            matchesTag = tags.contains(selectedTag);
        }
        
        item->setHidden(!(matchesSearch && matchesTag));
    }
    
    // Обновляем статус
    int visibleCount = 0;
    for (int i = 0; i < ui->recipesListWidget->count(); ++i) {
        if (!ui->recipesListWidget->item(i)->isHidden()) {
            visibleCount++;
        }
    }
    
    if (!selectedTag.isEmpty()) {
        ui->statusbar->showMessage(QString("Рецептов с тегом '%1': %2").arg(selectedTag).arg(visibleCount));
    } else {
        ui->statusbar->showMessage(QString("Показано рецептов: %1").arg(visibleCount));
    }
}