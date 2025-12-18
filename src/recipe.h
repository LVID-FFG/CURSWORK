#pragma once
#include <string>
#include <vector>
#include <memory>
using namespace std;

class Ingredient {
public:
    Ingredient(const string& name, const string& quantity, const string& unit = "");
    
    string getName() const { return name_; }
    string getQuantity() const { return quantity_; }
    string getUnit() const { return unit_; }
    
    void setName(const string& name) { name_ = name; }
    void setQuantity(const string& quantity) { quantity_ = quantity; }
    void setUnit(const string& unit) { unit_ = unit; }
    
private:
    string name_;
    string quantity_;
    string unit_;
};

class CookingStep {
public:
    CookingStep(int number, const string& description);
    
    int getStepNumber() const { return stepNumber_; }
    string getDescription() const { return description_; }
    
    void setStepNumber(int number) { stepNumber_ = number; }
    void setDescription(const string& description) { description_ = description; }
    
private:
    int stepNumber_;
    string description_;
};

class Recipe {
public:
    Recipe(const string& name, const string& description = "");
    
    int getId() const { return id_; }
    string getName() const { return name_; }
    string getDescription() const { return description_; }
    int getCookingTime() const { return cookingTime_; }
    string getDifficulty() const { return difficulty_; }
    string getCategory() const { return category_; }
    vector<Ingredient> getIngredients() const { return ingredients_; }
    vector<CookingStep> getSteps() const { return steps_; }
    vector<string> getTags() const { return tags_; }
    
    void setId(int id) { id_ = id; }
    void setName(const string& name) { name_ = name; }
    void setDescription(const string& description) { description_ = description; }
    void setCookingTime(int time) { cookingTime_ = time; }
    void setDifficulty(const string& difficulty) { difficulty_ = difficulty; }
    void setCategory(const string& category) { category_ = category; }
    
    void addIngredient(const Ingredient& ingredient);
    void removeIngredient(int index);
    void clearIngredients();
    
    void addStep(const CookingStep& step);
    void removeStep(int index);
    void clearSteps();
    
    void addTag(const string& tag);
    void removeTag(const string& tag);
    bool hasTag(const string& tag) const;
    void clearTags();
    
private:
    int id_;
    string name_;
    string description_;
    int cookingTime_;
    string difficulty_;
    string category_;
    vector<Ingredient> ingredients_;
    vector<CookingStep> steps_;
    vector<string> tags_;
};