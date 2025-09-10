#pragma once
#include <iostream>

template<typename T, size_t N>
class Dbg {
public:  
    static void printArray(const T (&arr)[N]);
};

