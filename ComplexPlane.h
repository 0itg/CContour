#pragma once

#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcbuffer.h"
#include "wx/display.h"

#include <complex>
#include <functional>

enum
{
    ID_Hello,
    ID_toolBar,
    ID_Circle,
    ID_Rect,
    ID_Polygon
};

class Contour;
class OutputPanel;
struct windowSettings {
    double realMin = -10;
    double realMax = 10;
    double imagMin = -10;
    double imagMax = 10;
};

// Left Panel in UI. User draws on the panel, then the points are stored as
// complex numbers and mapped to the ouput panel under some complex function. 

class ComplexPlane : public wxPanel
{
public:
    ComplexPlane(wxWindow* parent) :
        wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxFULL_REPAINT_ON_RESIZE) { SetBackgroundStyle(wxBG_STYLE_CUSTOM); }
    virtual ~ComplexPlane() {}

    std::complex<double> ScreenToComplex(wxPoint P);
    wxPoint ComplexToScreen(std::complex<double> C);
    double LengthToScreen(double r);
    double ScreenToLength(double r);

    windowSettings axes;
protected:
    int highlightedContour = -1;
    int activeContour = -1;
    int highlightedCtrlPoint = -1;
    int resolution = 100;
};