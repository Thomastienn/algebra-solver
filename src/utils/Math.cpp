#include "Math.h"

double Math::fastpow(double base, int exp) {
    if (exp < 0) {
        base = 1.0 / base;
        exp = -exp;
    }
    double result = 1.0;
    while (exp) {
        if (exp & 1)
            result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}
