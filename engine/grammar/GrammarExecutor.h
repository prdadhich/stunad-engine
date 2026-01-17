#pragma once
#include "GrammarProgram.h"
#include "../core/Kernel.h"
#include "../core/Solid.h"

class GrammarExecutor {
public:
    Solid* execute(const GrammarProgram& program, Kernel& kernel);
};
