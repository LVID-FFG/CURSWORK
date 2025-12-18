#include "recipe.h"
#include <algorithm>
using namespace std;

Ingredient::Ingredient(const string& name, const string& quantity, const string& unit) 
    : name_(name), quantity_(quantity), unit_(unit) {}

CookingStep::CookingStep(int number, const string& description) 
    : stepNumber_(number), description_(description) {}

Recipe::Recipe(const string& name, const string& description) 
    : id_(-1), name_(name), description_(description), cookingTime_(0), 
      difficulty_("Средний"), category_("Основное") {}

void Recipe::addIngredient(const Ingredient& ingredient) {
    ingredients_.push_back(ingredient);
}

void Recipe::removeIngredient(int index) {
    if (index >= 0 && index < ingredients_.size()) {
        ingredients_.erase(ingredients_.begin() + index);
    }
}

void Recipe::clearIngredients() {
    ingredients_.clear();
}

void Recipe::addStep(const CookingStep& step) {
    steps_.push_back(step);
    sort(steps_.begin(), steps_.end(), [](const CookingStep& a, const CookingStep& b) {
        return a.getStepNumber() < b.getStepNumber();
    });
}

void Recipe::removeStep(int index) {
    if (index >= 0 && index < steps_.size()) {
        steps_.erase(steps_.begin() + index);
        for (size_t i = 0; i < steps_.size(); ++i) {
            steps_[i].setStepNumber(i + 1);
        }
    }
}

void Recipe::clearSteps() {
    steps_.clear();
}

void Recipe::addTag(const string& tag) {
    if (!hasTag(tag)) {
        tags_.push_back(tag);
    }
}

void Recipe::removeTag(const string& tag) {
    tags_.erase(remove(tags_.begin(), tags_.end(), tag), tags_.end());
}

bool Recipe::hasTag(const string& tag) const {
    return find(tags_.begin(), tags_.end(), tag) != tags_.end();
}

void Recipe::clearTags() {
    tags_.clear();
}