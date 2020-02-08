#include "ComplexPlane.h"
#include "OutputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"
#include <complex>

ComplexPlane::ComplexPlane(wxWindow* parent) : axes(this),
wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxFULL_REPAINT_ON_RESIZE) {
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

ComplexPlane::~ComplexPlane()
{
    for (auto C : contours)
        delete C;
}

std::complex<double> ComplexPlane::ScreenToComplex(wxPoint P)
{
    return std::complex<double>((double)P.x /
        GetClientSize().x * (axes.realMax - axes.realMin)
        + axes.realMin, (double)P.y / GetClientSize().y
        * (axes.imagMax - axes.imagMin) + axes.imagMin);
}

wxPoint ComplexPlane::ComplexToScreen(std::complex<double> C)
{
    return wxPoint((C.real() - axes.realMin) / (axes.realMax - axes.realMin)
        * GetClientSize().x, (C.imag() - axes.imagMin) /
        (axes.imagMax - axes.imagMin) * GetClientSize().y);
}

double ComplexPlane::LengthToScreen(double r)
{
    return r * GetClientSize().x / (axes.realMax - axes.realMin);
}

double ComplexPlane::ScreenToLength(double r)
{
    return r * (axes.realMax - axes.realMin) / GetClientSize().x;
}

void ComplexPlane::OnMouseWheel(wxMouseEvent& mouse)
{
    int rot = mouse.GetWheelRotation() / mouse.GetWheelDelta();
    Zoom(mouse.GetPosition(), rot);
}

void ComplexPlane::OnMouseRightUp(wxMouseEvent& mouse)
{
    ReleaseMouseIfAble();
    if (state == STATE_PANNING) state = STATE_IDLE;
}

void ComplexPlane::OnMouseRightDown(wxMouseEvent& mouse)
{
    CaptureMouseIfAble();
    if (state == STATE_IDLE)
        state = STATE_PANNING;
}

void ComplexPlane::Highlight(wxPoint mousePos)
{
    bool notOnAnyContour = true;
    int lastHC = highlightedContour;
    int lastHCP = highlightedCtrlPoint;

    for (int i = 0; i < contours.size(); i++)
    {
        int CtrlPtIndex = contours[i]->
            OnCtrlPoint(ScreenToComplex(mousePos), this);
        if (CtrlPtIndex > -1)
        {
            notOnAnyContour = false;
            highlightedCtrlPoint = CtrlPtIndex;
            highlightedContour = i;
        }
        else if (contours[i]->
            IsOnContour(ScreenToComplex(mousePos), this))
        {
            notOnAnyContour = false;
            highlightedContour = i;
            highlightedCtrlPoint = -1;
        }
    }
    // Unhighlight the previously highlighted contour if the mouse
    // is not over one anymore.
    if (highlightedContour > -1 && notOnAnyContour)
    {
        highlightedContour = -1;
        highlightedCtrlPoint = -1;
    }
    // Only update the screen if different things are highlighted.
    // Theoretically more efficient.
    if (highlightedContour != lastHC || highlightedCtrlPoint != lastHCP)
    {
        Refresh();
        Update();
    }
}

void ComplexPlane::Pan(wxPoint mousePos)
{
    std::complex<double> displacement =
        lastMousePos - ScreenToComplex(mousePos);
    axes.realMax += displacement.real();
    axes.realMin += displacement.real();
    axes.imagMax += displacement.imag();
    axes.imagMin += displacement.imag();
    movedViewPort = true;
    Refresh();
    Update();
}

//void ComplexPlane::InversePan(wxPoint mousePos)
//{
//    std::complex<double> displacement =
//        lastMousePos - ScreenToComplex(mousePos);
//    axes.realMax -= displacement.real();
//    axes.realMin -= displacement.real();
//    axes.imagMax -= displacement.imag();
//    axes.imagMin -= displacement.imag();
//    Refresh();
//    Update();
//}

void ComplexPlane::Zoom(wxPoint mousePos, int zoomSteps)
{
    axes.realMax -= lastMousePos.real();
    axes.realMin -= lastMousePos.real();
    axes.imagMax -= lastMousePos.imag();
    axes.imagMin -= lastMousePos.imag();
    for (int i = 0; i < 4; i++)
    {
        axes.c[i] *= pow(zoomFactor, zoomSteps);
    }
    axes.realMax += lastMousePos.real();
    axes.realMin += lastMousePos.real();
    axes.imagMax += lastMousePos.imag();
    axes.imagMin += lastMousePos.imag();

    int MaxMark = GetClientSize().x / (axes.TARGET_HASH_COUNT / 2);
    const int MinMark = GetClientSize().x / (axes.TARGET_HASH_COUNT * 2);

    if (LengthToScreen(axes.reStep) < MinMark)
    { 
        axes.reStep *= 2;
    }
    else if (LengthToScreen(axes.reStep) > MaxMark)
    {
        axes.reStep /= 2;
    }
    if (LengthToScreen(axes.imStep) < MinMark)
    {
        axes.imStep *= 2;
    }
    else if (LengthToScreen(axes.imStep) > MaxMark)
    {
        axes.imStep /= 2;
    }
    movedViewPort = true;
    Refresh();
    Update();
}

void Axes::Draw(wxDC* dc)
{
    using namespace std::complex_literals;
    wxPoint center = parent->ComplexToScreen(0);
    wxPen pen(2);
    pen.SetColour(*wxBLACK);
    dc->SetPen(pen);
    dc->CrossHair(center);

    wxSize size = parent->GetClientSize();
    std::complex<double> cMark = 0;
    wxPoint mark;
    while (cMark.real() < realMax)
    {
        cMark += reStep;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x, mark.y + HASH_WIDTH / 2),
            wxPoint(mark.x, mark.y - HASH_WIDTH / 2));
    }
    cMark = 0;
    while (cMark.real() > realMin)
    {
        cMark -= reStep;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x, mark.y + HASH_WIDTH / 2),
            wxPoint(mark.x, mark.y - HASH_WIDTH / 2));
    }
    cMark = 0;
    while (cMark.imag() < imagMax)
    {
        cMark += imStep * 1i;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x + HASH_WIDTH / 2, mark.y),
            wxPoint(mark.x - HASH_WIDTH / 2, mark.y));
    }
    cMark = 0;
    while (cMark.imag() > imagMin)
    {
        cMark -= imStep * 1i;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x + HASH_WIDTH / 2, mark.y),
            wxPoint(mark.x - HASH_WIDTH / 2, mark.y));
    }
}

//double Dist(wxPoint X, wxPoint Y)
//{
//    double Xdist = X.x - Y.x;
//    double Ydist = X.y - Y.y;
//    return sqrt(Xdist * Xdist + Ydist * Ydist);
//}
