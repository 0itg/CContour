#pragma once
#include "Contour.h"

class ContourPolygon;

class ContourCircle : public Contour
{
public:
    ContourCircle(std::complex<double> c, double r = 0);
    virtual ContourCircle* Clone() { return new ContourCircle(*this); };

    void Draw(wxDC* dc, ComplexPlane* canvas);
    void moveCtrlPoint(std::complex<double> mousePos, int ptIndex = -1);
    // Changes the radius, rather than translating.
    void ActionNoCtrlPoint(std::complex<double> mousePos,
        std::complex<double> lastPointClicked);
    bool IsDone() { return true; }
    bool IsOnContour(std::complex<double> pt,
        ComplexPlane* canvas, int pixPrecision = 3);
    int OnCtrlPoint(std::complex<double> pt,
        ComplexPlane* canvas, int pixPrecision = 3);
    std::complex<double> Interpolate(double t);
    ContourPolygon* Subdivide(int res);

private:
    double radius;
};