#pragma once

#include <string>

namespace robothelpers
{

std::string gettimestr();

template <typename T>
concept Numerical = std::is_arithmetic_v<T>;

} // namespace robothelpers
