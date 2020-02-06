#include "OutputPanel.h"
#include "InputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "ContourLine.h"

wxBEGIN_EVENT_TABLE(InputPanel, wxPanel)
EVT_LEFT_UP(InputPanel::OnMouseLeftUp)
//EVT_LEFT_DOWN(InputPanel::OnMouseLeftDown)
EVT_RIGHT_UP(ComplexPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
//EVT_MIDDLE_DOWN(InputPanel::OnMouseMiddleDown)
//EVT_MIDDLE_UP(InputPanel::OnMouseMiddleUp)
EVT_MOUSEWHEEL(ComplexPlane::OnMouseWheel)
EVT_MOTION(InputPanel::OnMouseMoving)
EVT_KEY_UP(InputPanel::OnKeyDelete)
EVT_PAINT(InputPanel::OnPaint)
wxEND_EVENT_TABLE()

InputPanel::~InputPanel()
{
    for (auto C : subDivContours)
        delete C;
}

void InputPanel::OnMouseLeftUp(wxMouseEvent& mouse)
{
    // if state > -1, then a contour is selected for editing
    // and state equals the index of the contour.
    if (state > -1)
    {
        // If the contour is closed, finalize it by deselecting it
        if (drawnContours[state]->IsClosed())
        {
            ReleaseMouseIfAble();
            drawnContours[state]->Finalize();
            delete subDivContours[state];
            subDivContours[state] = drawnContours[state]->Subdivide(res);
            state = -1;
            highlightedContour = -1;
            highlightedCtrlPoint = -1;
        }
        // If not, this contour must have multiple control points,
        // so move on to the next one
        else
        {
            drawnContours[state]->
                AddPoint(ScreenToComplex(mouse.GetPosition()));
            highlightedCtrlPoint++;
        }
        Refresh();
        Update();
    }
    else if (state == STATE_IDLE)
    {
        CaptureMouseIfAble();
        // If a contour is not highlighted (< 0), then create a new one
        // And set to be the active contour.
        if (highlightedContour < 0)
        {
            drawnContours.push_back(CreateContour(mouse.GetPosition()));
            subDivContours.push_back(drawnContours.back()->Subdivide(res));
            state = drawnContours.size() - 1;
            highlightedContour = state;
        }
        // If not, then make the highlighted contour active.
        else
            state = highlightedContour;
    }
    else
    {
        state = STATE_IDLE;
    }
}

void InputPanel::OnMouseMoving(wxMouseEvent& mouse)
{
    // When the mouse moves, take contour-dependent action
    // depending on whether a control point is select or just part of
    // the contour. The control-point action should move the control
    // point. The non-control-point action is generally expected to
    // translate the contour, but it adjusts the radius of a circle.
    if (state > STATE_IDLE)
    {
        if (highlightedCtrlPoint < 0)
        {
            drawnContours[state]->ActionNoCtrlPoint(
                ScreenToComplex(mouse.GetPosition()), lastMousePos);
        }
        else
        {
            drawnContours[state]->
                moveCtrlPoint(ScreenToComplex(mouse.GetPosition()),
                    highlightedCtrlPoint);
        }
        delete subDivContours[state];
        subDivContours[state] =
            std::move(drawnContours[state]->Subdivide(res));
        Refresh();
        Update();
    }
    // When the mouse moves, recheck for highlighted contours
        // and control points (and automatically highlight the contour).
    else if (state == STATE_IDLE)
    {
        Highlight(mouse.GetPosition());
        for (auto out : outputs)
            out->highlightedContour = highlightedContour;
    }
    else if (state == STATE_PANNING)
    {
        Pan(mouse.GetPosition());
    }

    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPanel::OnKeyDelete(wxKeyEvent& Key)
{
    ReleaseMouseIfAble(); // Captured by OnMouseRightDown
// Right click while editing will delete the active contour
    if (highlightedContour > -1)
    {
        delete drawnContours[highlightedContour];
        drawnContours.erase(drawnContours.begin() + highlightedContour);
        delete subDivContours[highlightedContour];
        subDivContours.erase(subDivContours.begin() + highlightedContour);

        for (auto out : outputs)
        {
            delete out->drawnContours[highlightedContour];
            out->drawnContours.erase(out->drawnContours.begin() + highlightedContour);
            out->highlightedContour = -1;
            out->highlightedCtrlPoint = -1;
        }

        state = STATE_IDLE;
        highlightedContour = -1;
        highlightedCtrlPoint = -1;
        Refresh();
        Update();
    }
}

void InputPanel::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(*wxRED, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    for (auto C : drawnContours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }

    if (highlightedContour > -1)
    {
        pen.SetColour(drawnContours[highlightedContour]->color);
        dc.SetPen(pen);

        if (highlightedCtrlPoint > -1)
        {
            dc.DrawCircle(ComplexToScreen(drawnContours[highlightedContour]->
                GetCtrlPoint(highlightedCtrlPoint)), 7);
        }
        pen.SetWidth(2);
        dc.SetPen(pen);
        drawnContours[highlightedContour]->Draw(&dc, this);
    }
    axes.Draw(&dc);

    for (auto out : outputs)
    {
        out->Update();
        out->Refresh();
    }
}

void InputPanel::SetContourStyle(int id)
{
    contourStyle = id;
}

Contour* InputPanel::CreateContour(wxPoint mousePos)
{
    switch (contourStyle)
    {
    case ID_Circle:
        return new ContourCircle(ScreenToComplex(mousePos));
        break;
    case ID_Rect:
        highlightedCtrlPoint = 1;
        return new ContourRect(ScreenToComplex(mousePos));
        break;
    case ID_Polygon:
        highlightedCtrlPoint = 1;
        return new ContourPolygon(ScreenToComplex(mousePos));
        break;
    case ID_Line:
        highlightedCtrlPoint = 1;
        return new ContourLine(ScreenToComplex(mousePos));
        break;
    }
    return new ContourCircle(ScreenToComplex(mousePos));
}