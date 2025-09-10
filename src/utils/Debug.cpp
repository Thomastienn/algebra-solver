#include "Debug.h"
#include <iostream>

template<typename T, size_t N>
void Dbg<T, N>::printArray(const T (&arr)[N]) {
    std::cout << "[";
    for (size_t i = 0; i < N; ++i) {
        std::cout << arr[i];
        if (i < N - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}
