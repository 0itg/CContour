#pragma once
#include "ContourPolygon.h"

class ContourRect : public ContourPolygon
{
public:
    ContourRect(std::complex<double> c);
    ContourRect* Clone() { return new ContourRect(*this); };

    void Draw(wxDC* dc, ComplexPlane* canvas);
    void RemovePoint(int index);
    void moveCtrlPoint(std::complex<double> mousePos, int ptIndex = -1);
    bool IsDone() { return true; }
    void Finalize() {};
private:
    double perimeter;
    std::vector<double> sideLengths;
};
