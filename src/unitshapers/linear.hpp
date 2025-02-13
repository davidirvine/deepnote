#pragma once

namespace deepnote
{

    struct LinearUnitShaper
    {
        float operator()(const float value) const
        {
            return value;
        }
    };

} // namespace deepnote
