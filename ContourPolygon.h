#pragma once
#include "Contour.h"

#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;
template <class T> class ParsedFunc;

class ContourPolygon : public Contour
{
    friend class boost::serialization::access;

  public:
    // ContourPolygon() {}
    ContourPolygon(cplx c, wxColor col = wxColor(0, 0, 0),
                   std::string n = "Polygon") noexcept;
    ContourPolygon(wxColor col   = wxColor(0, 0, 0),
                   std::string n = "Polygon") noexcept;
    virtual ContourPolygon* Clone() noexcept
    {
        return new ContourPolygon(*this);
    };

    virtual void Draw(wxDC* dc, ComplexPlane* canvas);
    virtual int ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked);
    virtual CommandContourTranslate* CreateActionCommand(cplx c);
    virtual bool IsDone();
    virtual bool IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                  int pixPrecision = 3);
    virtual void Finalize();
    virtual cplx Interpolate(double t);
    void Subdivide(int res);

  protected:
    bool closed = false;
    std::vector<double> sideLengths;
    double perimeter = 0;
    void CalcSideLengths();

  private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Contour, ContourPolygon>(*this);
        ar& closed;
    }
};

BOOST_CLASS_EXPORT_KEY(ContourPolygon)