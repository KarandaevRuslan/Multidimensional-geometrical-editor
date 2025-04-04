#include "numTools.h"
#include <iostream>

bool isInteger(float value) {
    float intPart;
    float fracPart = std::modf(value, &intPart);
    return std::fabs(fracPart) < std::numeric_limits<float>::epsilon();
}
