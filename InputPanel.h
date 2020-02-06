#pragma once
#include "ComplexPlane.h"

class ContourPolygon;
class OutputPanel;

// Left Panel in UI. User draws on the panel, then the points are stored as
// complex numbers and mapped to the ouput panel under some complex function. 

class InputPanel : public ComplexPlane
{
    friend class OutputPanel;
public:
    InputPanel(wxWindow* parent)
        : ComplexPlane(parent) {}
    ~InputPanel();

    void OnMouseLeftUp(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseRightDown(wxMouseEvent& mouse);
    //void OnMouseMiddleDown(wxMouseEvent& mouse);
    //void OnMouseMiddleUp(wxMouseEvent& mouse);
    void OnMouseWheel(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnKeyDelete(wxKeyEvent& Key);
    void OnPaint(wxPaintEvent& paint);
    void SetContourStyle(int id);
private:
    Contour* CreateContour(wxPoint mousePos);

    // drawnContours stores contours in original form for editing.
    // subDivContours stores them as approximating polygons for mapping 
    std::vector<ContourPolygon*> subDivContours;
    // Pointers to outputs for or sending refresh signals.
    // App only uses one output for now, but more might be nice later.
    std::vector<OutputPanel*> outputs; 
    int contourStyle = ID_Circle;
    wxDECLARE_EVENT_TABLE();
};