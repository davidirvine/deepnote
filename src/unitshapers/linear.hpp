#pragma once

#include "unitshaper.hpp"

namespace deepnote 
{

class LinearUnitShaper : public UnitShaper 
{
public:
    float shape(const float value) 
    {
        return value;
    }
};
    
} // namespace deepnote 