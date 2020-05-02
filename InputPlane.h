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
#include <wx/tipwin.h>

typedef std::complex<double> cplx;

class ContourPolygon;
class ContourPoint;
class OutputPlane;
class AnimPanel;
template <class T> class ParsedFunc;

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
    void OnMouseLeftUpRotationTool(wxMouseEvent& mouse);
    void OnMouseLeftUpScaleTool(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseWheel(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnMouseMovingRotationTool(wxMouseEvent& mouse);
    void OnMouseMovingScaleTool(wxMouseEvent& mouse);
    void OnKeyUp(wxKeyEvent& Key);
    void OnPaint(wxPaintEvent& paint);
    void OnColorPicked(wxColourPickerEvent& colorPicked);
    void OnColorRandomizer(wxCommandEvent& event);
    void OnContourResCtrl(wxSpinEvent& event);
    void OnContourResCtrl(wxCommandEvent& event);
    void OnMouseMovingIdle(wxMouseEvent& mouse);

    int GetState() const { return state; }
    int GetRes() const { return res; }
    void RecalcAll();

    // Calls refresh/update for this panel and output panel(s)
    void Redraw();

    // "Type" meaning Circle, Polygon, Rectangle, etc.
    void SetContourType(int id);
    void RemoveContour(int index);
    void FinalizeContour(wxMouseEvent& mouse);

    std::shared_ptr<Contour> CreateContour(wxPoint mousePos);

    void AddContour(std::shared_ptr<Contour> C);
    void InsertContour(std::shared_ptr<Contour> C, size_t i);

    void UpdateGrid() { grid.CalcVisibleGrid(); }

    void AddAnimation(std::shared_ptr<Animation> A) { animations.push_back(A); }
    void InsertAnimation(int i, std::shared_ptr<Animation> A)
    {
        if (i < 0)
            animations.push_back(A);
        else
            animations.insert(animations.begin() + i, A);
    }
    auto GetAnimation(int i)
    {
        return i > -1 ? animations[i] : animations.back();
    }
    void RemoveAnimation(int i)
    {
        if (i > -1)
            animations.erase(animations.begin() + i);
        else
            animations.pop_back();
    }
    size_t AnimCount() { return animations.size(); }

    void SetColorPicker(wxColourPickerCtrl* ptr) { colorPicker = ptr; };
    wxColor RandomColor();
    void AddOutputPlane(OutputPlane* out);
    void SetAnimPanel(AnimPanel* a) { animPanel = a; }
    auto GetAnimPanel() { return animPanel; }

    // t = -1 means don't use the parameter.
    bool DrawFrame(wxBitmap& image, double t = -1);

    double GetLongestAnimDur();

    // Gets a pointer to the function stored in the specifed output plane.
    // Not really planning to support multiple outputs anytime soon,
    // but whatever.
    ParsedFunc<cplx>* GetFunction(size_t outputIndex = 0);

    void OnShowAxes_Grid_Zeros(wxCommandEvent& event);

    Grid* GetGrid() { return &grid; }

    // If true, when axes step values change, grid step values
    // change accordingly
    bool linkGridToAxes                  = true;
    bool randomizeColor                  = true;
    bool animating                       = false;
    bool drawTooltip                     = false;
    wxColor color                        = wxColor(0, 0, 200);
    const wxColor BGcolor                = *wxWHITE;
    const int COLOR_SIMILARITY_THRESHOLD = 96;

    wxStopWatch animTimer;
    union
    {
        int contourCounts[6] = { 0,0,0,0,0,0 };
        struct
        {
            int CircleCount;
            int PolygonCount;
            int RectCount;
            int LineCount;
            int PointCount;
            int ParametricCount;
        };
    };

private:
    void DeSelect();
    void SelectionTool(wxMouseEvent& mouse);

    const int CIRCLED_POINT_RADIUS = 7;
    int res                        = 500;
    bool animateGrid               = false;
    bool showZeros = true;
    Grid grid;

    ContourPoint* mouseOnZero = nullptr;

    // Pointers to outputs for or sending refresh signals.
    // App only uses one output for now, but more might be nice later.
    std::vector<OutputPlane*> outputs;

    wxColourPickerCtrl* colorPicker = nullptr;
    AnimPanel* animPanel            = nullptr;
    std::vector<std::shared_ptr<Animation>> animations;

    int contourType = ID_Circle;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<ComplexPlane>(*this);
        ar& linkGridToAxes;
        ar& res;
        ar& grid;
        ar& animations;
        ar& contourCounts;
    }
    wxDECLARE_EVENT_TABLE();
};

// Workaround for wxWidgets handling double clicks poorly in modal dialogs.
#define wxDClickWorkaround() if (!mouseLeftDown) return; \
                                  mouseLeftDown = false; \
                                  SetFocus();