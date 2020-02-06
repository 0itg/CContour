#pragma once
#include "ContourLine.h"

enum enum_gridLineType
{
    GRIDLINE_VERT,
    GRIDLINE_HORIZ
};

class GridLine : public ContourLine
{
public:
    GridLine(std::complex<double> c, int t);
    void Draw(wxDC* dc, ComplexPlane* canvas);
private:
    int type;
    //double scale = 1;
};

