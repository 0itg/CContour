#pragma once
#include "Contour.h"
#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;

class ContourPolygon;

class ContourCircle : public Contour
{
    friend class boost::serialization::access;

  public:
    ContourCircle() {}
    ContourCircle(cplx c, double r = 0, wxColor col = wxColor(255, 255, 255),
                  std::string n = "Circle");
    virtual ContourCircle* Clone()
    {
        return new ContourCircle(*this);
    };

    void Draw(wxDC* dc, ComplexPlane* canvas);
    void moveCtrlPoint(cplx mousePos, int ptIndex = -1);
    // Changes the radius, rather than translating.
    void ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked);
    bool IsDone()
    {
        return true;
    }
    bool IsPointOnContour(cplx pt, ComplexPlane* canvas, int pixPrecision = 3);
    int OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision = 3);
    cplx Interpolate(double t);
    void Subdivide(int res);

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