#include "options.hpp"
#include "env.hpp"
namespace zerodb
{
    Options::Options() : comparator(BytewiseComparator()), env(Env::Default()) {}
}