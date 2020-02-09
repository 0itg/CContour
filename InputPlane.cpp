#include "OutputPlane.h"
#include "InputPlane.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "ContourLine.h"
#include "Grid.h"

wxBEGIN_EVENT_TABLE(InputPlane, wxPanel)
EVT_LEFT_UP(InputPlane::OnMouseLeftUpContourTools)
//EVT_LEFT_DOWN(InputPanel::OnMouseLeftDown)
EVT_RIGHT_UP(InputPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
//EVT_MIDDLE_DOWN(InputPanel::OnMouseMiddleDown)
//EVT_MIDDLE_UP(InputPanel::OnMouseMiddleUp)
EVT_MOUSEWHEEL(InputPlane::OnMouseWheel)
EVT_MOTION(InputPlane::OnMouseMoving)
EVT_KEY_UP(InputPlane::OnKeyUp)
EVT_PAINT(InputPlane::OnPaint)
wxEND_EVENT_TABLE()

InputPlane::~InputPlane()
{
    for (auto C : subDivContours)
        delete C;
    delete grid;
}

InputPlane::InputPlane(wxFrame* parent)
    : ComplexPlane(parent)
{
    grid = new Grid(this);
}

void InputPlane::OnMouseLeftUpContourTools(wxMouseEvent& mouse)
{
    // if state > STATE_IDLE, then a contour is selected for editing
    // and state equals the index of the contour.
    if (state > STATE_IDLE)
    {
        // If the contour is closed, finalize and deselect it
        if (contours[state]->IsDone())
        {
            ReleaseMouseIfAble();
            contours[state]->Finalize();
            // For now delete the whole subvidived contour and recalculate.
            // Later, could recalculate only the affected portion.
            delete subDivContours[state];
            subDivContours[state] = contours[state]->Subdivide(res);
            state = STATE_IDLE;
            highlightedContour = -1;
            highlightedCtrlPoint = -1;
        }
        // If not, user must be drawing a contour with multiple control points,
        // so move on to the next one
        else
        {
            contours[state]->
                AddPoint(ScreenToComplex(mouse.GetPosition()));
            highlightedCtrlPoint++;
        }
        Refresh();
        Update();
    }
    else if (state == STATE_IDLE)
    {
        CaptureMouseIfAble();
        // If a contour is not highlighted (< 0), then create a new one
        // And set to be the active contour.
        if (highlightedContour < 0)
        {
            contours.push_back(CreateContour(mouse.GetPosition()));
            subDivContours.push_back(contours.back()->Subdivide(res));
            state = contours.size() - 1;
            highlightedContour = state;
            for (auto out : outputs)
                out->highlightedContour = state;
        }
        // If not, then make the highlighted contour active.
        else
            state = highlightedContour;
    }
    else
    {
        state = STATE_IDLE;
    }
}

void InputPlane::OnMouseLeftUpPaintbrush(wxMouseEvent& mouse)
{
    if (highlightedContour > -1)
        contours[highlightedContour]->color = color;
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
            highlightedCtrlPoint = -1;
            highlightedContour = -1;
            nextState = STATE_IDLE;
        }
        else
        {
            contours[state]->RemovePoint(highlightedCtrlPoint - 1);
            highlightedCtrlPoint--;
        }
        if (contours[state]->PointCount() < 2)
        {
            RemoveContour(state);
            nextState = STATE_IDLE;
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
        grid->hStep = axes.reStep;
        grid->vStep = axes.imStep;
    }
    for (auto out : outputs)
        out->movedViewPort = true;
    Refresh();
    Update();
}

void InputPlane::OnMouseMoving(wxMouseEvent& mouse)
{
    // NOTE: Status bar code will need to change for multiple output windows
    std::complex<double> mousePos = ScreenToComplex(mouse.GetPosition());
    std::complex<double> outCoord = outputs[0]->f(mousePos);
    std::string inputCoord = "z = " + std::to_string(mousePos.real()) +
        " + " + std::to_string(mousePos.imag()) + "i";
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
        if (highlightedCtrlPoint < 0)
        {
            contours[state]->ActionNoCtrlPoint(mousePos, lastMousePos);
        }
        else
        {
            contours[state]->
                moveCtrlPoint(mousePos, highlightedCtrlPoint);
        }
        delete subDivContours[state];
        subDivContours[state] =
            std::move(contours[state]->Subdivide(res));
        Refresh();
        Update();
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
    else if (state == STATE_PANNING)
    {
        Pan(mouse.GetPosition());
    }

    lastMousePos = ScreenToComplex(mouse.GetPosition());
}

void InputPlane::OnKeyUp(wxKeyEvent& Key)
{
    switch (Key.GetKeyCode())
    {
    case WXK_DELETE:
        ReleaseMouseIfAble(); // Captured by OnMouseRightDown
        if (highlightedContour > -1)
        {
            RemoveContour(highlightedContour);
            state = STATE_IDLE;
            Refresh();
            Update();
        }
        break;
    }
}

void InputPlane::OnPaint(wxPaintEvent& paint)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxPen pen(grid->color, 1);
    wxBrush brush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(pen);
    dc.SetBrush(brush);

    grid->Draw(&dc, this);
    pen.SetWidth(2);

    for (auto C : contours)
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
            dc.DrawCircle(ComplexToScreen(contours[highlightedContour]->
                GetCtrlPoint(highlightedCtrlPoint)), 7);
        }
        pen.SetWidth(3);
        dc.SetPen(pen);
        contours[highlightedContour]->Draw(&dc, this);
    }
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

void InputPlane::SetContourType(int id)
{
    contourType = id;
}

void InputPlane::RemoveContour(int index)
{
    delete contours[index];
    contours.erase(contours.begin() + index);
    delete subDivContours[index];
    subDivContours.erase(subDivContours.begin() + index);

    for (auto out : outputs)
    {
        delete out->contours[index];
        out->contours.erase(out->contours.begin() + index);
        out->highlightedContour = -1;
        out->highlightedCtrlPoint = -1;
    }
    highlightedContour = -1;
    highlightedCtrlPoint = -1;
}

Contour* InputPlane::CreateContour(wxPoint mousePos)
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
        return new ContourCircle(ScreenToComplex(mousePos), 0, colorToDraw);
        break;
    case ID_Rect:
        highlightedCtrlPoint = 1;
        return new ContourRect(ScreenToComplex(mousePos), colorToDraw);
        break;
    case ID_Polygon:
        highlightedCtrlPoint = 1;
        return new ContourPolygon(ScreenToComplex(mousePos), colorToDraw);
        break;
    case ID_Line:
        highlightedCtrlPoint = 1;
        return new ContourLine(ScreenToComplex(mousePos), colorToDraw);
        break;
    }
    // Default in case we get a bad ID somehow
    return new ContourCircle(ScreenToComplex(mousePos), 0, colorToDraw);
}

wxColor InputPlane::RandomColor()
{
    // Crude color randomizer. Generates RGB values at random, then rerolls
    // until the color is not too similar to the current color or the BG color.
    auto dist = [](wxColor c1, wxColor c2)
    {
        int red = c1.Red() - c2.Red();
        int green = c1.Green() - c2.Green();
        int blue = c1.Blue() - c2.Blue();
        return sqrt(red * red + green * green + blue * blue);
    };

    wxColor C = wxColor(rand() % 255, rand() % 255, rand() % 255);

    while (dist(C, color) < COLOR_SIMILARITY_THRESHOLD ||
           dist(C, BGcolor) < COLOR_SIMILARITY_THRESHOLD)
        C = wxColor(rand() % 255, rand() % 255, rand() % 255);
    return C;
}
