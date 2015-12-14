#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <memory>
#include <cmath>
#include <string>
#include <tuple>

using std::string;
using std::to_string;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;
using std::vector;
using std::tuple;
using std::tie;
using std::make_tuple;
using std::shared_ptr;

#include "include/io.h"
#include "include/matrix.h"
#include "src/MyObject.h"
#include "modellistener.h"
#include "pluginmanager.h"

#define PI 3.14159265

class MechanismReparer
{
    string input_path;
    string output_path;
    Observed observed;
    Filter *filter;

    int  objs_on_img = 0;

public:
    MechanismReparer();
    MechanismReparer(string &in, string &out, Observed &obs);

    void setInputPath(string in);
    void setOutputPath(string out);
    void setObserved(Observed &obs);
    void setFilter(Filter *f);

    void repair();
    void zero(IntMatr &m);
    void Fill(IntMatr &img, IntMatr &labels, int x, int y, int L);
    void Split(IntMatr &img);
    int round(int x, int R);
    float min(float a, float b);
    float max(float a, float b);
    bool noBgInCircle(const IntMatr &in, uint obj_No, int x, int y, int R);
    bool notFullBgCircle(const IntMatr &in, uint obj_No, int x, int y, int R);
    tuple<int, int, int, int> distanceTransform(const IntMatr &in, int obj_No);
    tuple<int, int, int, int> findBorders(const IntMatr &in, const uint obj);
    tuple<bool, int> countCogs(IntMatr &in, int X, int Y, int R, uint obj_No);
    void parseGears(IntMatr &in, vector<shared_ptr<IObject>> &array);
    void parseSpare(string path, vector<shared_ptr<IObject>> &array, vector<shared_ptr<IntMatr>> &binrs);
    tuple<int, int> findMissed(vector<shared_ptr<IObject>> &gears, vector<shared_ptr<IObject>> &vars);
    void putSpare(IntMatr &dest,const IntMatr *src, const IObject *to_gear, const IObject *from_gear);
    tuple<int, vector<shared_ptr<IObject>>, Image> repair_mechanism(Image &input, string path);
};

#endif // MODEL_H

