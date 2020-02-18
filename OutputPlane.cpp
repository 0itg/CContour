#include "OutputPlane.h"
#include "InputPlane.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"
#include "Token.h"
#include "Parser.h"

#include "Event_IDs.h"
//#include "wx/dcgraph.h"

wxBEGIN_EVENT_TABLE(OutputPlane, wxPanel)
EVT_LEFT_UP(OutputPlane::OnMouseLeftUp)
EVT_RIGHT_UP(ComplexPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
EVT_MOUSEWHEEL(ComplexPlane::OnMouseWheel)
EVT_MOTION(OutputPlane::OnMouseMoving)
EVT_PAINT(OutputPlane::OnPaint)
EVT_LEAVE_WINDOW(ComplexPlane::OnMouseLeaving)
EVT_MOUSE_CAPTURE_LOST(ComplexPlane::OnMouseCapLost)
wxEND_EVENT_TABLE()

OutputPlane::OutputPlane(wxFrame* parent, InputPlane* In) :
    ComplexPlane(parent), in(In) {
    f.Parse("z*z");
    In->outputs.push_back(this);
    tGrid = new TransformedGrid(this);
};

OutputPlane::~OutputPlane()
{
    delete tGrid;
}

void OutputPlane::OnMouseLeftUp(wxMouseEvent& mouse)
{
    if (panning) state = STATE_IDLE;
}

void OutputPlane::OnMouseMoving(wxMouseEvent& mouse)
{
    std::complex<double> outCoord = (ScreenToComplex(mouse.GetPosition()));
    std::string inputCoord = "f(z) = " + f.str();
    std::string outputCoord = "f(z) = " + std::to_string(outCoord.real()) +
        " + " + std::to_string(outCoord.imag()) + "i";
    statBar->SetStatusText(inputCoord, 0);
    statBar->SetStatusText(outputCoord, 1);

    if (panning) Pan(mouse.GetPosition());
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
    //wxGCDC dc(pdc);
    //wxDCClipper(dc, GetClientSize());
    dc.Clear();
    wxPen pen(tGrid->color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    //wxBrush brush(wxColor(255, 0, 0, 128), wxBRUSHSTYLE_SOLID);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    // Only recalculate the mapping if the viewport changed.
    if (movedViewPort) tGrid->MapGrid(in->grid, f);

    if (showGrid) tGrid->Draw(&dc, this);

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

    if (showAxes)axes.Draw(&dc);
    movedViewPort = false;
}

void OutputPlane::OnGridResCtrl(wxSpinEvent& event)
{
    tGrid->res = resCtrl->GetValue();
}

void OutputPlane::OnGridResCtrl(wxCommandEvent& event)
{
    tGrid->res = resCtrl->GetValue();
    movedViewPort = true;
    Refresh();
    Update();
}

void OutputPlane::OnFunctionEntry(wxCommandEvent& event)
{
    try
    {
        f.Parse(funcInput->GetLineText(0));
    }
    catch(std::invalid_argument& func)
    {
        f.Revert();
        wxMessageBox(func.what(), wxT("Invalid Function"), wxICON_INFORMATION);
    }
    movedViewPort = true;
    Refresh();
    Update();
}
