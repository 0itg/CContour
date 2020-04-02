#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/aui/aui.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/spinctrl.h>

#include <complex>
#include <functional>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>

typedef std::complex<double> cplx;

enum enum_states
{
    // values of state >= 0 represent contour indices.
    STATE_IDLE = -1,
};

enum actions
{
    ACTION_IDLE = 0,
    ACTION_CREATE,
    ACTION_TRANSLATE,
    ACTION_EDIT_POINT,
    ACTION_EDIT_RADIUS
};

class Contour;
class OutputPlane;
class ComplexPlane;
class Grid;
class NumCtrlPanel;
class CommandHistory;

struct Axes
{
    friend class boost::serialization::access;
    Axes() {}
    Axes(ComplexPlane* p) : parent(p){};
    ComplexPlane* parent;
    union {
        double c[4] = {-10, 10, -10, 10};
        struct
        {
            double realMin;
            double realMax;
            double imagMin;
            double imagMax;
        };
    };
    double reStep = 1;
    double imStep = 1;
    void Draw(wxDC* dc);
    const int TICK_WIDTH = 8;
    // Used to determine when the Step values should be automatically increased
    // So the tick marks don't get too close together or too far apart during
    // zoom. Min number of ticks is value / 2, max is value * 2.
    const int TARGET_TICK_COUNT = 20;
    // Label every n tick marks
    const int LABEL_SPACING = 4;
    // Labels can get no closer than this to the edge of the plane
    const int LABEL_PADDING = 4;
    void RecalcSteps();
    // void CopySettings(const Axes& A);
private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& parent;
        ar& c;
        ar& reStep;
        ar& imStep;
    }
};

class ComplexPlane : public wxPanel
{
    friend class boost::serialization::access;

public:
    ComplexPlane() {}
    ComplexPlane(wxWindow* parent, const std::string& name);
    // ComplexPlane(const ComplexPlane& P);
    virtual ~ComplexPlane() {}

    // Functions for converting between screen and mathematical coordinates.
    cplx ScreenToComplex(wxPoint P);
    wxPoint ComplexToScreen(cplx C);
    double LengthXToScreen(double r);
    double LengthYToScreen(double r);
    double ScreenXToLength(double r);
    double ScreenYToLength(double r);

    void OnMouseWheel(wxMouseEvent& mouse);
    void OnMouseLeftDown(wxMouseEvent& mouse);
    void OnMouseRightUp(wxMouseEvent& mouse);
    void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseCapLost(wxMouseCaptureLostEvent& mouse);
    void OnMouseLeaving(wxMouseEvent& mouse);
    void OnShowAxes_ShowGrid(wxCommandEvent& event);
    void ShowAxisControls();

    void RefreshShowAxes_ShowGrid();

    const std::string& GetName_() const { return name; }

    wxSize DoBestClientSize() const { return wxSize(200, 200); }
    void SetToolbar(wxToolBar* tb) { toolbar = tb; }
    void SetStatusBar(wxStatusBar* ptr) { statBar = ptr; };
    void SetToolPanel(NumCtrlPanel* ptr) { toolPanel = ptr; };
    auto GetToolPanel() const { return toolPanel; }
    virtual void PrepareForLoadFromFile() { ClearContours(); }

    // For convenience
    void CaptureMouseIfAble()
    {
        if (!HasCapture()) CaptureMouse();
    }
    void ReleaseMouseIfAble()
    {
        if (HasCapture()) ReleaseMouse();
    }

    // Flags the contour/control point under this point as highlighted,
    // which marks it for special treatment in the user input and paint
    // routines. True return value means the highlights changed.
    bool Highlight(wxPoint mousePos);
    void Pan(wxPoint mousePos);
    void Zoom(wxPoint mousePos, int zoomSteps);

    void SetResCtrl(wxSpinCtrl* r) { resCtrl = r; }
    int GetRes() { return resCtrl->GetValue(); }
    void ClearContours();
    virtual void UpdateGrid() {}

    int GetHighlightedContour() { return active; }
    void SetHighlightedContour(int h) { active = h; }

    wxArrayString GetContourNames();
    // Needs a better name. If true, returns a map of all the parametric
    // contours and their indices. If false, returns all the non-parametric
    // contours.
    std::map<std::string, int> GetParametricContours(bool parametric = true);

    size_t GetContourCount() { return contours.size(); }

    std::shared_ptr<Contour> GetContour(int index)
    {
        if (index < 0)
            return nullptr;
        else if (index < contours.size())
            return contours[index];
        else
            return nullptr;
    }

    void SetCommandHistory(CommandHistory* h) { history = h; }
    CommandHistory* GetHistoryPtr() { return history; }

    virtual bool DrawFrame(wxBitmap& image, double t = -1) { return false; }

    Axes axes;
    bool movedViewPort     = true;
    bool cullLargeSegments = false;

protected:
    std::string name;
    std::vector<std::shared_ptr<Contour>> contours;
    int active              = -1;
    int state               = -1;
    int activePt            = -1;
    const double zoomFactor = 1.1;
    cplx lastMousePos;
    cplx lastClickPos;

    bool mouseLeftDown = false;
    bool panning       = false;
    bool showAxes      = true;
    bool showGrid      = true;
    wxSpinCtrl* resCtrl;
    wxStatusBar* statBar;
    wxToolBar* toolbar;
    NumCtrlPanel* toolPanel;
    CommandHistory* history;

private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar.template register_type<ComplexPlane>();
        ar& axes;
        movedViewPort = false;
        ar& name;
        ar& contours;
        active   = -1;
        state    = -1;
        activePt = -1;
        panning  = false;
        ar& showAxes;
        ar& showGrid;
        // NOTE: Must set parent and pointers to other controls manually
        // after loading from archive, or load into a propely set-up object.
        // Second approach is the one the app uses.
    }
};