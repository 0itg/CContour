#pragma once
#include "ContourPolygon.h"

#include <boost/serialization/export.hpp>

typedef std::complex<double> cplx;

class ContourRect : public ContourPolygon
{
    friend class boost::serialization::access;

  public:
    ContourRect() {}
    ContourRect(cplx c, wxColor col = wxColor(255, 255, 255),
                std::string n = "Rectangle");
    ContourRect* Clone()
    {
        return new ContourRect(*this);
    };

    void Draw(wxDC* dc, ComplexPlane* canvas);
    void RemovePoint(int index);
    void moveCtrlPoint(cplx mousePos, int ptIndex = -1);
    bool IsDone()
    {
        return true;
    }
    void Finalize(){};

  private:
    std::vector<double> sideLengths;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<ContourPolygon, ContourRect>(
            *this);
    }
};

BOOST_CLASS_EXPORT_KEY(ContourRect)