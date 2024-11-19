#ifndef __LINEAR_UNIT_SHAPER_HPP
#define __LINEAR_UNIT_SHAPER_HPP

#include "unitshaper.hpp"

namespace deepnotedrone {

    class LinearUnitShaper : public UnitShaper {
        public:
            float operator()(float value) {
                return value;
            }
    };
}

#endif // __LINEAR_UNIT_SHAPER_HPP