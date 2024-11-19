#ifndef __UNITSHAPER_HPP
#define __UNITSHAPER_HPP

namespace deepnotedrone {
    /**
     * Takes linear input from 0.0 to 1.0 and returns a shaped output.
     */
    class UnitShaper {
        public:
            virtual float operator()(float value) = 0;
    };
}

#endif //__UNITSHAPER_HPP