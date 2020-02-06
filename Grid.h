#pragma once
#include "Contour.h"

class GridLine;

class Grid
{
public:
    Grid(ComplexPlane* parent, double hStep, double vStep);
	void Draw(wxDC* dc, ComplexPlane* canvas);
    std::complex<double> Interpolate(double t);
    ContourPolygon* Subdivide(int res);
private:
    std::vector<GridLine*> horiz;
    std::vector<GridLine*> vert;
};

