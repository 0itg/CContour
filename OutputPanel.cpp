#include "OutputPanel.h"
#include "ContourCircle.h"
#include "wxMemDbg.h"

wxBEGIN_EVENT_TABLE(OutputPanel, wxPanel)
//EVT_MOTION(OutputPanel::OnPaint)
EVT_PAINT(OutputPanel::OnPaint)
wxEND_EVENT_TABLE()

OutputPanel::~OutputPanel()
{
    for (auto C : mappedContours)
        delete C;
}

void OutputPanel::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(*wxRED, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    for (auto C : mappedContours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this, axes);
    }
    if (highlightedContour > -1)
    {
        pen.SetColour(mappedContours[highlightedContour]->color);
        pen.SetWidth(2);
        dc.SetPen(pen);
        mappedContours[highlightedContour]->Draw(&dc, this, axes);
    }
}