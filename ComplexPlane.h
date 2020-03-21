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
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>

typedef std::complex<double> cplx;

enum enum_states
{
    // values of state >= 0 represent contour indices.
    STATE_IDLE = -1,
};

class Contour;
class OutputPlane;
class ComplexPlane;
class Grid;
class NumCtrlPanel;

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

    void RefreshShowAxes_ShowGrid();

    std::string& GetName() { return name; }

    wxSize DoBestClientSize() const { return wxSize(200, 200); }
    void SetToolbar(wxToolBar* tb) { toolbar = tb; }
    void SetStatusBar(wxStatusBar* ptr) { statBar = ptr; };
    void SetToolPanel(NumCtrlPanel* ptr) { toolPanel = ptr; };
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
    // void InversePan(wxPoint mousePos);
    void Zoom(wxPoint mousePos, int zoomSteps);

    void SetResCtrl(wxSpinCtrl* r) { resCtrl = r; }
    void ClearContours();

    int GetHighlightedContour() { return highlightedContour; }
    void SetHighlightedContour(int h) { highlightedContour = h; }

    wxArrayString GetContourNames();

    size_t GetContourCount() { return contours.size(); }

    Contour* GetContour(size_t index)
    {
        if (index < contours.size())
            return contours[index].get();
        else
            return nullptr;
    }

    Axes axes;
    bool movedViewPort = true;

  protected:
    std::string name;
    std::vector<std::unique_ptr<Contour>> contours;
    int highlightedContour   = -1;
    int state                = -1;
    int highlightedCtrlPoint = -1;
    const double zoomFactor  = 1.1;
    cplx lastMousePos;
    cplx lastMidClick;

    bool mouseLeftDown = false;
    bool panning       = false;
    bool showAxes      = true;
    bool showGrid      = true;
    wxSpinCtrl* resCtrl;
    wxStatusBar* statBar;
    wxToolBar* toolbar;
    NumCtrlPanel* toolPanel;

  private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar.template register_type<ComplexPlane>();
        ar& axes;
        movedViewPort = false;
        ar& name;
        ar& contours;
        highlightedContour   = -1;
        state                = -1;
        highlightedCtrlPoint = -1;
        panning              = false;
        ar& showAxes;
        ar& showGrid;
        // NOTE: Must set parent and pointers to other controls manually
        // after loading from archive, or load into a propely set-up object.
        // Second approach is the one the app uses.
    }
};