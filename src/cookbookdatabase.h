#pragma once

#include <string>
#include <vector>
#include <memory>
#include <libpq-fe.h>
using namespace std;
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
    shared_ptr<Recipe> getRecipeById(int id);
    vector<shared_ptr<Recipe>> getAllRecipes();
    
    vector<string> getAllTags();
    
    string getLastError() const { return lastError_; }
    
private:
    bool createTables();
    bool saveRecipeTags(int recipeId, const vector<string>& tags);
    bool saveRecipeIngredients(int recipeId, const vector<Ingredient>& ingredients);
    bool saveRecipeSteps(int recipeId, const vector<CookingStep>& steps);
    
    vector<string> getRecipeTags(int recipeId);
    vector<Ingredient> getRecipeIngredients(int recipeId);
    vector<CookingStep> getRecipeSteps(int recipeId);
    
    string escapeString(const string& str);
    bool executeQuery(const string& query);
    
    PGconn* conn_;
    string lastError_;
};