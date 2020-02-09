#pragma once
#include "Contour.h"

class ContourPolygon : public Contour
{
public:
    ContourPolygon(std::complex<double> c, wxColor col = wxColor(255, 255, 255));
    ContourPolygon(wxColor col = wxColor(255, 255, 255));
    virtual ContourPolygon* Clone() { return new ContourPolygon(*this); };

    virtual void Draw(wxDC* dc, ComplexPlane* canvas);
    virtual void ActionNoCtrlPoint(std::complex<double> mousePos,
        std::complex<double> lastPointClicked);
    virtual bool IsDone();
    virtual bool IsOnContour(std::complex<double> pt,
        ComplexPlane* canvas, int pixPrecision = 3);
    virtual void Finalize();
    std::complex<double> Interpolate(double t);
    ContourPolygon* Subdivide(int res);
    ContourPolygon* Apply(std::function<std::complex<double>(std::complex<double>)> f);
protected:
    bool closed = false;
    double perimeter;
    std::vector<double> sideLengths;
    void CalcSideLengths();
};
