#pragma once
#include "Contour.h"

#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;

class ContourLine : public Contour
{
    friend class boost::serialization::access;

public:
    // ContourLine() {}
    ContourLine(cplx c, wxColor col = wxColor(255, 255, 255),
                std::string n = "Line") noexcept;
    ContourLine(cplx c, cplx d, wxColor col = wxColor(255, 255, 255),
                std::string n = "Line") noexcept;
    ContourLine(wxColor col = wxColor(255, 255, 255)) noexcept;
    virtual ContourLine* Clone() noexcept { return new ContourLine(*this); };

    virtual void Draw(wxDC* dc, ComplexPlane* canvas);
    virtual void AddPoint(cplx mousePos){};
    virtual int ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked)
    {
        Translate(mousePos, lastPointClicked);
        return ACTION_TRANSLATE;
    };
    virtual CommandContourTranslate* CreateActionCommand(cplx c);
    virtual bool IsDone() { return GetCtrlPoint(0) != GetCtrlPoint(1); };
    virtual bool IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                  int pixPrecision = 3);
    virtual cplx Interpolate(double t);

private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar.template register_type<ContourLine>();
        ar& boost::serialization::base_object<Contour>(*this);
    }
};

BOOST_CLASS_EXPORT_KEY(ContourLine)