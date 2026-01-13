#pragma once
#include <functional>

template<typename T>
class Param
{
public:
    Param(const T& v) : value(v) {}

    void Set(const T& v)
    {
        if (v != value)
        {
            value = v;
            if (onChange) onChange();
        }
    }

    const T& Get() const { return value; }

    std::function<void()> onChange;

private:
    T value;
};
