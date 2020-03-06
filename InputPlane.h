#pragma once
#include "ComplexPlane.h"
#include "Event_IDs.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <wx/clrpicker.h>

typedef std::complex<double> cplx;

class ContourPolygon;
class OutputPlane;
class Grid;

// Left Panel in UI. User draws on the panel, then the points are stored as
// complex numbers and mapped to the ouput panel under some complex function.

class InputPlane : public ComplexPlane
{
    friend class OutputPlane;
    friend class boost::serialization::access;

  public:
    InputPlane() {}
    InputPlane(wxWindow* parent, const std::string& name = "input");
    ~InputPlane();

    void OnMouseLeftUpContourTools(wxMouseEvent& mouse);
    void OnMouseLeftUpPaintbrush(wxMouseEvent& mouse);
    void OnMouseLeftUpSelectionTool(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    // void OnMouseRightDown(wxMouseEvent& mouse);
    // void OnMouseMiddleDown(wxMouseEvent& mouse);
    // void OnMouseMiddleUp(wxMouseEvent& mouse);
    void OnMouseWheel(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnKeyUp(wxKeyEvent& Key);
    void OnPaint(wxPaintEvent& paint);
    void OnColorPicked(wxColourPickerEvent& colorPicked);
    void OnColorRandomizer(wxCommandEvent& event);
    void OnContourResCtrl(wxSpinEvent& event);
    void OnContourResCtrl(wxCommandEvent& event);

    int GetState()
    {
        return state;
    }
    int GetRes()
    {
        return res;
    }
    void RecalcAll();
    void ClearSubDivs();

    // "Type" meaning Circle, Polygon, Rectangle, etc.
    void SetContourType(int id);
    void RemoveContour(int index);
    Contour* CreateContour(wxPoint mousePos);

    void SetColorPicker(wxColourPickerCtrl* ptr)
    {
        colorPicker = ptr;
    };
    wxColor RandomColor();

    void PrepareForLoadFromFile();

    // If true, when axes step values change, grid step values
    // change accordingly
    bool linkGridToAxes                  = true;
    bool randomizeColor                  = true;
    wxColor color                        = wxColor(0, 0, 200);
    const wxColor BGcolor                = *wxWHITE;
    const int COLOR_SIMILARITY_THRESHOLD = 96;

    Grid* grid;

  private:
    const int CIRCLED_POINT_RADIUS = 7;
    int res                        = 200;
    // drawnContours stores contours in original form for editing.
    // subDivContours stores them as approximating polygons for mapping
    std::vector<ContourPolygon*> subDivContours;
    // Pointers to outputs for or sending refresh signals.
    // App only uses one output for now, but more might be nice later.
    std::vector<OutputPlane*> outputs;
    wxColourPickerCtrl* colorPicker = nullptr;

    int contourType = ID_Circle;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<ComplexPlane>(*this);
        ar& linkGridToAxes;
        ar& res;
        ar& grid;
    }
    wxDECLARE_EVENT_TABLE();
};