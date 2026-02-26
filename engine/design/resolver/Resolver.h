#pragma once
#include "../../grammar/GrammarProgram.h"

template <typename IntentT>
class Resolver {
public:
    virtual ~Resolver() = default;
    virtual GrammarProgram resolve(const IntentT& intent) = 0;
};
