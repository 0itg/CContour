#pragma once
#include "ContourPolygon.h"

class ContourRect : public ContourPolygon
{
public:
    ContourRect(std::complex<double> c);
    ContourRect* Clone() { return new ContourRect(*this); };

    void Draw(wxDC* dc, ComplexPlane* canvas, windowSettings axes);
    void moveCtrlPoint(std::complex<double> mousePos, int ptIndex = -1);
    bool IsClosed() { return true; }
    void Finalize() {};
private:
    double perimeter;
    std::vector<double> sideLengths;
};
