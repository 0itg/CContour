#pragma once
#include "Contour.h"
#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;

class ContourPolygon;

class ContourCircle : public Contour
{
    friend class boost::serialization::access;

public:
    ContourCircle() noexcept {}
    ContourCircle(cplx c, double r = 0, wxColor col = wxColor(255, 255, 255),
                  std::string n = "Circle") noexcept;
    virtual ContourCircle* Clone() noexcept
    {
        return new ContourCircle(*this);
    };

    void Draw(wxDC* dc, ComplexPlane* canvas);
    // Changes the radius, rather than translating.
    int ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked);
    virtual CommandContourEditRadius* CreateActionCommand(cplx c);
    bool IsDone() { return radius != 0; }
    bool IsPointOnContour(cplx pt, ComplexPlane* canvas, int pixPrecision = 3);
    int OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision = 3);
    cplx Interpolate(double t);
    void Subdivide(int res);
    void SetRadius(double r) { radius = r; }
    double GetRadius() { return radius; }

    virtual void RotateAndScale(cplx V, cplx pivot = cplx(INFINITY, INFINITY));

    virtual std::tuple<int, int, int> PopulateSupplementalMenu(ToolPanel* TP);

private:
    double radius = 0;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& radius;
        ar& boost::serialization::base_object<Contour>(*this);
    }
};

BOOST_CLASS_EXPORT_KEY(ContourCircle)