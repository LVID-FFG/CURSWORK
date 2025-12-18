#include "cookbookdatabase.h"
#include "recipe.h"
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

CookBookDatabase::CookBookDatabase() : conn_(nullptr) {}

CookBookDatabase::~CookBookDatabase() {
    disconnect();
}

bool CookBookDatabase::connect() {
    cout << "Подключение к PostgreSQL..." << endl;
    
    string connInfo = "host=localhost dbname=cookbook user=cookbookuser password=cookbook123";
    
    conn_ = PQconnectdb(connInfo.c_str());
    
    if (PQstatus(conn_) != CONNECTION_OK) {
        lastError_ = PQerrorMessage(conn_);
        cout << "Ошибка: " << lastError_ << endl;
        return false;
    }
    
    cout << "Подключение успешно!" << endl;
    return createTables();
}

void CookBookDatabase::disconnect() {
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

bool CookBookDatabase::createTables() {
    if (!conn_) {
        lastError_ = "Нет подключения к БД";
        return false;
    }
    
    // Создаем все таблицы
    const char* queries[] = {
        // Таблица рецептов
        "CREATE TABLE IF NOT EXISTS recipes ("
        "id SERIAL PRIMARY KEY,"
        "name VARCHAR(255) NOT NULL,"
        "description TEXT,"
        "cooking_time INTEGER DEFAULT 0,"
        "difficulty VARCHAR(50) DEFAULT 'Средний',"
        "category VARCHAR(100) DEFAULT 'Основное',"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);",
        
        // Таблица ингредиентов
        "CREATE TABLE IF NOT EXISTS recipe_ingredients ("
        "id SERIAL PRIMARY KEY,"
        "recipe_id INTEGER REFERENCES recipes(id) ON DELETE CASCADE,"
        "name VARCHAR(255) NOT NULL,"
        "quantity VARCHAR(100),"
        "unit VARCHAR(50),"
        "sort_order INTEGER DEFAULT 0);",
        
        // Таблица шагов
        "CREATE TABLE IF NOT EXISTS cooking_steps ("
        "id SERIAL PRIMARY KEY,"
        "recipe_id INTEGER REFERENCES recipes(id) ON DELETE CASCADE,"
        "step_number INTEGER NOT NULL,"
        "description TEXT NOT NULL,"
        "sort_order INTEGER DEFAULT 0);",
        
        // Таблица тегов
        "CREATE TABLE IF NOT EXISTS tags ("
        "id SERIAL PRIMARY KEY,"
        "name VARCHAR(100) UNIQUE NOT NULL);",
        
        // Таблица связи рецептов и тегов
        "CREATE TABLE IF NOT EXISTS recipe_tags ("
        "recipe_id INTEGER REFERENCES recipes(id) ON DELETE CASCADE,"
        "tag_id INTEGER REFERENCES tags(id) ON DELETE CASCADE,"
        "PRIMARY KEY (recipe_id, tag_id));"
    };
    
    for (const char* query : queries) {
        if (!executeQuery(query)) {
            return false;
        }
    }
    
    return true;
}

bool CookBookDatabase::executeQuery(const string& query) {
    if (!conn_) {
        lastError_ = "Нет подключения к БД";
        return false;
    }
    
    PGresult* res = PQexec(conn_, query.c_str());
    ExecStatusType status = PQresultStatus(res);
    bool success = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);
    
    if (!success) {
        lastError_ = PQerrorMessage(conn_);
    }
    
    PQclear(res);
    return success;
}

int CookBookDatabase::addRecipe(Recipe& recipe) {
    if (!conn_) return -1;
    
    string name = escapeString(recipe.getName());
    string description = escapeString(recipe.getDescription());
    string difficulty = escapeString(recipe.getDifficulty());
    string category = escapeString(recipe.getCategory());
    
    string query = "INSERT INTO recipes (name, description, cooking_time, difficulty, category) "
                   "VALUES (" + name + ", " + description + ", " + 
                   to_string(recipe.getCookingTime()) + ", " + difficulty + ", " + 
                   category + ") RETURNING id;";
    
    PGresult* res = PQexec(conn_, query.c_str());
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        lastError_ = PQerrorMessage(conn_);
        PQclear(res);
        return -1;
    }
    
    int recipeId = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    recipe.setId(recipeId);
    
    saveRecipeIngredients(recipeId, recipe.getIngredients());
    saveRecipeSteps(recipeId, recipe.getSteps());
    saveRecipeTags(recipeId, recipe.getTags());
    
    return recipeId;
}

string CookBookDatabase::escapeString(const string& str) {
    if (!conn_ || str.empty()) return "''";
    
    char* escaped = PQescapeLiteral(conn_, str.c_str(), str.length());
    if (!escaped) {
        lastError_ = "Ошибка экранирования строки";
        return "''";
    }
    
    string result(escaped);
    PQfreemem(escaped);
    return result;
}

shared_ptr<Recipe> CookBookDatabase::getRecipeById(int id) {
    if (!conn_) return nullptr;
    
    string query = "SELECT name, description, cooking_time, difficulty, category "
                   "FROM recipes WHERE id = " + to_string(id) + ";";
    
    PGresult* res = PQexec(conn_, query.c_str());
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return nullptr;
    }
    
    auto recipe = make_shared<Recipe>(
        PQgetvalue(res, 0, 0),
        PQgetvalue(res, 0, 1)
    );
    
    recipe->setId(id);
    recipe->setCookingTime(atoi(PQgetvalue(res, 0, 2)));
    recipe->setDifficulty(PQgetvalue(res, 0, 3));
    recipe->setCategory(PQgetvalue(res, 0, 4));
    
    PQclear(res);
    
    // Загружаем ингредиенты
    auto ingredients = getRecipeIngredients(id);
    for (const auto& ing : ingredients) {
        recipe->addIngredient(ing);
    }
    
    // Загружаем шаги
    auto steps = getRecipeSteps(id);
    for (const auto& step : steps) {
        recipe->addStep(step);
    }
    
    // Загружаем теги
    auto tags = getRecipeTags(id);
    for (const auto& tag : tags) {
        recipe->addTag(tag);
    }
    
    return recipe;
}

vector<shared_ptr<Recipe>> CookBookDatabase::getAllRecipes() {
    vector<shared_ptr<Recipe>> recipes;
    
    if (!conn_) return recipes;
    
    string query = "SELECT id, name, description, cooking_time, difficulty, category "
                   "FROM recipes ORDER BY name;";
    PGresult* res = PQexec(conn_, query.c_str());
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return recipes;
    }
    
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        int id = atoi(PQgetvalue(res, i, 0));
        
        auto recipe = make_shared<Recipe>(
            PQgetvalue(res, i, 1),
            PQgetvalue(res, i, 2)
        );
        
        recipe->setId(id);
        recipe->setCookingTime(atoi(PQgetvalue(res, i, 3)));
        recipe->setDifficulty(PQgetvalue(res, i, 4));
        recipe->setCategory(PQgetvalue(res, i, 5));
        
        recipes.push_back(recipe);
    }
    
    PQclear(res);
    return recipes;
}

bool CookBookDatabase::deleteRecipe(int recipeId) {
    if (!conn_ || recipeId <= 0) return false;
    
    string query = "DELETE FROM recipes WHERE id = " + to_string(recipeId) + ";";
    return executeQuery(query);
}

vector<Ingredient> CookBookDatabase::getRecipeIngredients(int recipeId) {
    vector<Ingredient> ingredients;
    
    if (!conn_) return ingredients;
    
    string query = "SELECT name, quantity, unit FROM recipe_ingredients "
                   "WHERE recipe_id = " + to_string(recipeId) + " ORDER BY sort_order;";
    
    PGresult* res = PQexec(conn_, query.c_str());
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return ingredients;
    }
    
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        ingredients.push_back(Ingredient(
            PQgetvalue(res, i, 0),
            PQgetvalue(res, i, 1),
            PQgetvalue(res, i, 2)
        ));
    }
    
    PQclear(res);
    return ingredients;
}

vector<CookingStep> CookBookDatabase::getRecipeSteps(int recipeId) {
    vector<CookingStep> steps;
    
    if (!conn_) return steps;
    
    string query = "SELECT step_number, description FROM cooking_steps "
                   "WHERE recipe_id = " + to_string(recipeId) + " ORDER BY sort_order;";
    
    PGresult* res = PQexec(conn_, query.c_str());
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return steps;
    }
    
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        steps.push_back(CookingStep(
            atoi(PQgetvalue(res, i, 0)),
            PQgetvalue(res, i, 1)
        ));
    }
    
    PQclear(res);
    return steps;
}

vector<string> CookBookDatabase::getRecipeTags(int recipeId) {
    vector<string> tags;
    
    if (!conn_) return tags;
    
    string query = "SELECT t.name FROM tags t "
                   "JOIN recipe_tags rt ON t.id = rt.tag_id "
                   "WHERE rt.recipe_id = " + to_string(recipeId) + " ORDER BY t.name;";
    
    PGresult* res = PQexec(conn_, query.c_str());
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return tags;
    }
    
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        tags.push_back(PQgetvalue(res, i, 0));
    }
    
    PQclear(res);
    return tags;
}

bool CookBookDatabase::saveRecipeIngredients(int recipeId, const vector<Ingredient>& ingredients) {
    if (!conn_) return false;
    
    // Удаляем старые ингредиенты
    string deleteQuery = "DELETE FROM recipe_ingredients WHERE recipe_id = " + to_string(recipeId) + ";";
    executeQuery(deleteQuery);
    
    // Добавляем новые
    for (size_t i = 0; i < ingredients.size(); ++i) {
        const auto& ing = ingredients[i];
        
        string name = escapeString(ing.getName());
        string quantity = escapeString(ing.getQuantity());
        string unit = escapeString(ing.getUnit());
        
        string query = "INSERT INTO recipe_ingredients (recipe_id, name, quantity, unit, sort_order) "
                       "VALUES (" + to_string(recipeId) + ", " + name + ", " + quantity + ", " + unit + ", " + to_string(i) + ");";
        
        if (!executeQuery(query)) {
            return false;
        }
    }
    
    return true;
}

bool CookBookDatabase::saveRecipeSteps(int recipeId, const vector<CookingStep>& steps) {
    if (!conn_) return false;
    
    // Удаляем старые шаги
    string deleteQuery = "DELETE FROM cooking_steps WHERE recipe_id = " + to_string(recipeId) + ";";
    executeQuery(deleteQuery);
    
    // Добавляем новые
    for (size_t i = 0; i < steps.size(); ++i) {
        const auto& step = steps[i];
        
        string description = escapeString(step.getDescription());
        
        string query = "INSERT INTO cooking_steps (recipe_id, step_number, description, sort_order) "
                       "VALUES (" + to_string(recipeId) + ", " + to_string(step.getStepNumber()) + ", " + 
                       description + ", " + to_string(i) + ");";
        
        if (!executeQuery(query)) {
            return false;
        }
    }
    
    return true;
}

bool CookBookDatabase::saveRecipeTags(int recipeId, const vector<string>& tags) {
    if (!conn_) return false;
    
    // Удаляем старые связи
    string deleteQuery = "DELETE FROM recipe_tags WHERE recipe_id = " + to_string(recipeId) + ";";
    executeQuery(deleteQuery);
    
    // Добавляем новые теги
    for (const auto& tagName : tags) {
        string tag = escapeString(tagName);
        
        // Получаем или создаем тег
        string tagQuery = "SELECT id FROM tags WHERE name = " + tag + ";";
        PGresult* res = PQexec(conn_, tagQuery.c_str());
        
        int tagId = -1;
        if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
            tagId = atoi(PQgetvalue(res, 0, 0));
        }
        PQclear(res);
        
        if (tagId == -1) {
            string insertQuery = "INSERT INTO tags (name) VALUES (" + tag + ") RETURNING id;";
            res = PQexec(conn_, insertQuery.c_str());
            
            if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                tagId = atoi(PQgetvalue(res, 0, 0));
            }
            PQclear(res);
        }
        
        // Связываем тег с рецептом
        if (tagId != -1) {
            string linkQuery = "INSERT INTO recipe_tags (recipe_id, tag_id) VALUES (" + to_string(recipeId) + ", " + to_string(tagId) + ");";
            executeQuery(linkQuery);
        }
    }
    
    return true;
}

bool CookBookDatabase::updateRecipe(const Recipe& recipe) {
    if (!conn_) return false;
    
    int recipeId = recipe.getId();
    if (recipeId <= 0) return false;
    
    string name = escapeString(recipe.getName());
    string description = escapeString(recipe.getDescription());
    string difficulty = escapeString(recipe.getDifficulty());
    string category = escapeString(recipe.getCategory());
    
    string query = "UPDATE recipes SET name = " + name + ", description = " + description + 
                   ", cooking_time = " + to_string(recipe.getCookingTime()) + 
                   ", difficulty = " + difficulty + ", category = " + category + 
                   " WHERE id = " + to_string(recipeId) + ";";
    
    if (!executeQuery(query)) {
        return false;
    }
    
    saveRecipeIngredients(recipeId, recipe.getIngredients());
    saveRecipeSteps(recipeId, recipe.getSteps());
    saveRecipeTags(recipeId, recipe.getTags());
    
    return true;
}

vector<string> CookBookDatabase::getAllTags() {
    vector<string> tags;
    
    if (!conn_) return tags;
    
    PGresult* res = PQexec(conn_, "SELECT name FROM tags ORDER BY name;");
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return tags;
    }
    
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        tags.push_back(PQgetvalue(res, i, 0));
    }
    
    PQclear(res);
    return tags;
}