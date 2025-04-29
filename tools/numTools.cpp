#include "numTools.h"
#include <cmath>
#include <limits>

bool isInteger(float value) {
    float intPart;
    float fracPart = std::modf(value, &intPart);
    return std::fabs(fracPart) < std::numeric_limits<float>::epsilon();
}
