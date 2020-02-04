#pragma once

#include "ComplexPlane.h"

#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>

struct windowSettings;
class InputPanel;
class ContourPolygon;

// All contours include a set of points, may be open or closed,
// and need to define a function for interpolating the points
// (e.g. straight lines for polygons, or some spline function)

class Contour
{
public:
    virtual ~Contour() {}
    virtual Contour* Clone() = 0;

    std::vector<std::complex<double>> points;
    virtual void Draw(wxDC* dc, ComplexPlane* canvas, windowSettings axes) = 0;
    virtual void AddPoint(std::complex<double> mousePos);
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

    void DrawCtrlPoint(wxDC* dc, wxPoint p);
    wxColor color = *wxRED;
};