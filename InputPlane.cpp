#include "InputPlane.h"
#include "ContourCircle.h"
#include "ContourLine.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"
#include "OutputPlane.h"
#include "ToolPanel.h"

#include <wx/dcgraph.h>

BOOST_CLASS_EXPORT_GUID(InputPlane, "InputPlane")

// clang-format off
wxBEGIN_EVENT_TABLE(InputPlane, wxPanel)
EVT_LEFT_UP(InputPlane::OnMouseLeftUpContourTools)
EVT_LEFT_DOWN(ComplexPlane::OnMouseLeftDown)
EVT_RIGHT_UP(InputPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
//EVT_MIDDLE_DOWN(InputPanel::OnMouseMiddleDown)
//EVT_MIDDLE_UP(InputPanel::OnMouseMiddleUp)
EVT_MOUSEWHEEL(InputPlane::OnMouseWheel)
EVT_MOTION(InputPlane::OnMouseMoving)
EVT_KEY_UP(InputPlane::OnKeyUp)
EVT_PAINT(InputPlane::OnPaint)
EVT_LEAVE_WINDOW(ComplexPlane::OnMouseLeaving)
EVT_MOUSE_CAPTURE_LOST(ComplexPlane::OnMouseCapLost)
wxEND_EVENT_TABLE();
// clang-format on

void InputPlane::OnMouseLeftUpContourTools(wxMouseEvent& mouse)
{
    // Workaround for wxWidgets handling double clicks poorly in save dialog
    if (!mouseLeftDown)
        return;
    mouseLeftDown = false;
    SetFocus();

    // if state > STATE_IDLE, then a contour is selected for editing
    // and state equals the index of the contour.
    if (state > STATE_IDLE)
    {
        // If the contour is closed, finalize and deselect it
        if (contours[state]->IsDone())
        {
            ReleaseMouseIfAble();
            contours[state]->Finalize();

            toolPanel->PopulateContourTextCtrls(contours[state].get());
            // For now delete the whole subvidived contour and recalculate.
            // Later, could recalculate only the affected portion.
            contours[state]->Subdivide(res);
            state                = STATE_IDLE;
            highlightedContour   = -1;
            highlightedCtrlPoint = -1;
        }
        // If not, user must be drawing a contour with multiple control points,
        // so move on to the next one
        else
        {
            contours[state]->AddPoint(ScreenToComplex(mouse.GetPosition()));
            toolPanel->PopulateContourTextCtrls(contours[state].get());
            highlightedCtrlPoint++;
        }
        Refresh();
        Update();
    }
    else if (state == STATE_IDLE)
    {
        CaptureMouseIfAble();

        wxPoint pt = mouse.GetPosition();

        // If a contour is not highlighted (< 0), then create a new one
        // And set to be the active contour. If a contour is highlighted,
        // User can still create a new one with Ctrl-click
        if (highlightedContour < 0 || mouse.ControlDown())
        {
            cplx c;
            bool snap = false;

            // Snap to control point if Ctrl key is down
            if (highlightedCtrlPoint > 0 && mouse.ControlDown())
            {
                snap = true;
                c    = contours[highlightedContour]->GetCtrlPoint(
                    highlightedCtrlPoint);
            }
            contours.push_back(CreateContour(pt));

            if (snap)
                contours.back()->SetCtrlPoint(0, c);

            contours.back()->Subdivide(res);
            state              = contours.size() - 1;
            highlightedContour = state;
            toolPanel->PopulateContourTextCtrls(contours[state].get());
            animPanel->UpdateComboBoxes();

            for (auto out : outputs)
            {
                out->highlightedContour = state;
                // make space for transformed contour later on
                out->contours.push_back(nullptr);
            }
        }
        // If not, then make the highlighted contour active.
        else
        {
            state = highlightedContour;
            toolPanel->PopulateContourTextCtrls(contours[highlightedContour].get());
        }
    }
    else
    {
        state = STATE_IDLE;
        toolPanel->PopulateAxisTextCtrls();
    }
}

void InputPlane::OnMouseLeftUpPaintbrush(wxMouseEvent& mouse)
{
    // Workaround for wxWidgets handling double clicks poorly in save dialog
    if (!mouseLeftDown)
        return;
    mouseLeftDown = false;
   SetFocus();

    if (highlightedContour > -1)
    {
        contours[highlightedContour]->color           = color;
        contours[highlightedContour]->markedForRedraw = true;
    }
    Refresh();
    Update();
}

void InputPlane::OnMouseLeftUpSelectionTool(wxMouseEvent& mouse)
{
    // Workaround for wxWidgets handling double clicks poorly in save dialog
    if (!mouseLeftDown)
        return;
    mouseLeftDown = false;
    SetFocus();

    wxPoint mousePos(mouse.GetPosition());
    int i = 0;

    for (i = 0; i < contours.size(); i++)
    {
        if (contours[i]->IsPointOnContour(ScreenToComplex(mousePos), this))
        {
            highlightedContour = i;
            break;
        }
    }
    if (i == contours.size())
    {
        highlightedContour = -1;
        toolPanel->PopulateAxisTextCtrls();
    }
    else
    {
        toolPanel->PopulateContourTextCtrls(contours[i].get());
    }
}

void InputPlane::OnMouseRightUp(wxMouseEvent& mouse)
{
    ReleaseMouseIfAble();
    if (state > STATE_IDLE && highlightedCtrlPoint > -1)
    {
        // Remove the selected point if the contour is done. If it is still
        // being edited, the selected point is temporary, so remove the
        // previous one.
        int nextState = state;
        if (contours[state]->IsDone())
        {
            contours[state]->RemovePoint(highlightedCtrlPoint);
            toolPanel->PopulateContourTextCtrls(contours[state].get());
            highlightedCtrlPoint = -1;
            highlightedContour   = -1;
            nextState            = STATE_IDLE;
        }
        else
        {
            contours[state]->RemovePoint(highlightedCtrlPoint - 1);
            highlightedCtrlPoint--;
            toolPanel->PopulateContourTextCtrls(contours[state].get());
        }
        if (contours[state]->GetPointCount() < 2)
        {
            RemoveContour(state);
            nextState = STATE_IDLE;
            toolPanel->PopulateAxisTextCtrls();
            animPanel->UpdateComboBoxes();
        }
        state = nextState;
        Refresh();
        Update();
    }
    ComplexPlane::OnMouseRightUp(mouse);
}

void InputPlane::OnMouseWheel(wxMouseEvent& mouse)
{
    ComplexPlane::OnMouseWheel(mouse); // Calls the Zoom function.
    if (linkGridToAxes)
    {
        grid.hStep = axes.reStep;
        grid.vStep = axes.imStep;
    }
    for (auto out : outputs)
        out->movedViewPort = true;
}

void InputPlane::OnMouseMoving(wxMouseEvent& mouse)
{
    // NOTE: Status bar code will need to change for multiple output windows
    cplx mousePos          = ScreenToComplex(mouse.GetPosition());
    cplx outCoord          = outputs[0]->f(mousePos);
    std::string inputCoord = "z = " + std::to_string(mousePos.real()) + " + " +
                             std::to_string(mousePos.imag()) + "i";
    std::string outputCoord = "f(z) = " + std::to_string(outCoord.real()) +
                              " + " + std::to_string(outCoord.imag()) + "i";
    statBar->SetStatusText(inputCoord, 0);
    statBar->SetStatusText(outputCoord, 1);

    // When the mouse moves, take contour-dependent action
    // depending on whether a control point is select or just part of
    // the contour. The control-point action should move the control
    // point. The non-control-point action is generally expected to
    // translate the contour, but it adjusts the radius of a circle.
    if (state > STATE_IDLE)
    {
        CaptureMouseIfAble();
        if (highlightedCtrlPoint < 0)
        {
            if (contours[state]->ActionNoCtrlPoint(mousePos, lastMousePos))
            {
                contours[state]->Subdivide(res);
                toolPanel->Refresh();
                toolPanel->Update();
                Refresh();
                Update();
            }
            else
            {
                highlightedContour = -1;
                highlightedCtrlPoint = -1;
                state = STATE_IDLE;
                ReleaseMouseIfAble();
            }
        }
        else
        {
            contours[state]->SetCtrlPoint(highlightedCtrlPoint, mousePos);
            contours[state]->Subdivide(res);
            toolPanel->Refresh();
            toolPanel->Update();
            Refresh();
            Update();
        }
    }
    // When the mouse moves, recheck for highlighted contours
    // and control points (and automatically highlight the contour).
    else if (state == STATE_IDLE)
    {
        Highlight(mouse.GetPosition());
        for (auto out : outputs)
        {
            out->highlightedContour = highlightedContour;
            Refresh();
            Update();
        }
    }
    if (panning)
    {
        Pan(mouse.GetPosition());
        for (auto out : outputs)
        {
            out->movedViewPort = true;
        }
        toolPanel->Refresh();
        toolPanel->Update();
    }

    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPlane::OnKeyUp(wxKeyEvent& Key)
{
    switch (Key.GetKeyCode())
    {
    case WXK_ESCAPE:
    case WXK_DELETE:
        toolPanel->PopulateAxisTextCtrls();
        toolPanel->Refresh();
        toolPanel->Update();
        ReleaseMouseIfAble(); // Captured by OnMouseRightDown
        if (highlightedContour > -1)
        {
            RemoveContour(highlightedContour);
            state              = STATE_IDLE;
            highlightedContour = -1;
            Refresh();
            Update();
            animPanel->UpdateComboBoxes();
        }
        break;
    }
}

void InputPlane::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC pdc(this);
    wxGCDC dc(pdc);
    wxDCClipper(dc, GetClientSize());
    dc.Clear();
    wxPen pen(grid.color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    if (animating)
        for (auto& A : animations)
            A->FrameAt(animTimer.Time());

    if (showGrid)
        grid.Draw(&dc, this);
    pen.SetWidth(2);

    for (auto& C : contours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }

    // Redraw the highlighted contour with thicker pen.
    // If a control point is highlighted, circle it with thin pen first.
    if (highlightedContour > -1)
    {
        pen.SetColour(contours[highlightedContour]->color);
        dc.SetPen(pen);

        if (highlightedCtrlPoint > -1)
        {
            dc.DrawCircle(
                ComplexToScreen(contours[highlightedContour]->GetCtrlPoint(
                    highlightedCtrlPoint)),
                CIRCLED_POINT_RADIUS);
        }
        pen.SetWidth(3);
        dc.SetPen(pen);
        contours[highlightedContour]->Draw(&dc, this);
    }
    if (showAxes)
        axes.Draw(&dc);

    for (auto out : outputs)
    {
        out->Refresh();
        out->Update();
    }
}

void InputPlane::OnColorPicked(wxColourPickerEvent& colorPicked)
{
    color = colorPicked.GetColour();
}

void InputPlane::OnColorRandomizer(wxCommandEvent& event)
{
    randomizeColor = event.IsChecked();
}

void InputPlane::OnContourResCtrl(wxSpinEvent& event)
{
    res = resCtrl->GetValue();
}

void InputPlane::OnContourResCtrl(wxCommandEvent& event)
{
    res = resCtrl->GetValue();
    RecalcAll();
    Update();
    Refresh();
}

void InputPlane::RecalcAll()
{
    for (auto& C : contours)
    {
        C->Subdivide(res);
    }
}

void InputPlane::SetContourType(int id)
{
    contourType = id;
}

void InputPlane::RemoveContour(int index)
{
    contours.erase(contours.begin() + index);

    for (auto out : outputs)
    {
        out->contours.erase(out->contours.begin() + index);
        out->highlightedContour   = -1;
        out->highlightedCtrlPoint = -1;
    }
    highlightedContour   = -1;
    highlightedCtrlPoint = -1;
}

std::unique_ptr<Contour> InputPlane::CreateContour(wxPoint mousePos)
{
    wxColor colorToDraw = color;
    if (randomizeColor)
    {
        color = RandomColor();
        if (colorPicker != nullptr)
            colorPicker->SetColour(color);
    }

    switch (contourType)
    {
    case ID_Circle:
        CircleCount++;
        return std::make_unique<ContourCircle>(ScreenToComplex(mousePos), 0, colorToDraw,
            "Circle " + std::to_string(CircleCount));
        break;
    case ID_Rect:
        RectCount++;
        highlightedCtrlPoint = 1;
        return std::make_unique<ContourRect>(ScreenToComplex(mousePos), colorToDraw,
            "Rectangle " + std::to_string(RectCount));
        break;
    case ID_Polygon:
        PolygonCount++;
        highlightedCtrlPoint = 1;
        return std::make_unique<ContourPolygon>(ScreenToComplex(mousePos), colorToDraw,
            "Polygon " + std::to_string(PolygonCount));
        break;
    case ID_Line:
        LineCount++;
        highlightedCtrlPoint = 1;
        return std::make_unique<ContourLine>(ScreenToComplex(mousePos), colorToDraw,
            "Line " + std::to_string(LineCount));
        break;
    }
    // Default in case we get a bad ID somehow
    CircleCount++;
    return std::make_unique<ContourCircle>(ScreenToComplex(mousePos), 0, colorToDraw,
        "Circle " + std::to_string(CircleCount));
}

void InputPlane::AddContour(std::unique_ptr<Contour> C)
{
    contours.push_back(std::move(C));
    for (auto out : outputs)
        out->contours.push_back(nullptr);
}

wxColor InputPlane::RandomColor()
{
    // Crude color randomizer. Generates RGB values at random, then rerolls
    // until the color is not too similar to the current color or the BG color.
    auto dist = [](wxColor c1, wxColor c2) {
        int red   = c1.Red() - c2.Red();
        int green = c1.Green() - c2.Green();
        int blue  = c1.Blue() - c2.Blue();
        return sqrt(red * red + green * green + blue * blue);
    };

    wxColor C = wxColor(rand() % 255, rand() % 255, rand() % 255);

    while (dist(C, color) < COLOR_SIMILARITY_THRESHOLD ||
           dist(C, BGcolor) < COLOR_SIMILARITY_THRESHOLD)
        C = wxColor(rand() % 255, rand() % 255, rand() % 255);
    return C;
}
