#pragma once

#include "ComplexPlane.h"
#include <complex>

class Contour;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

class OutputPanel : public ComplexPlane
{
    friend class InputPanel;
public:
    windowSettings axes;

    int resolution = 100;
    OutputPanel(wxWindow* parent) : ComplexPlane(parent) {};
    ~OutputPanel();
    void OnPaint(wxPaintEvent& paint);

private:
    std::vector<Contour*> mappedContours;
    wxDECLARE_EVENT_TABLE();
};