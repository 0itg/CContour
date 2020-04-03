#include "OutputPlane.h"
#include "InputPlane.h"

#include <wx/dcgraph.h>
#include <wx/richtooltip.h>

BOOST_CLASS_EXPORT_GUID(OutputPlane, "OutputPlane")

// clang-format off
wxBEGIN_EVENT_TABLE(OutputPlane, wxPanel)
EVT_LEFT_UP(OutputPlane::OnMouseLeftUp)
EVT_LEFT_DOWN(ComplexPlane::OnMouseLeftDown)
EVT_RIGHT_UP(ComplexPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
EVT_MOUSEWHEEL(OutputPlane::OnMouseWheel)
EVT_MOTION(OutputPlane::OnMouseMoving)
EVT_PAINT(OutputPlane::OnPaint)
EVT_LEAVE_WINDOW(ComplexPlane::OnMouseLeaving)
EVT_MOUSE_CAPTURE_LOST(ComplexPlane::OnMouseCapLost)
wxEND_EVENT_TABLE();
// clang-format on

OutputPlane::OutputPlane(wxWindow* parent, InputPlane* In, const std::string& n)
    : ComplexPlane(parent, n), in(In), tGrid(this)
{
    f = parser.Parse("z*z");
    In->AddOutputPlane(this);
    cullLargeSegments = true;
};

void OutputPlane::OnMouseLeftUp(wxMouseEvent& mouse)
{
    if (panning) state = STATE_IDLE;
}

void OutputPlane::OnMouseMoving(wxMouseEvent& mouse)
{
    cplx outCoord           = (ScreenToComplex(mouse.GetPosition()));
    std::string inputCoord  = "f(z) = " + f.str();
    std::string outputCoord = "f(z) = " + std::to_string(outCoord.real()) +
                              " + " + std::to_string(outCoord.imag()) + "i";
    statBar->SetStatusText(inputCoord, 0);
    statBar->SetStatusText(outputCoord, 1);

    if (panning)
    {
        Pan(mouse.GetPosition());
        Update();
        Refresh();
        toolPanel->Update();
        toolPanel->Refresh();
    }
    lastMousePos = ScreenToComplex(mouse.GetPosition());
    if (Highlight(mouse.GetPosition()))
    {
        Update();
        Refresh();
    }
    if (active > -1 && in->contours[active]->isPathOnly) active = -1;
    int temp   = in->active;
    in->active = active;
    if (temp != active)
    {
        in->Update();
        in->Refresh();
    }
}

void OutputPlane::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC pdc(this);
    wxGCDC dc(pdc);
    dc.Clear();
    dc.SetClippingRegion(GetClientSize());
    wxPen pen(tGrid.color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    // Only recalculate the mapping if the viewport changed.
    if (movedViewPort) { tGrid.MapGrid(in->grid, f); }

    if (showGrid) tGrid.Draw(&dc, this);

    auto& inputContours = in->contours;
    for (int i = 0; i < inputContours.size(); i++)
    {
        if (inputContours[i]->markedForRedraw)
        {
            contours[i] =
                std::unique_ptr<ContourPolygon>(inputContours[i]->Map(f));
            inputContours[i]->markedForRedraw = false;
        }
    }
    pen.SetWidth(2);

    size_t size = contours.size();
    for (int i = 0; i < size; i++)
    {
        auto& C = contours[i];
        if (!inputContours[i]->isPathOnly)
        {
            pen.SetColour(C->color);
            dc.SetPen(pen);
            C->Draw(&dc, this);
        }
    }
    if (active > -1)
    {
        auto& C = contours[active];
        if (!inputContours[active]->isPathOnly)
        {
            pen.SetColour(C->color);
            pen.SetWidth(3);
            dc.SetPen(pen);
            C->Draw(&dc, this);
        }
    }

    if (showAxes) axes.Draw(&dc);
    movedViewPort = false;
}

void OutputPlane::OnGridResCtrl(wxSpinEvent& event)
{
    tGrid.res = resCtrl->GetValue();
}

void OutputPlane::OnGridResCtrl(wxCommandEvent& event)
{
    tGrid.res     = resCtrl->GetValue();
    movedViewPort = true;
    Update();
    Refresh();
}

void OutputPlane::OnFunctionEntry(wxCommandEvent& event)
{
    EnterFunction(funcInput->GetLineText(0));
    in->GetAnimPanel()->UpdateComboBoxes();
}

void OutputPlane::OnMouseWheel(wxMouseEvent& event)
{
    history->RecordCommand(std::make_unique<CommandAxesSet>(this));
    ComplexPlane::OnMouseWheel(event);
    history->UpdateLastCommand();
}

// Override necessary to make sure animations play smoothly while
// interacting with the input plane;
void OutputPlane::Pan(wxPoint mousePos)
{
    ComplexPlane::Pan(mousePos);
    if (in->animating)
    {
        in->Update();
        in->Refresh();
    }
}

// Override necessary to make sure animations play smoothly while
// interacting with the input plane;
void OutputPlane::Zoom(wxPoint mousePos, int zoomSteps)
{
    ComplexPlane::Zoom(mousePos, zoomSteps);
    if (in->animating)
    {
        in->Update();
        in->Refresh();
    }
}

void OutputPlane::EnterFunction(std::string s)
{
    try
    {
        auto g = parser.Parse(s);
        g.eval();
        history->RecordCommand(
            std::make_unique<CommandOutputFuncEntry>(g, this));
        f = g;
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Function"), func.what());
        errormsg.ShowFor(funcInput);
    }
    movedViewPort = true;
    varPanel->Populate(f);
    varPanel->Update();
    varPanel->Refresh();
    MarkAllForRedraw();
    Update();
    Refresh();
}

void OutputPlane::CopyFunction(ParsedFunc<cplx> g)
{
    f             = g;
    movedViewPort = true;
    varPanel->Populate(f);
    varPanel->Update();
    varPanel->Refresh();
    MarkAllForRedraw();
    Update();
    Refresh();
}

void OutputPlane::MarkAllForRedraw()
{
    in->RecalcAll();
    auto& inputContours = in->contours;
    contours.resize(inputContours.size());
    for (int i = 0; i < contours.size(); i++)
    {
        contours[i] = std::unique_ptr<ContourPolygon>(inputContours[i]->Map(f));
    }
}

bool OutputPlane::DrawFrame(wxBitmap& image, double t)
{
    auto clientSize = GetClientSize();
    SetClientSize(image.GetSize());

    wxMemoryDC pdc(image);
    if (!pdc.IsOk()) return false;
    wxGCDC dc(pdc);
    dc.Clear();
    dc.SetClippingRegion(GetClientSize());
    wxPen pen(tGrid.color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    if (t >= 0)
        for (auto& A : in->animations)
            A->FrameAt(t * 1000);

    tGrid.MapGrid(in->grid, f);

    auto& inputContours = in->contours;
    for (int i = 0; i < inputContours.size(); i++)
    {
        if (!inputContours[i]->isPathOnly)
            contours[i] = std::unique_ptr<ContourPolygon>(inputContours[i]->Map(f));
    }

    if (showGrid) tGrid.Draw(&dc, this);
    pen.SetWidth(2);

    size_t size = contours.size();
    for (int i = 0; i < size; i++)
    {
        auto& C = contours[i];
        if (!inputContours[i]->isPathOnly)
        {
            pen.SetColour(C->color);
            dc.SetPen(pen);
            C->Draw(&dc, this);
        }
    }
    if (showAxes) axes.Draw(&dc);

    SetClientSize(clientSize);
    return true;
}

int OutputPlane::GetRes() { return tGrid.res; }
