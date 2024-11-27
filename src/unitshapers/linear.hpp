#pragma once

#include "unitshaper.hpp"

class LinearUnitShaper : public UnitShaper 
{
public:
    float shape(const float value) 
    {
        return value;
    }
};