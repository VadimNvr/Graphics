#include "model.h"

class LevelFilterOp
{
public:
    uint operator () (const Image &m) const
    {
        uint r, g, b;

        tie(r, g, b) = m(0, 0);

        if (r > 100)
            return 1;

        return g > 100 ? 1 : 0;
    }

    static const int radius = 0;
};

class GroupsToImgOp
{
public:
    tuple<uint, uint, uint> operator () (const IntMatr &m) const
    {
        uint group = m(0, 0);

        switch (group)
        {
            case 0:
            return make_tuple(0x000, 0x000, 0x000);

            case 0xfff:
            return make_tuple(250, 0x000, 0x000);

            default:
            return make_tuple(0x000, 250, 0x000);
        }
    }
    static const int radius = 0;
};

MechanismReparer::MechanismReparer(): filter(0) {}

MechanismReparer::MechanismReparer(string &in, string &out, Observed &obs): filter(0)
{
    input_path = in;
    output_path = out;
    observed = obs;
}

void MechanismReparer::setInputPath(string in)
{
    input_path = in;
}
void MechanismReparer::setOutputPath(string out)
{
    output_path = out;
}

void MechanismReparer::setObserved(Observed &obs)
{
    observed = obs;
}

void MechanismReparer::setFilter(Filter *f)
{
    filter = f;
}

void MechanismReparer::repair()
{
    if(!input_path.empty() && !output_path.empty())
    {
        vector<shared_ptr<IObject>> gears_array;
        Image img = load_image(input_path.c_str());
        Image result;

        int spare_id;

        observed.send("Starting the process!");

        if (filter)
            observed.send(QString("Applying filter: ")+QString(filter->getName().c_str()));

        tie(spare_id, gears_array, result) = repair_mechanism(img, input_path.c_str());

        //result = img.unary_map(LevelFilterOp()).unary_map(GroupsToImgOp());
        //result = img;
        observed.send("Mechanism has been repaired!\n");
        //observed.send("");

        for (auto elem: gears_array)
        {
            elem->Write(observed);
        }

        if (filter)
            result = result.unary_map(*filter);

        save_image(result, output_path.c_str());
    }
}

void MechanismReparer::zero(IntMatr &m)
{
    for (uint i = 0; i < m.n_rows; ++i)
    for (uint j = 0; j < m.n_cols; ++j)
        m(i, j) = 0;
}

/*######### RECURSIVE METHOD FROM LECTION MATERIALS #########*/
void MechanismReparer::Fill(IntMatr &img, IntMatr &labels, int x, int y, int L)
{
    if ( (labels(y, x) == 0) && (img(y, x) > 0) )
    {
        int H = img.n_rows;
        int W = img.n_cols;

        labels(y, x) = L;

        if ( x > 0 ) 
            Fill(img, labels, x - 1, y, L);

        if ( x < W - 1 ) 
            Fill(img, labels, x + 1, y, L);

        if ( y > 0 ) 
            Fill(img, labels, x, y - 1, L);

        if ( y < H - 1 ) 
            Fill(img, labels, x, y + 1, L);
    }
}

void MechanismReparer::Split(IntMatr &img)
{
    int H = img.n_rows;
    int W = img.n_cols;

    IntMatr labels(H, W);
    zero(labels);

    for(int x = 0; x < W; ++x)
    for(int y = 0; y < H; ++y)
        if ( (labels(y, x) == 0) && (img(y, x) > 0) )
            Fill(img, labels, x, y, ++objs_on_img);

    img = labels.deep_copy();
}
/*##########################################################*/

int MechanismReparer::round(int x, int R)
{
    return int(sqrt( pow(R, 2) - pow(x, 2) ));
}

float MechanismReparer::min(float a, float b)
{
    return a <= b ? a : b;
}

float MechanismReparer::max(float a, float b)
{
    return a >= b ? a : b;
}

bool MechanismReparer::noBgInCircle(const IntMatr &in, uint obj_No, int x, int y, int R)
{
    int H = in.n_rows;
    int W = in.n_cols;

    if ( (x - R < 0) || (x + R >= W) || (y - R < 0) || (y + R >= H) )
        return false;

    for (int dx = -R; dx <= R; dx++)
    {
        int dy = round(dx, R);

        if ( (in(y+dy, x+dx) != obj_No) || 
             (in(y-dy, x+dx) != obj_No)   
           )
            return false;
    }

    return true;
}

bool MechanismReparer::notFullBgCircle(const IntMatr &in, uint obj_No, int x, int y, int R)
{
    int H = in.n_rows;
    int W = in.n_cols;

    for (int dx = 0; dx <= R; dx++)
    {
        int dy = round(dx, R);

        if ( (x+dx <  W) && (y+dy <  H) && (in(y+dy, x+dx) == obj_No) )
            return true;

        if ( (x+dx <  W) && (y-dy >= 0) && (in(y-dy, x+dx) == obj_No) )
            return true;

        if ( (x-dx >= 0) && (y+dy <  H) && (in(y+dy, x-dx) == obj_No) )
            return true;

        if ( (x-dx >= 0) && (y-dy >= 0) && (in(y-dy, x-dx) == obj_No) )
            return true;
    }

    return false;
}

tuple<int, int, int, int> MechanismReparer::distanceTransform(const IntMatr &in, int obj_No)
{
    int H = in.n_rows;
    int W = in.n_cols;

    int inside_R = 0;
    int outside_R = 0;
    int center_X;
    int center_Y;

    int last_R = 2;

    /* CREATING A MAP OF DISTANCE FROM THE BORDER, 
       AND FINDING INSIDE RADIUS */
    for (int x = W/3; x < W*2/3; x++)
    for (int y = H/3; y < H*2/3; y++)
    if ( in(y, x) != 0 )
    {
        int cur_R = last_R - 2;

        while (noBgInCircle(in, obj_No, x, y, ++cur_R)) { /* DO NOTHING */  }

        if (cur_R > inside_R)
        {
            inside_R = cur_R - 1;
            center_X = x;
            center_Y = y;
        }

        last_R = cur_R;
    }

    /* FINDING OUTSIDE RADIUS */
    int cur_R = inside_R;

    while (notFullBgCircle(in, obj_No, center_X, center_Y, ++cur_R)) { /* DO NOTHING */ }

    outside_R = cur_R - 1;

    return make_tuple(inside_R, outside_R, center_X, center_Y);
}

tuple<int, int, int, int> 
MechanismReparer::findBorders(const IntMatr &in, const uint obj)
{
    int W = in.n_cols,
        H = in.n_rows;

    int l = W; 
    int r = 0; 
    int t = H; 
    int b = 0;

    for (int x = 0; x < W; x++)
    for (int y = 0; y < H; y++)
    if ( in(y, x) == obj ) 
    {
        if (x < l) l = x;
        if (x > r) r = x;
        if (y < t) t = y;
        if (y > b) b = y;
    }

    return make_tuple(l - 1, r + 1, t - 1, b + 1);
}

tuple<bool, int> MechanismReparer::countCogs(IntMatr &in, int X, int Y, int R, uint obj_No)
{
    int count  = 0;
    bool in_cog = true;

    int alpha0;

    for (int angle = 0; angle <= 360; ++angle)
    {
        int dx = R * cos(PI*angle / 180);
        int dy = R * sin(PI*angle / 180);

        if ( (in(Y+dy, X+dx) == obj_No) && !in_cog )
        {
            count++;
            alpha0 = angle + 5;
            in_cog = true;
        }
        else
        if ( (in(Y+dy, X+dx) != obj_No) && in_cog )
            in_cog = false;
    }

    alpha0 += 360 / pow(count, 2);

    for (int cog = 0; cog < count; cog++)
    {
        int dx = R * cos(2*PI*cog/count + PI*alpha0/180);
        int dy = R * sin(2*PI*cog/count + PI*alpha0/180);

        if (in(Y+dy, X+dx) != obj_No)
        {
            observed.send("Number of cogs was counted");
            return make_tuple(true, count);
        }
    }

    observed.send("Number of cogs was counted");
    return make_tuple(false, count);
}

void MechanismReparer::parseGears(IntMatr &in, vector<shared_ptr<IObject>> &array)
{
    for (int it = 1; it <= objs_on_img; it++)
    {
        int l, r, t, b;

        int inside_R;
        int outside_R;
        int center_X;
        int center_Y;

        IntMatr dist_map;

        tie(l, r, t, b) = findBorders(in, it);

        tie(inside_R, outside_R, center_X, center_Y) = distanceTransform(in.submatrix(t, l, b - t + 1, r - l + 1), it);

        center_Y += t;
        center_X += l;

        if (inside_R == outside_R)
        {
            observed.send("Axis was found");
            array.push_back(shared_ptr<IObject>( new Axis(make_tuple(center_X, center_Y)) ));
        }

        else
        {
            bool is_broken;
            int cogs;
            tie(is_broken, cogs) = countCogs(in, center_X, center_Y, (inside_R + outside_R)/2, it);

            if (is_broken)
                observed.send("Broken gear was found");

            array.push_back(shared_ptr<IObject>( new Gear(make_tuple(center_X, center_Y), inside_R, outside_R, is_broken, cogs) ));
        }
    }
}

void MechanismReparer::parseSpare(string path, vector<shared_ptr<IObject>> &array, vector<shared_ptr<IntMatr>> &binrs)
{
    path.resize(path.size() - 4);

    for (int it = 1; it <= 3; it++)
    {
        int inside_R;
        int outside_R;
        int center_X;
        int center_Y;

        Image gear_img = load_image( (path + "_" + to_string(it) + ".bmp").c_str() );

        IntMatr binary = gear_img.unary_map(LevelFilterOp());

        tie(inside_R, outside_R, center_X, center_Y) = distanceTransform(binary, 1);

        binrs.push_back(shared_ptr<IntMatr>( new IntMatr(binary) ));
        array.push_back(shared_ptr<IObject>( new Gear(make_tuple(center_X, center_Y), inside_R, outside_R, false, 0) ));
    }
}

tuple<int, int> MechanismReparer::findMissed(vector<shared_ptr<IObject>> &gears, vector<shared_ptr<IObject>> &vars)
{
    int x0, y0;
    int x1, y1;
    int x2, y2;

    float in_R0, out_R0;
    float in_R1, out_R1;
    float in_R2, out_R2;
    float max_out_R0;

    for (uint it = 0; it < gears.size(); ++it)
    {
        IObject *obj = (gears[it]).get();

        if ( (obj->gettype() == AXLE) || (dynamic_cast<Gear *>(obj)->is_broken) )
        {
            tie(x0, y0) = gears[it]  ->location;

            if (it > 0)
            {
                tie(x1, y1) = gears[it-1]->location;
                tie(in_R1, out_R1) = gears[it-1]->getR();
            }
            if (it < gears.size()-1)
            {
                tie(x2, y2) = gears[it+1]->location;
                tie(in_R2, out_R2) = gears[it+1]->getR();
            }

            if ((it > 0) && (it < gears.size()-1))
            {
                out_R0 = min( sqrt( pow(x1-x0, 2) + pow(y1-y0, 2) ) - in_R1,
                                sqrt( pow(x2-x0, 2) + pow(y2-y0, 2) ) - in_R2   );

                max_out_R0 = max( sqrt( pow(x1-x0, 2) + pow(y1-y0, 2) ) - in_R1,
                                sqrt( pow(x2-x0, 2) + pow(y2-y0, 2) ) - in_R2   );

                in_R0  = min( sqrt( pow(x1-x0, 2) + pow(y1-y0, 2) ) - out_R1,
                                sqrt( pow(x2-x0, 2) + pow(y2-y0, 2) ) - out_R2  );
            }
            else if (it > 0)
            {
                out_R0 = max_out_R0 = sqrt( pow(x1-x0, 2) + pow(y1-y0, 2) ) - in_R1;
                in_R0  = sqrt( pow(x1-x0, 2) + pow(y1-y0, 2) ) - out_R1;
            }
            else
            {
                out_R0 = max_out_R0 = sqrt( pow(x2-x0, 2) + pow(y2-y0, 2) ) - in_R2;
                in_R0  = sqrt( pow(x2-x0, 2) + pow(y2-y0, 2) ) - out_R2;
            }

            for (uint it2 = 0; it2 < vars.size(); ++it2)
            {
                float in_R3, out_R3;

                tie(in_R3, out_R3) = vars[it2]->getR();

                if ( (in_R3 < in_R0) && (out_R3 < out_R0) && (out_R3 > in_R0) )
                    return make_tuple(it, it2 + 1);
            }

        }
    }

    return make_tuple(0, 0);
}

void MechanismReparer::putSpare(IntMatr &dest,const IntMatr *src, const IObject *to_gear, const IObject *from_gear)
{
    int target_X;
    int target_Y;
    int source_X;
    int source_Y;
    int H = src->n_rows, 
        W = src->n_cols;

    tie(target_X, target_Y) = to_gear  ->location;
    tie(source_X, source_Y) = from_gear->location;

    for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
    {
        int dx = j - source_X;
        int dy = i - source_Y;

        if ( (*src)(i, j) > 0 )
            dest(target_Y + dy, target_X + dx) = 0xfff;
    }
}

tuple<int, vector<shared_ptr<IObject>>, Image>
MechanismReparer::repair_mechanism(Image &input, string path)
{
    auto object_array = vector<shared_ptr<IObject>>();
    auto spare_gears  = vector<shared_ptr<IObject>>();
    auto spare_images = vector<shared_ptr<IntMatr>>();
    int  result_idx,   // Chosen spare gear
         tochange_idx; // Gear in a mechanism to change

    /* Parsing spare parts and collecting its information in spare_gears*/
    parseSpare(path, spare_gears, spare_images);

    /* Binarization */
    IntMatr img = input.unary_map(LevelFilterOp());

    observed.send("Level filter was applied");

    /* Split gears to groups */
    Split(img);

    observed.send("Gears were split to components");

    /* Parsing gears and collecting its information in object_array */
    parseGears(img, object_array);

    tie(tochange_idx, result_idx) = findMissed(object_array, spare_gears);

    /* Putting spare gear to source image */
    path.resize(path.size() - 4);
    Image spare = load_image( (path+"_"+to_string(result_idx)+".bmp").c_str() );

    IntMatr bin_spare = spare.unary_map(LevelFilterOp());

    putSpare( /* to */ img, 
              /* from */  spare_images[result_idx - 1].get(), 
              /* to_gear */ object_array[tochange_idx].get(),
              /* from gear */ spare_gears[result_idx - 1].get() 
            );

    Image src = img.unary_map(GroupsToImgOp());

    return make_tuple(result_idx, object_array, src.deep_copy());
}
