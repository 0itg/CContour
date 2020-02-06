#pragma once

#include "ComplexPlane.h"
#include <complex>

class InputPanel;
class Contour;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

class OutputPanel : public ComplexPlane
{
    friend class InputPanel;
public:
    OutputPanel(wxWindow* parent, InputPanel* In);
    //~OutputPanel();
    void OnMouseLeftUp(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);

    int resolution = 100;
    std::function<std::complex<double>(std::complex<double>)> f =
        [](std::complex<double> z) { return z * z; };
private:
    InputPanel* in;
    //std::vector<Contour*> mappedContours;
    wxDECLARE_EVENT_TABLE();
};