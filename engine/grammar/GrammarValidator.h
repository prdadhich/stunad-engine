#pragma once
#include "GrammarProgram.h"
#include <string>


enum class ValueType {
    Profile,
    Solid
};




class GrammarValidator {
public:
    bool validate(const GrammarProgram& program, std::string& error);
    std::vector<ValueType> types;
};
