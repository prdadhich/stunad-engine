#pragma once
#include "../core/Model.h"
#include "../modules/Pedestal.h"

class LampModel : public Model
{
public:
    LampModel(Kernel* k) : base(k)
    {
        base.radius.onChange = [this]() { dirty = true; };
        base.height.onChange = [this]() { dirty = true; };
    }

    Solid* Get()
    {
        if (dirty)
            Regenerate();
        return current;
    }

    void Regenerate() override
    {
        current = base.Build();
        dirty = false;
    }

    Pedestal base;

private:
    Solid* current = nullptr;
};
