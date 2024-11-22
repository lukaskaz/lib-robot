#pragma once

#include <random>
#include <string>

namespace robothelpers
{

std::string gettimestr();

template <typename T>
concept Numerical = std::is_arithmetic_v<T>;

template <Numerical T>
    requires std::is_arithmetic_v<T>
class Random
{
  public:
    explicit Random(const std::pair<T, T>& range) :
        gen(rd()), dist{range.first, range.second}
    {}

    T operator()()
    {
        return dist(gen);
    }

  private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<T> dist;
};

} // namespace robothelpers
