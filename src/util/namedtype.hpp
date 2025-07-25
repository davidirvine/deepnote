#pragma once

#include <utility>

namespace deepnote
{
    //
    //  Taken from: https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
    //
    template <typename T, typename Parameter>
    struct NamedType
    {
    public:
        explicit NamedType(const T &value) : value_(value) {}
        explicit NamedType(T &&value) : value_(std::move(value)) {}
        T &get() { return value_; }
        const T &get() const { return value_; }

        bool operator== (const NamedType<T, Parameter> &rhs) const
        {
            return value_ == rhs.value_;
        }

    private:
        T value_;
    };

} // namespace deepnote
