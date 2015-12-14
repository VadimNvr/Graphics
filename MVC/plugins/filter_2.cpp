#include "../API/API.h"

#include <initializer_list>
#include <iostream>
#include <vector>
#include <algorithm>

using std::tie;
using std::string;
using std::make_tuple;
using std::vector;

class MedianFilter: public Filter
{
    string name;
    
public:
    MedianFilter(): name("Median - фильтр") {};
    
    string getName() const { return name; }
    
    tuple<uint, uint, uint> operator () (const Image &m) const
    {
        uint size = 2 * radius + 1;
        uint r, g, b;
        vector<uint> r_array, g_array, b_array;
        
        for (uint i = 0; i < size; ++i) {
            for (uint j = 0; j < size; ++j) {
                tie(r, g, b) = m(i, j);
                r_array.push_back(r);
                g_array.push_back(g);
                b_array.push_back(b);
            }
        }
        
        std::sort(r_array.begin(), r_array.end());
        std::sort(g_array.begin(), g_array.end());
        std::sort(b_array.begin(), b_array.end());
        
        auto middle = size * size / 2;
        r = r_array[middle];
        g = g_array[middle];
        b = b_array[middle];
        
        return make_tuple(r, g, b);
    }
};

extern "C" Filter* create_object()
{
    return new MedianFilter;
}

extern "C" void destroy_object( Filter* object )
{
    delete object;
}
