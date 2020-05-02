#pragma once
#include "Contour.h"

#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;

class ContourPoint : public Contour
{
	friend class boost::serialization::access;
public:
    ContourPoint(cplx c = cplx(0,0), wxColor col = wxColor(0, 0, 0),
        std::string n = "Point", int ord = 0) noexcept;
    virtual ContourPoint* Clone() noexcept
    {
        return new ContourPoint(*this);
    };
    virtual void Draw(wxDC* dc, ComplexPlane* canvas);
    virtual void DrawLabel(wxDC* dc, ComplexPlane* canvas);
    // I think this is impossible to activate in practice.
    virtual int ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked);
    virtual CommandContourTranslate* CreateActionCommand(cplx c);
    virtual bool IsDone() { return true; };
    // Always returns the point's location. Nothing to interpolate.
    virtual cplx Interpolate(double t) { return points[0]; }
    // Only one point. Can't add more.
    virtual void AddPoint(cplx mousePos) {}
    virtual bool RotationEnabled() { return false; }
    virtual bool ScalingEnabled() { return false; }
    virtual bool IsPointOnContour(cplx pt, ComplexPlane* canvas,
        int pixPrecision = 4);
    virtual Contour* Map(ParsedFunc<cplx>& f, int res);
    static constexpr int POINT_RADIUS = 3;

    int GetOrder() { return order; }
    int SetOrder(int o) { order = 0; }
private:
    int order = 0;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Contour, ContourPoint>(*this);
        ar& order;
    }
};

BOOST_CLASS_EXPORT_KEY(ContourPoint)