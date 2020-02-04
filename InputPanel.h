#pragma once
#include "ComplexPlane.h"

class ContourPolygon;

class InputPanel : public ComplexPlane
{
public:
    InputPanel(wxWindow* parent, OutputPanel* OP)
        : ComplexPlane(parent), Output(OP) {}
    ~InputPanel();

    void OnMouseLeftUp(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);
    void SetContourStyle(int id);
    void Map(Contour* C,
        std::function<std::complex<double>(std::complex<double>)> f);

    std::function<std::complex<double>(std::complex<double>)> f =
        [](std::complex<double> z) {return z * z; };
private:
    Contour* CreateContour(wxPoint mousePos);

    std::vector<Contour*> drawnContours;
    //std::vector<Contour*> subDivContours;
    int contourStyle = ID_Circle;
    std::complex<double> lastMousePos;
    OutputPanel* Output;
    wxDECLARE_EVENT_TABLE();
};