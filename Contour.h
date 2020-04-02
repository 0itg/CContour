#pragma once

#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "Commands.h"
#include "ComplexPlane.h"
#include "Utilities.h"

struct Axes;
class ToolPanel;
class InputPlane;
class ContourPolygon;
class ComplexPlane;
class ToolPanel;
class Command;
template <class T> class ParsedFunc;

// All contours include a set of points, may be open or closed,
// and need to define a function for interpolating the points
// (e.g. straight lines for polygons, or some spline function)

typedef std::complex<double> cplx;

class Contour
{
    friend class ToolPanel;
    friend class boost::serialization::access;

    public:
    virtual ~Contour() {}
    virtual Contour* Clone() = 0;

    virtual void Draw(wxDC* dc, ComplexPlane* canvas) = 0;
    virtual void AddPoint(cplx mousePos);
    virtual void RemovePoint(int index);

    // Action to be taken when selecting the contour between control
    // points. Typically it will just call the Translate function.
    // Return value tells what type of action was taken.
    virtual int ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked) = 0;
    virtual Command* CreateActionCommand(cplx c)                        = 0;

    virtual void Translate(cplx z1, cplx z2);

    virtual bool RotationEnabled() { return true; }
    virtual bool ScalingEnabled() { return true; }
    // if pivot = cplx(INFINITY, INFINITY), default to this->center
    virtual void RotateAndScale(cplx V, cplx pivot = cplx(INFINITY, INFINITY));
    virtual void Rotate(cplx V, cplx pivot = cplx(INFINITY, INFINITY));
    virtual void Rotate(double angle, cplx pivot = cplx(INFINITY, INFINITY));
    virtual void Scale(double factor, cplx pivot = cplx(INFINITY, INFINITY));

    // Returns true if editing is finished on this contour.
    virtual bool IsDone() = 0;

    virtual bool IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                  int pixPrecision = 3) = 0;

    // Returns the index of the control point under the mouse, -1 if none.
    virtual int OnCtrlPoint(cplx pt, ComplexPlane* canvas,
                            int pixPrecision = 3);
    virtual cplx GetCtrlPoint(int index);
    virtual void SetCtrlPoint(int index, cplx c);
    virtual std::string& GetName() { return name; }

    // Any actions to be taken when editing is finished.
    virtual void Finalize() { CalcCenter(); };

    // Base version draws the Contour's name, followed by a list of control
    // points and LinkedCtrls linked to them. See NumCtrlPanel in ToolPanel.h.
    virtual void PopulateMenu(ToolPanel* TP);

    // Allows derived classes to add extra menu items before the control points,
    // E.g. a LinkedTextCtrl for the radius of a circle.
    // returns number of sizer slots used.
    virtual std::tuple<int, int, int> PopulateSupplementalMenu(ToolPanel* TP)
    {
        return std::make_tuple(0, 0, 0);
    }

    // Parameterizing the contour as g(t) with 0 < t < 1, returns g(t).
    virtual cplx Interpolate(double t) = 0;

    // Creates a polygonal approximation of the contour with number of segments
    // dependent on res. No actual necessity for the number to equal res
    // precisely, but it would be expected from the user.
    virtual void Subdivide(int res) = 0;

    // Creates a Polygon by applying f to the subDiv points.
    virtual ContourPolygon* Map(ParsedFunc<cplx>& f);

    int GetPointCount() { return (int)points.size(); }
    void Reserve(size_t size) { points.reserve(size); }
    cplx GetCenter() { return center; }
    cplx CalcCenter();
    void DrawCtrlPoint(wxDC* dc, wxPoint p);

    // Useed to separate out functions which have a ParsedFunc that can be
    // animated by parameterizing a variable.
    virtual bool IsParametric() { return false; }

    wxColor color = *wxRED;

    // Used for deciding whether OutputPlane needs to recalculate curves.
    bool markedForRedraw = true;
    // If true, contour should be drawn with dashed lines and OutputPlane
    // should skip it.
    bool isPathOnly = false;

    protected:
    std::string name;
    std::vector<cplx> points;
    std::vector<cplx> subDiv;

    cplx center;

    private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& name;
        ar& points;
        ar& color;
        ar& isPathOnly;
        CalcCenter();
    }
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Contour)