#pragma once

#include "ComplexPlane.h"
#include <complex>
#include "Parser.h"

class InputPlane;
class Contour;
class TransformedGrid;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

class OutputPlane : public ComplexPlane
{
    friend class InputPlane;
public:
    OutputPlane(wxFrame* parent, InputPlane* In);
    ~OutputPlane();
    void OnMouseLeftUp(wxMouseEvent& mouse);
    //void OnMouseRightUp(wxMouseEvent& mouse);
    //void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);
    void OnFunctionEntry(wxCommandEvent& event);

    void SetFuncInput(wxTextCtrl* fIn) { funcInput = fIn; }

    int res = 200;

    Parser<std::complex<double>> f;
private:
    InputPlane* in;
    TransformedGrid* tGrid;
    wxTextCtrl* funcInput;
    wxDECLARE_EVENT_TABLE();
};