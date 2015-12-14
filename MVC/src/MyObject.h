#pragma once

#include <fstream>
#include <sstream>
#include <tuple>
#include "modellistener.h"

using std::tuple;
using std::ostream;
using std::endl;

enum objtype { GEAR, AXLE };

struct IObject
{
    IObject(tuple<int, int> in_location = tuple<int, int>(0, 0))
        : location(in_location)
    {}

    virtual ~IObject() {}
    virtual void Write(ostream &io_stream) = 0;
    virtual void Write(Observed &io_stream) = 0;
    virtual objtype gettype() const = 0;
    virtual tuple<int, int> getR() const { return make_tuple(0, 0); }

    tuple<int, int> location;
};

struct Gear
    : public IObject
{
    /// radius of the inscribed circle
    float min_r;
    /// radis of the circumscribed circle 
    float max_r;
    /// is the gear is broken?
    bool is_broken;
    /// number of cogs
    int num_cogs;

    Gear(tuple<int, int> in_location = tuple<int, int>(0, 0), float in_min_r = 0,
        float in_max_r = 0, bool in_is_broken = true, int in_num_cogs = 0)
        : IObject(in_location), min_r(in_min_r), max_r(in_max_r),
        is_broken(in_is_broken), num_cogs(in_num_cogs)
    {}
    
    void Write(ostream &io_stream)
    {
        int x, y;
        tie(x, y) = location;
        io_stream << "Gear " << x << ' ' << y << ' ' << max_r << ' ' <<
            min_r << ' ' << (is_broken ? "TRUE " : "FALSE ") <<  num_cogs << endl;
    }

    void Write(Observed &obs)
    {
        std::stringstream ss;
        int x, y;
        tie(x, y) = location;
        ss << "Gear " << x << ' ' << y << ' ' << max_r << ' ' <<
            min_r << ' ' << (is_broken ? "TRUE " : "FALSE ") <<  num_cogs;
        obs.send(ss.str().c_str());
    }

    objtype gettype() const { return GEAR; }

    tuple<int, int> getR() const { return make_tuple(min_r, max_r); }
};

struct Axis
    : public IObject
{
    Axis(tuple<int, int> in_location = tuple<int, int>(0, 0))
        : IObject(in_location)
    {}

    void Write(ostream &io_stream)
    {
        int x, y;
        tie(x, y) = location;
        io_stream << "Axis " << x << ' ' << y << endl;
    }

    void Write(Observed &obs)
    {
        std::stringstream ss;
        int x, y;
        tie(x, y) = location;
        ss << "Axis " << x << ' ' << y;
        obs.send(ss.str().c_str());
    }

    objtype gettype() const { return AXLE; }
};
