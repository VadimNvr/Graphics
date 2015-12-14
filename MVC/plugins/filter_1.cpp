#include "../API/API.h"

#include <initializer_list>
#include <iostream>
#include <vector>
#include <algorithm>

using std::tie;
using std::string;
using std::make_tuple;
using std::vector;

class BoxFilter: public Filter
{
    string name;
    
public:
    BoxFilter(): name("Box - фильтр") {};
    
    string getName() const { return name; }
    
    tuple<uint, uint, uint> operator () (const Image &m) const
    {
        uint size = 2 * radius + 1;
        uint r, g, b, sum_r = 0, sum_g = 0, sum_b = 0;
        for (uint i = 0; i < size; ++i) {
            for (uint j = 0; j < size; ++j) {
                tie(r, g, b) = m(i, j);
                sum_r += r;
                sum_g += g;
                sum_b += b;
            }
        }
        auto norm = size * size;
        sum_r /= norm;
        sum_g /= norm;
        sum_b /= norm;
        return make_tuple(sum_r, sum_g, sum_b);
    }
};

extern "C" Filter* create_object()
{
    return new BoxFilter;
}

extern "C" void destroy_object( Filter* object )
{
    delete object;
}