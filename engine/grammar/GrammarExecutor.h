#pragma once
#include "GrammarProgram.h"
#include "../core/Kernel.h"
#include "../core/Solid.h"
#include <memory>
class GrammarExecutor {
public:
    std::unique_ptr<Solid> execute(const GrammarProgram& program, Kernel& kernel);
};
