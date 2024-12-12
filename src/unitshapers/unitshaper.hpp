#pragma once

namespace deepnote 
{

//
//  Takes linear input from 0.0 to 1.0 and returns a shaped output.
//
class UnitShaper 
{
    public:
        virtual float shape(const float value) = 0;
};

} // namespace deepnote