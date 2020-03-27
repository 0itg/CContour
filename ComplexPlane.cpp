#include "ComplexPlane.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"
#include "OutputPlane.h"
#include "ToolPanel.h"

#include "Event_IDs.h"

#include <complex>
#include <iomanip>
#include <sstream>

ComplexPlane::ComplexPlane(wxWindow* parent, const std::string& n)
    : axes(this), wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                          wxFULL_REPAINT_ON_RESIZE),
      resCtrl(nullptr), statBar(nullptr), toolPanel(nullptr), name(n)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

cplx ComplexPlane::ScreenToComplex(wxPoint P)
{
    return cplx(
        (double)P.x / GetClientSize().x * (axes.realMax - axes.realMin) +
            axes.realMin,
        (1 - (double)P.y / GetClientSize().y) * (axes.imagMax - axes.imagMin) +
            axes.imagMin);
}

wxPoint ComplexPlane::ComplexToScreen(cplx C)
{
    return wxPoint((C.real() - axes.realMin) / (axes.realMax - axes.realMin) *
                       GetClientSize().x,
                   (axes.imagMin - C.imag()) / (axes.imagMax - axes.imagMin) *
                           GetClientSize().y +
                       GetClientSize().y);
}

double ComplexPlane::LengthXToScreen(double r)
{
    return r * GetClientSize().x / (axes.realMax - axes.realMin);
}

double ComplexPlane::LengthYToScreen(double r)
{
    return r * GetClientSize().y / (axes.imagMax - axes.imagMin);
}

double ComplexPlane::ScreenXToLength(double r)
{
    return r * (axes.realMax - axes.realMin) / GetClientSize().x;
}

double ComplexPlane::ScreenYToLength(double r)
{
    return r * (axes.imagMax - axes.imagMin) / GetClientSize().y;
}

void ComplexPlane::OnMouseWheel(wxMouseEvent& mouse)
{
    int rot = mouse.GetWheelRotation() / mouse.GetWheelDelta();
    Zoom(mouse.GetPosition(), rot);
    Update();
    Refresh();
}

void ComplexPlane::OnMouseLeftDown(wxMouseEvent& mouse)
{
    mouseLeftDown = true;
}

void ComplexPlane::OnMouseRightUp(wxMouseEvent& mouse)
{
    ReleaseMouseIfAble();
    panning = false;
    history->UpdateLastCommand(ScreenToComplex(mouse.GetPosition()));
}

void ComplexPlane::OnMouseRightDown(wxMouseEvent& mouse)
{
    CaptureMouseIfAble();
    history->RecordCommand(std::make_unique<CommandAxesSet>(this));
    panning = true;
}

void ComplexPlane::OnMouseCapLost(wxMouseCaptureLostEvent& mouse)
{
    ReleaseMouseIfAble();
    panning = false;
    history->UpdateLastCommand(lastMousePos);
}

void ComplexPlane::OnMouseLeaving(wxMouseEvent& mouse)
{
    statBar->SetStatusText("", 0);
    statBar->SetStatusText("", 1);
}

void ComplexPlane::OnShowAxes_ShowGrid(wxCommandEvent& event)
{
    switch (event.GetId())
    {
    case ID_Show_Axes:
        showAxes = !showAxes;
        break;
    case ID_Show_Grid:
        showGrid = !showGrid;
    }
    Update();
    Refresh();
}

void ComplexPlane::ShowAxisControls()
{
    if (toolPanel) toolPanel->PopulateAxisTextCtrls();
}

void ComplexPlane::RefreshShowAxes_ShowGrid()
{
    toolbar->ToggleTool(ID_Show_Axes, showAxes);
    toolbar->ToggleTool(ID_Show_Grid, showGrid);
}

bool ComplexPlane::Highlight(wxPoint mousePos)
{
    bool notOnAnyContour = true;
    int lastHC           = highlightedContour;
    int lastHCP          = highlightedCtrlPoint;

    for (int i = 0; i < contours.size(); i++)
    {
        int CtrlPtIndex =
            contours[i]->OnCtrlPoint(ScreenToComplex(mousePos), this);
        if (CtrlPtIndex > -1)
        {
            notOnAnyContour      = false;
            highlightedCtrlPoint = CtrlPtIndex;
            highlightedContour   = i;
        }
        else if (contours[i]->IsPointOnContour(ScreenToComplex(mousePos), this))
        {
            notOnAnyContour      = false;
            highlightedContour   = i;
            highlightedCtrlPoint = -1;
        }
    }
    // Unhighlight the previously highlighted contour if the mouse
    // is not over one anymore.
    if (highlightedContour > -1 && notOnAnyContour)
    {
        highlightedContour   = -1;
        highlightedCtrlPoint = -1;
    }
    // true signals that the highlighted contour has changed.
    return (highlightedContour != lastHC || highlightedCtrlPoint != lastHCP);
}

void ComplexPlane::Pan(wxPoint mousePos)
{
    cplx displacement = lastMousePos - ScreenToComplex(mousePos);
    axes.realMax += displacement.real();
    axes.realMin += displacement.real();
    axes.imagMax += displacement.imag();
    axes.imagMin += displacement.imag();
    movedViewPort = true;
    toolPanel->Update();
    toolPanel->Refresh();
}

void ComplexPlane::Zoom(wxPoint mousePos, int zoomSteps)
{
    cplx zoomCenter = ScreenToComplex(mousePos);
    // Zoom around the mouse position. To this, first translate the viewport
    // so mousePos is at the origin, then apply the zoom, then translate back.
    axes.realMax -= zoomCenter.real();
    axes.realMin -= zoomCenter.real();
    axes.imagMax -= zoomCenter.imag();
    axes.imagMin -= zoomCenter.imag();

    for (int i = 0; i < 4; i++)
    {
        axes.c[i] *= pow(zoomFactor, zoomSteps);
    }

    axes.realMax += zoomCenter.real();
    axes.realMin += zoomCenter.real();
    axes.imagMax += zoomCenter.imag();
    axes.imagMin += zoomCenter.imag();

    // If the user zooms in or symbolStack too far, the tick marks will get too
    // far apart or too close together. Rescale when they are more than twice
    // as far apart or half as far apart.

    axes.RecalcSteps();
    movedViewPort = true;
    toolPanel->Update();
    toolPanel->Refresh();
}

void ComplexPlane::ClearContours()
{
    contours.clear();
    highlightedContour   = -1;
    highlightedCtrlPoint = -1;
    state                = -1;
}

wxArrayString ComplexPlane::GetContourNames()
{
    wxArrayString names;

    for (auto& C : contours)
    {
        names.Add(C->GetName());
    }

    return names;
}

std::map<std::string, int> ComplexPlane::GetParametricContours(bool parametric)
{
    std::map<std::string, int> names;
    int index = 0;
    for (auto& C : contours)
    {
        if (C->IsParametric() == parametric)
            names[C->GetName()] = index;
        index++;
    }
    return names;
}

void Axes::Draw(wxDC* dc)
{
    using namespace std::complex_literals;
    wxPoint center = parent->ComplexToScreen(0);
    wxPen pen(2);
    pen.SetColour(*wxBLACK);
    dc->SetPen(pen);
    // CrossHair = axes through the given point
    dc->CrossHair(center);

    wxFont font = wxFont(wxFontInfo(6));

    // Draw the tick marks on the axes.
    wxSize size = parent->GetClientSize();
    wxPoint mark;

    // Get offset and adjust tick mark count so text aligns with zero.
    int count  = realMin / reStep;
    cplx cMark = realMin - fmod(realMin, reStep);

    while (cMark.real() < realMax)
    {
        count++;
        cMark += reStep;
        mark = parent->ComplexToScreen(cMark);

        wxSize textOffset = font.GetPixelSize();
        int textTopEdge   = -TICK_WIDTH;
        int textBottomEdge =
            parent->GetClientSize().y - TICK_WIDTH - font.GetPixelSize().y;

        // Draw the labels near the axis if it's on screen. Otherwise, draw
        // them at the top or bottom, depending on which side is closer
        // to the axis.
        if (center.y > 0 && center.y < parent->GetClientSize().y)
        {
            dc->DrawLine(mark.x, mark.y + TICK_WIDTH / 2, mark.x,
                         mark.y - TICK_WIDTH / 2);
        }
        if (center.y <= textTopEdge + LABEL_PADDING)
        { mark.y = textTopEdge + LABEL_PADDING; }
        else if (center.y > textBottomEdge - LABEL_PADDING)
        {
            mark.y = textBottomEdge - LABEL_PADDING;
        }

        if (count % LABEL_SPACING == 0 && count != 0)
        {
            std::ostringstream oss;
            oss << std::setprecision(4) << std::noshowpoint << cMark.real();
            std::string label = oss.str();
            dc->DrawText(label, mark.x - parent->GetTextExtent(label).x / 2,
                         mark.y + TICK_WIDTH / 2 + 1);
        }
    }

    // Treat the vertical axis similarly.

    count = imagMin / imStep;
    cMark = (imagMin - fmod(imagMin, imStep)) * 1i;

    while (cMark.imag() < imagMax)
    {
        count++;
        cMark += imStep * 1i;
        mark = parent->ComplexToScreen(cMark);
        std::ostringstream oss;
        oss << std::setprecision(4) << std::noshowpoint << cMark.imag() << 'i';
        std::string label = oss.str();
        wxSize textOffset = dc->GetTextExtent(label);

        int textLeftEdge  = textOffset.x + TICK_WIDTH;
        int textRightEdge = parent->GetClientSize().x + TICK_WIDTH;

        if (center.x > 0 && center.x < parent->GetClientSize().x)
        {
            dc->DrawLine(wxPoint(mark.x + TICK_WIDTH / 2, mark.y),
                         wxPoint(mark.x - TICK_WIDTH / 2, mark.y));
        }
        if (center.x <= textLeftEdge + LABEL_PADDING)
        { mark.x = textLeftEdge + LABEL_PADDING; }
        else if (center.x > textRightEdge - LABEL_PADDING)
        {
            mark.x = textRightEdge - LABEL_PADDING;
        }

        if (count % LABEL_SPACING == 0 && count != 0)
        {
            dc->DrawText(label, mark.x - textLeftEdge,
                         mark.y - textOffset.y / 2);
        }
    }
}

void Axes::RecalcSteps()
{
    const double MaxMark = parent->GetClientSize().x / (TARGET_TICK_COUNT / 2.0);
    const double MinMark = parent->GetClientSize().x / (TARGET_TICK_COUNT);
    if (parent->LengthXToScreen(reStep) < MinMark) { reStep *= 2; }
    else if (parent->LengthXToScreen(reStep) > MaxMark)
    {
        reStep /= 2;
    }
    if (parent->LengthYToScreen(imStep) < MinMark) { imStep *= 2; }
    else if (parent->LengthYToScreen(imStep) > MaxMark)
    {
        imStep /= 2;
    }
}
