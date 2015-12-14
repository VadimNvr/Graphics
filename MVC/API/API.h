#include "../include/io.h"
#include <string>
#include <tuple>

using std::tuple;
using std::string;

class Filter
{
    
public:
    
    virtual tuple<uint, uint, uint> operator () (const Image &m) const = 0;
    
    virtual string getName() const = 0;

    static const int radius = 2;
};
