#pragma once

#include "ComplexPlane.h"
#include <complex>

class InputPlane;
class Contour;
class TransformedGrid;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

class OutputPlane : public ComplexPlane
{
    friend class InputPlane;
public:
    OutputPlane(wxWindow* parent, InputPlane* In);
    ~OutputPlane();
    void OnMouseLeftUp(wxMouseEvent& mouse);
    //void OnMouseRightUp(wxMouseEvent& mouse);
    //void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);

    int res = 100;
    std::function<std::complex<double>(std::complex<double>)> f =
        [](std::complex<double> z) { return z * z; };
private:
    InputPlane* in;
    TransformedGrid* tGrid;
    wxDECLARE_EVENT_TABLE();
};