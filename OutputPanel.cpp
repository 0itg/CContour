#include "OutputPanel.h"
#include "InputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "wxMemDbg.h"

wxBEGIN_EVENT_TABLE(OutputPanel, wxPanel)
//EVT_MOTION(OutputPanel::OnPaint)
EVT_PAINT(OutputPanel::OnPaint)
wxEND_EVENT_TABLE()

OutputPanel::OutputPanel(wxWindow* parent, InputPanel* In) :
    ComplexPlane(parent), in(In), axes(this) {
    In->outputs.push_back(this);
};

void OutputPanel::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(*wxRED, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    std::vector<Contour*> mappedContours;

    for (auto C : in->subDivContours)
    {
        mappedContours.emplace_back(C->Apply(f));
    }
    for (auto C : mappedContours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this, axes);
    }
    if (in->highlightedContour > -1)
    {
        pen.SetColour(mappedContours[in->highlightedContour]->color);
        pen.SetWidth(2);
        dc.SetPen(pen);
        mappedContours[in->highlightedContour]->Draw(&dc, this, axes);
    }
    axes.Draw(&dc);
    for (auto C : mappedContours) delete C;
}