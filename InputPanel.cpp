#include "OutputPanel.h"
#include "InputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "ContourLine.h"
#include "Grid.h"

wxBEGIN_EVENT_TABLE(InputPanel, wxPanel)
EVT_LEFT_UP(InputPanel::OnMouseLeftUp)
//EVT_LEFT_DOWN(InputPanel::OnMouseLeftDown)
EVT_RIGHT_UP(InputPanel::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
//EVT_MIDDLE_DOWN(InputPanel::OnMouseMiddleDown)
//EVT_MIDDLE_UP(InputPanel::OnMouseMiddleUp)
EVT_MOUSEWHEEL(InputPanel::OnMouseWheel)
EVT_MOTION(InputPanel::OnMouseMoving)
EVT_KEY_UP(InputPanel::OnKeyDelete)
EVT_PAINT(InputPanel::OnPaint)
wxEND_EVENT_TABLE()

InputPanel::~InputPanel()
{
    for (auto C : subDivContours)
        delete C;
    delete grid;
}

InputPanel::InputPanel(wxWindow* parent)
    : ComplexPlane(parent)
{
    grid = new Grid(this);
}

void InputPanel::OnMouseLeftUp(wxMouseEvent& mouse)
{
    // if state > STATE_IDLE, then a contour is selected for editing
    // and state equals the index of the contour.
    if (state > STATE_IDLE)
    {
        // If the contour is closed, finalize and deselect it
        if (contours[state]->IsClosed())
        {
            ReleaseMouseIfAble();
            contours[state]->Finalize();
            // For now delete the whole subvidived contour and recalculate.
            // Later, could recalculate only the affected portion.
            delete subDivContours[state];
            subDivContours[state] = contours[state]->Subdivide(res);
            state = STATE_IDLE;
            highlightedContour = -1;
            highlightedCtrlPoint = -1;
        }
        // If not, user must be drawing a contour with multiple control points,
        // so move on to the next one
        else
        {
            contours[state]->
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
            contours.push_back(CreateContour(mouse.GetPosition()));
            subDivContours.push_back(contours.back()->Subdivide(res));
            state = contours.size() - 1;
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

void InputPanel::OnMouseRightUp(wxMouseEvent& mouse)
{
    ReleaseMouseIfAble();
    if (state > STATE_IDLE && highlightedCtrlPoint > -1)
    {
        int nextState = state;
        if (contours[state]->IsClosed())
        {
            contours[state]->RemovePoint(highlightedCtrlPoint);
            highlightedCtrlPoint = -1;
            highlightedContour = -1;
            nextState = STATE_IDLE;
        }
        else
        {
            contours[state]->RemovePoint(highlightedCtrlPoint - 1);
            highlightedCtrlPoint--;
        }
        if (contours[state]->PointCount() < 2)
        {
            RemoveContour(state);
            nextState = STATE_IDLE;
        }
        state = nextState;
        Refresh();
        Update();
    }
    ComplexPlane::OnMouseRightUp(mouse);
}

void InputPanel::OnMouseWheel(wxMouseEvent& mouse)
{
    ComplexPlane::OnMouseWheel(mouse);
    if (linkGridToAxes)
    {
        grid->hStep = axes.reStep;
        grid->vStep = axes.imStep;
    }
    Refresh();
    Update();
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
            contours[state]->ActionNoCtrlPoint(
                ScreenToComplex(mouse.GetPosition()), lastMousePos);
        }
        else
        {
            contours[state]->
                moveCtrlPoint(ScreenToComplex(mouse.GetPosition()),
                    highlightedCtrlPoint);
        }
        delete subDivContours[state];
        subDivContours[state] =
            std::move(contours[state]->Subdivide(res));
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
        RemoveContour(highlightedContour);
        state = STATE_IDLE;
        Refresh();
        Update();
    }
}

void InputPanel::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(grid->color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    grid->Draw(&dc, this);

    for (auto C : contours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }

    if (highlightedContour > -1)
    {
        pen.SetColour(contours[highlightedContour]->color);
        dc.SetPen(pen);

        if (highlightedCtrlPoint > -1)
        {
            dc.DrawCircle(ComplexToScreen(contours[highlightedContour]->
                GetCtrlPoint(highlightedCtrlPoint)), 7);
        }
        pen.SetWidth(2);
        dc.SetPen(pen);
        contours[highlightedContour]->Draw(&dc, this);
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

void InputPanel::RemoveContour(int index)
{
    delete contours[index];
    contours.erase(contours.begin() + index);
    delete subDivContours[index];
    subDivContours.erase(subDivContours.begin() + index);

    for (auto out : outputs)
    {
        delete out->contours[index];
        out->contours.erase(out->contours.begin() + index);
        out->highlightedContour = -1;
        out->highlightedCtrlPoint = -1;
    }
    highlightedContour = -1;
    highlightedCtrlPoint = -1;
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