#pragma once
#include "Animation.h"
#include "ComplexPlane.h"
#include "Event_IDs.h"
#include "Grid.h"

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
class AnimPanel;

// Left Panel in UI. User draws on the panel, then the points are stored as
// complex numbers and mapped to the ouput panel under some complex function.
//
// This class has sort of evolved into the central class of the application,
// because most of what the user does is directly or indirectly manipulate the
// input contours.

class InputPlane : public ComplexPlane
{
    friend class OutputPlane;
    friend class boost::serialization::access;

  public:
    InputPlane() {}
    InputPlane(wxWindow* parent, const std::string& n = "Input")
        : ComplexPlane(parent, n), colorPicker(nullptr), grid(this)
    {
        grid.CalcVisibleGrid();
    }

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

    int GetState() const { return state; }
    int GetRes() const { return res; }
    void RecalcAll();

    // Calls refresh/update for this panel and output panel(s)
    void Redraw();

    // "Type" meaning Circle, Polygon, Rectangle, etc.
    void SetContourType(int id);
    void RemoveContour(int index);
    
    std::unique_ptr<Contour> CreateContour(wxPoint mousePos);
    
    void AddContour(std::unique_ptr<Contour> C);

    void CalcVisibleGrid() { grid.CalcVisibleGrid(); }

    void AddAnimation(std::unique_ptr<Animation> A)
    {
        animations.push_back(std::move(A));
    }

    void SetColorPicker(wxColourPickerCtrl* ptr) { colorPicker = ptr; };
    wxColor RandomColor();
    void AddOutputPlane(OutputPlane* out);
    void SetAnimPanel(AnimPanel* a) { animPanel = a; }

    // If true, when axes step values change, grid step values
    // change accordingly
    bool linkGridToAxes                  = true;
    bool randomizeColor                  = true;
    bool animating                       = false;
    wxColor color                        = wxColor(0, 0, 200);
    const wxColor BGcolor                = *wxWHITE;
    const int COLOR_SIMILARITY_THRESHOLD = 96;

    std::vector<std::unique_ptr<Animation>> animations;

    wxStopWatch animTimer;

  private:
    int CircleCount                = 0;
    int PolygonCount               = 0;
    int RectCount                  = 0;
    int LineCount                  = 0;
    const int CIRCLED_POINT_RADIUS = 7;
    int res                        = 500;
    Grid grid;

    // Pointers to outputs for or sending refresh signals.
    // App only uses one output for now, but more might be nice later.
    std::vector<OutputPlane*> outputs;

    wxColourPickerCtrl* colorPicker = nullptr;
    AnimPanel* animPanel            = nullptr;

    int contourType = ID_Circle;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<ComplexPlane>(*this);
        ar & linkGridToAxes;
        ar & res;
        resCtrl->SetValue(res);
        ar & grid;
        ar & animations;
    }
    wxDECLARE_EVENT_TABLE();
};