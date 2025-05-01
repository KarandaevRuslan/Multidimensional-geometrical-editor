#ifndef NUM_TOOLS_H
#define NUM_TOOLS_H

#include "../model/NDShape.h"

bool isInteger(float value);
std::vector<std::size_t> sortedIds(const NDShape& sh);

#endif // NUM_TOOLS_H
