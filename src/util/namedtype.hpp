#pragma once 

#include <utility>

namespace deepnote
{
template <typename T, typename Parameter>
class NamedType
{
public:
    explicit NamedType(const T& value) : value_(value) {}
    explicit NamedType(T&& value) : value_(std::move(value)) {}
    T& get() { return value_; }
    const T& get() const { return value_; }
private:
    T value_;
};


template<template<typename T> class GenericTypeName, typename T>
GenericTypeName<T> make_named(const T& value) 
{
    return GenericTypeName<T>(value);
}

} // namespace deepnote