#pragma once
#include "ContourPolygon.h"

#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;

class ContourRect : public ContourPolygon
{
    friend class boost::serialization::access;

    public:
    ContourRect() noexcept { closed = true; }
    ContourRect(cplx c, wxColor col = wxColor(255, 255, 255),
                std::string n = "Rectangle") noexcept;
    ContourRect* Clone() noexcept { return new ContourRect(*this); };

    void Draw(wxDC* dc, ComplexPlane* canvas);
    void RemovePoint(int index);
    void SetCtrlPoint(int ptIndex, cplx mousePos);
    bool IsDone() { return true; }
    void Finalize(){};
    // Rotation is disabled, since drawing/editing functions assume a rectangle
    // to be square with the screen. Use a ContourPolygon if rotation is needed.
    virtual bool RotationEnabled() { return false; }
    virtual void Rotate(double angle, cplx pivot = cplx(INFINITY, INFINITY)) {}
    virtual void Rotate(cplx V, cplx pivot = cplx(INFINITY, INFINITY)) {}
    // Since rotation is disabled, the function ignores the arg and just scales.
    virtual void RotateAndScale(cplx V, cplx pivot = cplx(INFINITY, INFINITY));

    private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<ContourPolygon, ContourRect>(
            *this);
    }
};

BOOST_CLASS_EXPORT_KEY(ContourRect)