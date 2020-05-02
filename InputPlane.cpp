#include "InputPlane.h"
#include "ContourCircle.h"
#include "ContourRect.h"
#include "ContourPoint.h"
#include "OutputPlane.h"

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
    wxDClickWorkaround()

    // if state > STATE_IDLE, then a contour is selected for editing
    // and state equals the index of the contour.
    if (state > STATE_IDLE)
    {
        // If the contour is closed, finalize and deselect it
        if (contours[state]->IsDone())
        {
            FinalizeContour(mouse);
        }
        // If not, user must be drawing a contour with multiple control points,
        // so move on to the next one
        else
        {
            contours[state]->AddPoint(ScreenToComplex(mouse.GetPosition()));
            toolPanel->PopulateContourTextCtrls(contours[state].get());
            activePt++;
        }
        Redraw();
    }
    else if (state == STATE_IDLE)
    {
        CaptureMouseIfAble();

        wxPoint pt = mouse.GetPosition();

        // If a contour is not highlighted (< 0), then create a new one
        // And set to be the active contour. If a contour is highlighted,
        // User can still create a new one with Ctrl-click
        if (active < 0 || mouse.ControlDown())
        {
            //cplx c;
            //bool snap = false;

            // Snap to control point if Ctrl key is down
            //if (activePt > 0 && mouse.ControlDown())
            //{
            //    snap = true;
            //    c    = contours[active]->GetCtrlPoint(activePt);
            //}
            contours.push_back(CreateContour(pt));
            history->RecordCommand(
                std::make_unique<CommandAddContour>(this, contours.back()));

            //if (snap) contours.back()->SetCtrlPoint(0, c);

            contours.back()->markedForRedraw = true;
            state  = contours.size() - 1;
            active = state;
            toolPanel->PopulateContourTextCtrls(contours[state].get());
            animPanel->UpdateComboBoxes();

            for (auto out : outputs)
            {
                out->active = state;
                // make space for transformed contour later on
                out->contours.push_back(nullptr);
            }

            if (contours.back()->IsDone()) FinalizeContour(mouse);
        }
        // If not, then make the highlighted contour active.
        else
        {
            auto mPos = ScreenToComplex(mouse.GetPosition());
            if (activePt > -1)
            {
                history->RecordCommand(std::make_unique<CommandContourSetPoint>(
                    contours[active].get(), mPos, activePt));
            }
            else
            {
                history->RecordCommand(std::unique_ptr<Command>(
                    contours[active]->CreateActionCommand(mPos)));
            }
            state = active;
            toolPanel->PopulateContourTextCtrls(contours[active].get());
        }
    }
    lastClickPos = ScreenToComplex(mouse.GetPosition());
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
        if (activePt < 0)
        {
            if (contours[state]->ActionNoCtrlPoint(mousePos, lastMousePos))
            {
                contours[state]->markedForRedraw = true;
                toolPanel->Update();
                toolPanel->Refresh();
                Redraw();
            }
            else
            {
                DeSelect();
                ReleaseMouseIfAble();
            }
        }
        else
        {
            contours[state]->SetCtrlPoint(activePt, mousePos);
            contours[state]->markedForRedraw = true;
            toolPanel->Update();
            toolPanel->Refresh();
            Redraw();
        }
    }
    // When the mouse moves, recheck for highlighted contours
    // and control points.
    else if (state == STATE_IDLE)
    {
        OnMouseMovingIdle(mouse);
    }

    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPlane::OnMouseLeftUpPaintbrush(wxMouseEvent& mouse)
{
    wxDClickWorkaround()

    if (active > -1)
    {
        history->RecordCommand(std::make_unique<CommandContourColorSet>(
            contours[active].get(), color));
        contours[active]->color           = color;
        contours[active]->markedForRedraw = true;
    }
    Redraw();
}

void InputPlane::OnMouseLeftUpSelectionTool(wxMouseEvent& mouse)
{
    wxDClickWorkaround()
    SelectionTool(mouse);
}

void InputPlane::OnMouseLeftUpRotationTool(wxMouseEvent& mouse)
{
    wxDClickWorkaround()

    auto v = [&] {
        return ScreenToComplex(mouse.GetPosition()) -
               contours[state]->GetCenter();
    };

    if (state == STATE_IDLE)
    {
        SelectionTool(mouse);
        if (state > STATE_IDLE && contours[active]->RotationEnabled())
        {
            if (state > -1)
                history->RecordCommand(std::make_unique<CommandContourRotate>(
                    contours[active].get(), v()));
        }
        else
        {
            DeSelect();
        }
    }
    else
    {
        history->UpdateLastCommand(v());
        DeSelect();
    }
}

void InputPlane::OnMouseMovingRotationTool(wxMouseEvent& mouse)
{
    if (state > STATE_IDLE)
    {
        contours[state]->Rotate((ScreenToComplex(mouse.GetPosition()) -
                                 contours[state]->GetCenter()) /
                                (lastMousePos - contours[state]->GetCenter()));
        contours[state]->markedForRedraw = true;
        toolPanel->Update();
        toolPanel->Refresh();
        for (auto out : outputs)
        {
            out->movedViewPort = true;
        }
        Redraw();
    }
    else
        OnMouseMovingIdle(mouse);

    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPlane::OnMouseLeftUpScaleTool(wxMouseEvent& mouse)
{
    wxDClickWorkaround()
    auto factor = [&] {
        return ScreenToComplex(mouse.GetPosition()) -
               contours[state]->GetCenter();
    };

    if (state == STATE_IDLE)
    {
        SelectionTool(mouse);
        if (state > STATE_IDLE && contours[state]->ScalingEnabled())
        {
            if (active > -1)
                history->RecordCommand(std::make_unique<CommandContourScale>(
                    contours[active].get(), factor()));
        }
        else
        {
            DeSelect();
        }
    }
    else
    {
        history->UpdateLastCommand(factor());
        DeSelect();
    }
}

void InputPlane::OnMouseMovingScaleTool(wxMouseEvent& mouse)
{
    if (state > STATE_IDLE)
    {
        contours[state]->Scale(
            abs((ScreenToComplex(mouse.GetPosition()) -
                 contours[state]->GetCenter()) /
                (lastMousePos - contours[state]->GetCenter())));
        contours[state]->markedForRedraw = true;
        toolPanel->Update();
        toolPanel->Refresh();
        for (auto out : outputs)
        {
            out->movedViewPort = true;
        }
        Redraw();
    }
    else
        OnMouseMovingIdle(mouse);

    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPlane::OnMouseRightUp(wxMouseEvent& mouse)
{
    ReleaseMouseIfAble();
    if (state > STATE_IDLE && activePt > -1)
    {
        // Remove the selected point if the contour is done. If it is still
        // being edited, the selected point is temporary, so remove the
        // previous one.
        int nextState = state;
        if (contours[state]->IsDone())
        {
            contours[state]->RemovePoint(activePt);
            toolPanel->PopulateContourTextCtrls(contours[state].get());
            activePt  = -1;
            active    = -1;
            nextState = STATE_IDLE;
        }
        else
        {
            contours[state]->RemovePoint(activePt - 1);
            activePt--;
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
    }
    ComplexPlane::OnMouseRightUp(mouse);
    for (auto out : outputs)
        out->CalcZerosAndPoles();
    Redraw();
}

void InputPlane::OnMouseWheel(wxMouseEvent& mouse)
{
    if (state == STATE_IDLE)
    {
        history->RecordCommand(
            std::make_unique<CommandAxesSet>(this, &this->grid));
        ComplexPlane::OnMouseWheel(mouse); // Calls the Zoom function.
        history->UpdateLastCommand();
        if (linkGridToAxes)
        {
            grid.hStep = axes.reStep;
            grid.vStep = axes.imStep;
        }
        for (auto out : outputs)
        {
            out->movedViewPort = true;
            out->CalcZerosAndPoles();
        }
        grid.CalcVisibleGrid();
        Redraw();
    }
}

void InputPlane::OnKeyUp(wxKeyEvent& Key)
{
    switch (Key.GetKeyCode())
    {
    case WXK_ESCAPE:
    case WXK_DELETE:
        toolPanel->PopulateAxisTextCtrls();
        toolPanel->Update();
        toolPanel->Refresh();
        ReleaseMouseIfAble(); // Captured by OnMouseRightDown
        if (active > -1)
        {
            if (contours[active]->IsDone())
            {
                if (state > STATE_IDLE)
                    history->UpdateLastCommand(lastMousePos);
                history->RecordCommand(
                    std::make_unique<CommandRemoveContour>(this, active));
            }
            else
            {
                history->PopCommand();
            }
            RemoveContour(active);
            state  = STATE_IDLE;
            active = -1;
            Redraw();
            animPanel->UpdateComboBoxes();
        }
        break;
    }
}

void InputPlane::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC pdc(this);
    wxGCDC dc(pdc);
    dc.Clear();
    dc.SetClippingRegion(GetClientSize());
    wxPen pen(grid.color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    if (animating)
    {
        for (auto& A : animations)
        {
            A->FrameAt(animTimer.Time());
            if (A->animateGrid) animateGrid = true;
        }
        if (showZeros)
            for (auto out : outputs) out->CalcZerosAndPoles();
    }
    if (animateGrid)
    {
        for (auto out : outputs)
        {
            out->MarkAllForRedraw();
            out->movedViewPort = true;
        }

        animateGrid = false;
    }

    if (showGrid) grid.Draw(&dc, this);

    if (showZeros)
    {
        pen.SetWidth(2);
        pen.SetColour(*wxBLACK);
        dc.SetPen(pen);
        for (auto out : outputs)
        {
            for (auto& P : out->zerosAndPoles)
                P->Draw(&dc, this);
            if (drawTooltip && mouseOnZero)
                mouseOnZero->DrawLabel(&dc, this);
        }
    }

    for (auto& C : contours)
    {
        if (!C->isPathOnly)
        {
            pen.SetWidth(2);
            pen.SetStyle(wxPENSTYLE_SOLID);
        }
        else
        {
            pen.SetWidth(1);
            pen.SetStyle(wxPENSTYLE_LONG_DASH);
        }
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }

    // Redraw the highlighted contour with thicker pen.
    // If a control point is highlighted, circle it with thin pen first.
    if (active > -1)
    {
        auto& C = contours[active];
        pen.SetColour(C->color);
        pen.SetStyle(wxPENSTYLE_SOLID);
        dc.SetPen(pen);

        if (activePt > -1)
        {
            dc.DrawCircle(ComplexToScreen(C->GetCtrlPoint(activePt)),
                          CIRCLED_POINT_RADIUS);
        }

        if (C->isPathOnly) pen.SetStyle(wxPENSTYLE_LONG_DASH);

        pen.SetWidth(3);
        dc.SetPen(pen);
        contours[active]->Draw(&dc, this);
    }
    if (showAxes) axes.Draw(&dc);
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
    Redraw();
}

void InputPlane::OnMouseMovingIdle(wxMouseEvent& mouse)
{
    bool callRedraw = false;
    if (Highlight(mouse.GetPosition())) callRedraw = true;

    for (auto out : outputs)
    {
        out->active = active;

        // Draws tooltip showing order and location of any zero/pole under
        // the cursor, or closes the last tooltip if none.
        drawTooltip = false;
        ContourPoint* mouseOnZeroTemp = nullptr;
        for (auto& Z : out->zerosAndPoles)
        {
            if (Z->IsPointOnContour(
                ScreenToComplex(mouse.GetPosition()), this))
            {
                mouseOnZeroTemp = Z.get();
                callRedraw = true;
            }
        }
        // wxWidgets' tooltip bounding rectangle doesn't work correctly,
        // but we can close the tooltip manually.
        if (!mouseOnZeroTemp && tooltip)
        {
            tooltip->Close();
        }
        else if (mouseOnZero != mouseOnZeroTemp)
                drawTooltip = true;
        mouseOnZero = mouseOnZeroTemp;
    }
    if (panning)
    {
        Pan(mouse.GetPosition());
        grid.CalcVisibleGrid();
        for (auto out : outputs)
        {
            out->movedViewPort = true;
        }
        callRedraw = true;
        toolPanel->Update();
        toolPanel->Refresh();
    }
    if (callRedraw) Redraw();
    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPlane::DeSelect()
{
    state    = -1;
    active   = -1;
    activePt = -1;
}

void InputPlane::SelectionTool(wxMouseEvent& mouse)
{
    wxPoint mousePos(mouse.GetPosition());
    int i = 0;

    for (i = 0; i < contours.size(); i++)
    {
        if (contours[i]->IsPointOnContour(ScreenToComplex(mousePos), this))
        {
            active = i;
            state  = i;
            break;
        }
    }
    if (i == contours.size())
    {
        DeSelect();
        toolPanel->PopulateAxisTextCtrls();
    }
    else
    {
        toolPanel->PopulateContourTextCtrls(contours[i].get());
    }
}

void InputPlane::RecalcAll()
{
    for (auto& C : contours)
    {
        C->markedForRedraw = true;
    }
}

void InputPlane::Redraw()
{
    Update();
    Refresh();
    for (auto out : outputs)
    {
        out->Update();
        out->Refresh();
    }
}

void InputPlane::SetContourType(int id) { contourType = id; }

void InputPlane::RemoveContour(int index)
{
    contours.erase(contours.begin() + index);

    for (auto out : outputs)
    {
        out->contours.erase(out->contours.begin() + index);
        out->active   = -1;
        out->activePt = -1;
    }
    active   = -1;
    activePt = -1;
}

void InputPlane::FinalizeContour(wxMouseEvent& mouse)
{
    ReleaseMouseIfAble();
    contours[state]->Finalize();

    history->UpdateLastCommand(ScreenToComplex(mouse.GetPosition()));
    toolPanel->PopulateContourTextCtrls(contours[state].get());

    contours[state]->markedForRedraw = true;
    DeSelect();
}

std::shared_ptr<Contour> InputPlane::CreateContour(wxPoint mousePos)
{
    wxColor colorToDraw = color;
    if (randomizeColor)
    {
        color = RandomColor();
        if (colorPicker != nullptr) colorPicker->SetColour(color);
    }

    switch (contourType)
    {
    case ID_Circle:
        CircleCount++;
        return std::make_shared<ContourCircle>(
            ScreenToComplex(mousePos), 0, colorToDraw,
            "Circle " + std::to_string(CircleCount));
        break;
    case ID_Rect:
        RectCount++;
        activePt = 1;
        return std::make_shared<ContourRect>(
            ScreenToComplex(mousePos), colorToDraw,
            "Rectangle " + std::to_string(RectCount));
        break;
    case ID_Polygon:
        PolygonCount++;
        activePt = 1;
        return std::make_shared<ContourPolygon>(
            ScreenToComplex(mousePos), colorToDraw,
            "Polygon " + std::to_string(PolygonCount));
        break;
    case ID_Line:
        LineCount++;
        activePt = 1;
        return std::make_shared<ContourLine>(
            ScreenToComplex(mousePos), colorToDraw,
            "Line " + std::to_string(LineCount));
        break;
    case ID_Point:
        PointCount++;
        activePt = 1;
        return std::make_shared<ContourPoint>(
            ScreenToComplex(mousePos), colorToDraw,
            "Point " + std::to_string(PointCount));
        break;
    }
    // Default in case we get a bad ID somehow
    CircleCount++;
    return std::make_shared<ContourCircle>(
        ScreenToComplex(mousePos), 0, colorToDraw,
        "Circle " + std::to_string(CircleCount));
}

void InputPlane::AddContour(std::shared_ptr<Contour> C)
{
    contours.push_back(C);
    for (auto out : outputs)
        out->contours.push_back(nullptr);
}

void InputPlane::InsertContour(std::shared_ptr<Contour> C, size_t i)
{
    contours.insert(contours.begin() + i, C);
    for (auto out : outputs)
        out->contours.insert(out->contours.begin() + i, nullptr);
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

void InputPlane::AddOutputPlane(OutputPlane* out) { outputs.push_back(out); }

bool InputPlane::DrawFrame(wxBitmap& image, double t)
{

    // A bit of a hack, because drawing code uses the panel's client size.
    // Temporary sets the inputplane's client size to match the output image.
    // Can't use the dc's GetSize() in drawing code because of wxWidgets issues.
    // A better solution would be to directly pass client size info to the
    // drawing functions.
    auto clientSize = GetClientSize();
    SetClientSize(image.GetSize());

    wxMemoryDC pdc(image);
    if (!pdc.IsOk()) return false;
    wxGCDC dc(pdc);
    dc.Clear();
    dc.SetClippingRegion(GetClientSize());
    wxPen pen(grid.color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    if (t >= 0)
        for (auto& A : animations)
            A->FrameAt(t * 1000);

    if (showGrid) grid.Draw(&dc, this);

    if (showZeros)
    {
        pen.SetWidth(2);
        pen.SetColour(*wxBLACK);
        dc.SetPen(pen);
        for (auto out : outputs)
        {
            out->CalcZerosAndPoles();
            for (auto& P : out->zerosAndPoles)
            {
                P->Draw(&dc, this);
            }
        }
    }
    pen.SetWidth(2);

    for (auto& C : contours)
    {
        pen.SetColour(C->color);
        dc.SetPen(pen);
        C->Draw(&dc, this);
    }
    if (showAxes) axes.Draw(&dc);

    SetClientSize(clientSize);
    return true;
}

double InputPlane::GetLongestAnimDur()
{
    double dur = 0;
    for (auto& A : animations)
    {
        dur = std::max(dur, A->duration_ms / 1000.0);
    }
    return dur;
}

ParsedFunc<cplx>* InputPlane::GetFunction(size_t i) { return &outputs[i]->f; }

void InputPlane::OnShowAxes_Grid_Zeros(wxCommandEvent& event)
{
    if (event.GetId() == ID_Show_Zeros)
    {
        showZeros = !showZeros;
        for (auto out : outputs) out->CalcZerosAndPoles();
    }
    ComplexPlane::OnShowAxes_Grid_Zeros(event);
}
