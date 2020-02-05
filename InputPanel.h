#pragma once
#include "ComplexPlane.h"

class ContourPolygon;
class OutputPanel;

class InputPanel : public ComplexPlane
{
    friend class OutputPanel;
public:
    InputPanel(wxWindow* parent/*, OutputPanel* OP*/)
        : ComplexPlane(parent)/*, Output(OP)*/ {}
    ~InputPanel();

    void OnMouseLeftUp(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);
    void SetContourStyle(int id);
    //void Map(Contour* C,
    //    std::function<std::complex<double>(std::complex<double>)> f);
private:
    Contour* CreateContour(wxPoint mousePos);

    // drawnContours stores contours in original form for editing.
    // subDivContours stores them as approximating polygons for mapping 
    std::vector<Contour*> drawnContours;
    std::vector<ContourPolygon*> subDivContours;
    // Pointers to outputs for or sending refresh signals.
    // App only uses one output for now, but more might be nice later.
    std::vector<OutputPanel*> outputs; 
    int contourStyle = ID_Circle;
    std::complex<double> lastMousePos;
    wxDECLARE_EVENT_TABLE();
};