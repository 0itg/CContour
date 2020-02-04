#include "OutputPanel.h"
#include "InputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"

wxBEGIN_EVENT_TABLE(InputPanel, wxPanel)
EVT_LEFT_UP(InputPanel::OnMouseLeftUp)
//EVT_LEFT_DOWN(InputPanel::OnMouseLeftDown)
EVT_RIGHT_UP(InputPanel::OnMouseRightUp)
//EVT_RIGHT_DOWN(InputPanel::OnMouseRightUp)
//EVT_LEAVE_WINDOW(InputPanel::OnMouseLeave)
EVT_MOTION(InputPanel::OnMouseMoving)
EVT_PAINT(InputPanel::OnPaint)
wxEND_EVENT_TABLE()

InputPanel::~InputPanel()
{
    for (auto C : drawnContours)
        delete C;
}

void InputPanel::OnMouseLeftUp(wxMouseEvent& mouse)
{
    // if activeContour > -1, then a contour is selected for editing
    // and it equals the index of the contour.
    if (activeContour > -1)
    {
        // If the contour is closed, finalize it by deselecting it
        if (drawnContours[activeContour]->IsClosed())
        {
            drawnContours[activeContour]->Finalize();
            activeContour = -1;
            highlightedContour = -1;
            highlightedCtrlPoint = -1;
        }
        // If not, this contour must have multiple control points,
        // so move on to the next one
        else
        {
            drawnContours[activeContour]->
                AddPoint(ScreenToComplex(mouse.GetPosition()));
            highlightedCtrlPoint++;
        }
        Refresh();
        Update();
    }
    else
    {
        // If a contour is not highlighted (< 0), then create a new one
        // And set to be the active contour.
        if (highlightedContour < 0)
        {
            drawnContours.push_back(CreateContour(mouse.GetPosition()));
            activeContour = drawnContours.size() - 1;
            highlightedContour = activeContour;
        }
        // If not, then make the highlighted contour active.
        else
            activeContour = highlightedContour;
    }
}

void InputPanel::OnMouseRightUp(wxMouseEvent& mouse)
{
    // Right click will delete the active contour
    if (activeContour > -1)
    {
        delete drawnContours[activeContour];
        drawnContours.erase(drawnContours.begin() + activeContour);
        activeContour = -1;
        highlightedContour = -1;
        highlightedCtrlPoint = -1;
        Refresh();
        Update();
    }
    else
    {
        // On right click, delete the highlighted contour, if there is one.
        if (highlightedContour > -1)
        {
            delete drawnContours[highlightedContour];
            drawnContours.erase(drawnContours.begin() + highlightedContour);
            highlightedContour = -1;
            highlightedCtrlPoint = -1;
            Refresh();
            Update();
        }
    }
}

void InputPanel::OnMouseMoving(wxMouseEvent& mouse)
{
    // When the mouse moves, take contour-dependent action
    // depending on whether a control point is select or just part of
    // the contour. The control-point action should move the control
    // point. The non-control-point action is generally expected to
    // translate the contour, but it adjusts the radius of a circle.
    if (activeContour > -1)
    {
        if (highlightedCtrlPoint < 0)
        {
            drawnContours[activeContour]->ActionNoCtrlPoint(
                ScreenToComplex(mouse.GetPosition()), lastMousePos);
        }
        else
        {
            drawnContours[activeContour]->
                moveCtrlPoint(ScreenToComplex(mouse.GetPosition()),
                    highlightedCtrlPoint);
        }
        lastMousePos = ScreenToComplex(mouse.GetPosition());
        Refresh();
        Update();
    }
    // When the mouse moves, recheck for highlighted contours
        // and control points (and automatically highlight the contour).
    else
    {
        bool notOnAnyContour = true;
        int lastHC = highlightedContour;
        int lastHCP = highlightedCtrlPoint;

        for (int i = 0; i < drawnContours.size(); i++)
        {
            int CtrlPtIndex = drawnContours[i]->
                OnCtrlPoint(ScreenToComplex(mouse.GetPosition()), this);
            if (CtrlPtIndex > -1)
            {
                notOnAnyContour = false;
                highlightedCtrlPoint = CtrlPtIndex;
                highlightedContour = i;
            }
            else if (drawnContours[i]->
                IsOnContour(ScreenToComplex(mouse.GetPosition()), this))
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
        lastMousePos = ScreenToComplex(mouse.GetPosition());
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

    for (auto C : Output->mappedContours)
        delete C;
    Output->mappedContours.clear();
    Output->highlightedContour = highlightedContour;
    for (auto C : drawnContours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this, axes);
        Map(C, f); // Wrong place for this from efficiency perspective.
        // Only the altered contour needs its transformed version updated.
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
        drawnContours[highlightedContour]->Draw(&dc, this, axes);
    }
    Output->Refresh();
    Output->Update();
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
    }
    return new ContourCircle(ScreenToComplex(mousePos));
}

void InputPanel::Map(Contour* C,
    std::function<std::complex<double>(std::complex<double>)> f)
{
    // Approximate the mapped contour by interpolating points
    // of the input and transforming those.
    //ContourPolygon* D = new ContourPolygon();
    ContourPolygon* D = C->Subdivide(resolution);
    D->Transform(f);
    Output->mappedContours.push_back(D);
}