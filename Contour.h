#pragma once

//#include "ComplexPlane.h"

#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>

struct Axes;
class InputPanel;
class ContourPolygon;
class ComplexPlane;

// All contours include a set of points, may be open or closed,
// and need to define a function for interpolating the points
// (e.g. straight lines for polygons, or some spline function)

class Contour
{
public:
    virtual ~Contour() {}
    virtual Contour* Clone() = 0;

    virtual void Draw(wxDC* dc, ComplexPlane* canvas) = 0;
    virtual void AddPoint(std::complex<double> mousePos);
    virtual void RemovePoint(int index);
    virtual void moveCtrlPoint(std::complex<double> mousePos, int ptIndex = -1);
    virtual void ActionNoCtrlPoint(std::complex<double> mousePos,
        std::complex<double> lastPointClicked) = 0;
    virtual void Translate(std::complex<double> mousePos,
        std::complex<double> lastPointClicked);
    virtual bool IsClosed() = 0;
    virtual bool IsOnContour(std::complex<double> pt, ComplexPlane* canvas,
        int pixPrecision = 3) = 0;
    virtual int OnCtrlPoint(std::complex<double> pt, ComplexPlane* canvas,
        int pixPrecision = 3);
    virtual std::complex<double> GetCtrlPoint(int index);
    virtual void Finalize() {};
    virtual std::complex<double> Interpolate(double t) = 0;
    virtual ContourPolygon* Subdivide(int res) = 0;
    int PointCount() { return points.size(); }

    void DrawCtrlPoint(wxDC* dc, wxPoint p);
    wxColor color = *wxRED;
protected:
    std::vector<std::complex<double>> points;
};

double DistancePointToLine(std::complex<double> pt,
    std::complex<double> z1, std::complex<double>z2);
bool IsInsideLine(std::complex<double> pt,
    std::complex<double> z1, std::complex<double>z2);