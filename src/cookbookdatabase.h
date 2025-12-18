#pragma once

#include <string>
#include <vector>
#include <memory>
#include <libpq-fe.h>

class Recipe;
class Ingredient;
class CookingStep;

class CookBookDatabase {
public:
    CookBookDatabase();
    ~CookBookDatabase();
    
    bool connect();
    void disconnect();
    bool isConnected() const { return conn_ != nullptr; }
    
    int addRecipe(Recipe& recipe);
    bool updateRecipe(const Recipe& recipe);
    bool deleteRecipe(int recipeId);
    std::shared_ptr<Recipe> getRecipeById(int id);
    std::vector<std::shared_ptr<Recipe>> getAllRecipes();
    
    std::vector<std::string> getAllTags();
    
    std::string getLastError() const { return lastError_; }
    
private:
    bool createTables();
    bool saveRecipeTags(int recipeId, const std::vector<std::string>& tags);
    bool saveRecipeIngredients(int recipeId, const std::vector<Ingredient>& ingredients);
    bool saveRecipeSteps(int recipeId, const std::vector<CookingStep>& steps);
    
    std::vector<std::string> getRecipeTags(int recipeId);
    std::vector<Ingredient> getRecipeIngredients(int recipeId);
    std::vector<CookingStep> getRecipeSteps(int recipeId);
    
    std::string escapeString(const std::string& str);
    bool executeQuery(const std::string& query);
    
    PGconn* conn_;
    std::string lastError_;
};