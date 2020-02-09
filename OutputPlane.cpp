#include "OutputPlane.h"
#include "InputPlane.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"

wxBEGIN_EVENT_TABLE(OutputPlane, wxPanel)
EVT_LEFT_UP(OutputPlane::OnMouseLeftUp)
EVT_RIGHT_UP(ComplexPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
EVT_MOUSEWHEEL(ComplexPlane::OnMouseWheel)
EVT_MOTION(OutputPlane::OnMouseMoving)
EVT_PAINT(OutputPlane::OnPaint)
wxEND_EVENT_TABLE()

OutputPlane::OutputPlane(wxFrame* parent, InputPlane* In) :
    ComplexPlane(parent), in(In) {
    In->outputs.push_back(this);
    tGrid = new TransformedGrid(this);
};

OutputPlane::~OutputPlane()
{
    delete tGrid;
}

void OutputPlane::OnMouseLeftUp(wxMouseEvent& mouse)
{
    if (state == STATE_PANNING) state = STATE_IDLE;
}

void OutputPlane::OnMouseMoving(wxMouseEvent& mouse)
{
    std::complex<double> outCoord = f(ScreenToComplex(mouse.GetPosition()));
    std::string inputCoord = "f(z) = z^2"; // TEMPORARY
    std::string outputCoord = "f(z) = " + std::to_string(outCoord.real()) +
        " + " + std::to_string(outCoord.imag()) + "i";
    statBar->SetStatusText(inputCoord, 0);
    statBar->SetStatusText(outputCoord, 1);

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

void OutputPlane::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(tGrid->color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    // Only recalculate the mapping if the viewport changed.
    if (movedViewPort) tGrid->MapGrid(in->grid, f);

    tGrid->Draw(&dc, this);

    for (auto C : contours) delete C;
    contours.clear();
    for (auto C : in->subDivContours)
    {
        contours.push_back(C->Apply(f));
    }
    pen.SetWidth(2);

    for (auto C : contours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }
    if (highlightedContour > -1)
    {
        pen.SetColour(contours[highlightedContour]->color);
        pen.SetWidth(3);
        dc.SetPen(pen);
        contours[highlightedContour]->Draw(&dc, this);
    }

    axes.Draw(&dc);
    movedViewPort = false;
}