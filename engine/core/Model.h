#pragma once

class Model
{
public:
    virtual ~Model() {}

    virtual void Regenerate() = 0;

protected:
    bool dirty = true;
};
