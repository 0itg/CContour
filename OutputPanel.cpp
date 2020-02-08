#include "OutputPanel.h"
#include "InputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"

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
    tGrid = new TransformedGrid(this);
};

OutputPanel::~OutputPanel()
{
    delete tGrid;
}

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
    wxPen pen(tGrid->color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    if (movedViewPort) tGrid->CalcVisibleGrid(in->grid, f);

    tGrid->Draw(&dc, this);

    for (auto C : contours) delete C;
    contours.clear();

    for (auto C : in->subDivContours)
    {
        contours.emplace_back(C->Apply(f));
    }

    for (auto C : contours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }
    if (highlightedContour > -1)
    {
        pen.SetColour(contours[highlightedContour]->color);
        pen.SetWidth(2);
        dc.SetPen(pen);
        contours[highlightedContour]->Draw(&dc, this);
    }

    axes.Draw(&dc);
    movedViewPort = false;
}