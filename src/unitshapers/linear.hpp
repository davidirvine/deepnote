#pragma once

namespace deepnote
{

    struct LinearUnitShaper
    {
    public:
        float operator()(const float value) const
        {
            return value;
        }
    };

} // namespace deepnote
