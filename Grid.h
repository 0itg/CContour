#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>
//#include "Contour.h"

class ContourLine;
class ContourPolygon;
class ComplexPlane;

class Grid
{
    friend class TransformedGrid;
public:
    Grid(ComplexPlane* par) : parent(par) {}
    Grid(ComplexPlane* par, double h, double v) :
        hStep(h), vStep(v), parent(par) {}
    ~Grid();

	virtual void Draw(wxDC* dc, ComplexPlane* canvas);
    void CalcVisibleGrid();

    //std::complex<double> Interpolate(double t);
    //ContourPolygon* Subdivide(int res);

    double hStep = 1;
    double vStep = 1;
    wxColor color = wxColor(225, 225, 225);
protected:
    ComplexPlane* parent;
private:
    std::vector<ContourLine*> horiz;
    std::vector<ContourLine*> vert;
};

class TransformedGrid : public Grid
{
public:
    TransformedGrid(ComplexPlane* par) : Grid(par) {}
    ~TransformedGrid();
    void Draw(wxDC* dc, ComplexPlane* canvas);
    void CalcVisibleGrid(Grid* grid,
        std::function<std::complex<double>(std::complex<double>)> f);
    int res = 80;
private:
    std::vector<ContourPolygon*> horiz;
    std::vector<ContourPolygon*> vert;
};