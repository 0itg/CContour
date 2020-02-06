#include "OutputPanel.h"
#include "InputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "wxMemDbg.h"

wxBEGIN_EVENT_TABLE(OutputPanel, wxPanel)
EVT_LEFT_UP(OutputPanel::OnMouseLeftUp)
EVT_RIGHT_UP(ComplexPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
EVT_MOUSEWHEEL(ComplexPlane::OnMouseWheel)
EVT_MOTION(OutputPanel::OnMouseMoving)
EVT_PAINT(OutputPanel::OnPaint)
wxEND_EVENT_TABLE()

OutputPanel::OutputPanel(wxWindow* parent, InputPanel* In) :
    ComplexPlane(parent), in(In) {
    In->outputs.push_back(this);
};

void OutputPanel::OnMouseLeftUp(wxMouseEvent& mouse)
{
    if (state == STATE_PANNING) state = STATE_IDLE;
}

void OutputPanel::OnMouseMoving(wxMouseEvent& mouse)
{
    if (state == STATE_PANNING) Pan(mouse.GetPosition());
    lastMousePos = ScreenToComplex(mouse.GetPosition());
    Highlight(mouse.GetPosition());
    int temp = in->highlightedContour;
    in->highlightedContour = highlightedContour;
    if (temp != highlightedContour)
    {
        in->Refresh();
        in->Update();
    }
}

void OutputPanel::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(*wxRED, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    for (auto C : drawnContours) delete C;
    drawnContours.clear();

    for (auto C : in->subDivContours)
    {
        drawnContours.emplace_back(C->Apply(f));
    }

    for (auto C : drawnContours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }
    if (highlightedContour > -1)
    {
        pen.SetColour(drawnContours[highlightedContour]->color);
        pen.SetWidth(2);
        dc.SetPen(pen);
        drawnContours[highlightedContour]->Draw(&dc, this);
    }

    axes.Draw(&dc);
}