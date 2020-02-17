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

enum enum_buttons
{
    ID_toolBar,

    ID_Circle,
    ID_Rect,
    ID_Polygon,
    ID_Line,

    ID_Paintbrush,
    ID_Color_Randomizer,
    ID_Color_Picker,
    ID_Function_Entry
};

enum enum_states
{
    // values of state >= 0 represent contour indices.
    STATE_IDLE = -1,
};

class Contour;
class OutputPlane;
class ComplexPlane;
class Grid;

struct Axes {
    Axes(ComplexPlane* p) : parent(p) {};
    ComplexPlane* parent;
    union {
        double c[4] = { -10, 10, -10, 10 };
        struct {
            double realMin;
            double realMax;
            double imagMin;
            double imagMax;
        };
    };
    double reStep = 1;
    double imStep = 1;
    void Draw(wxDC* dc);
    const int TICK_WIDTH = 8;
    // Used to determine when the Step values should be automatically increased
    // So the tick marks don't get too close together or too far apart during
    // zoom. Min number of ticks is value / 2, max is value * 2.
    const int TARGET_TICK_COUNT = 20;
    // Label every n tick marks
    const int LABEL_SPACING = 4;
    // Labels can get no closer than this to the edge of the plane
    const int LABEL_PADDING = 4;
};

class ComplexPlane : public wxPanel
{
public:
    ComplexPlane(wxFrame* parent);
    virtual ~ComplexPlane();

    // Functions for converting between screen and mathematical coordinates.
    std::complex<double> ScreenToComplex(wxPoint P);
    wxPoint ComplexToScreen(std::complex<double> C);
    double LengthToScreen(double r);
    double ScreenToLength(double r);

    void OnMouseWheel(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseCapLost(wxMouseCaptureLostEvent& mouse);
    //void OnMouseEntering(wxMouseEvent& mouse);
    void OnMouseLeaving(wxMouseEvent& mouse);

    // For convenience
    void CaptureMouseIfAble() { if (!HasCapture()) CaptureMouse(); }
    void ReleaseMouseIfAble() { if (HasCapture())  ReleaseMouse(); }

    // Flags the contour/control point under this point as highlighted,
    // which marks it for special treatment in the user input and paint
    // routines
    void Highlight(wxPoint mousePos);

    void Pan(wxPoint mousePos);
    //void InversePan(wxPoint mousePos);
    void Zoom(wxPoint mousePos, int zoomSteps);

    Axes axes;

protected:
    std::vector<Contour*> contours;
    int highlightedContour = -1;
    int state = -1;
    int highlightedCtrlPoint = -1;
    int res = 100;
    const double zoomFactor = 1.1;
    std::complex<double> lastMousePos;
    std::complex<double> lastMidClick;

    bool panning = false;
    bool movedViewPort = true;
    wxStatusBar* statBar;
};