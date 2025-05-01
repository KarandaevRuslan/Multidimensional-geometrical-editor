#include "numTools.h"
#include <algorithm>
#include <cmath>
#include <limits>

bool isInteger(float value) {
    float intPart;
    float fracPart = std::modf(value, &intPart);
    return std::fabs(fracPart) < std::numeric_limits<float>::epsilon();
}

std::vector<std::size_t> sortedIds(const NDShape& sh)
{
    std::vector<std::size_t> ids;
    ids.reserve(sh.verticesSize());
    for(auto&& p:sh.getAllVertices()) ids.push_back(p.first);
    std::sort(ids.begin(),ids.end());
    return ids;
}
