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

class ComplexPlane;

typedef std::complex<double> cplx;

double DistancePointToLine(cplx pt, cplx z1, cplx z2);
bool IsInsideBox(cplx pt, cplx z1, cplx z2);

void DrawClippedLine(wxPoint p1, wxPoint p2, wxDC* dc, ComplexPlane* canvas);

// Removes file extension from string.
std::string removeExt(const std::string& str);

// Serialization for wxColor.
template <class Archive>
void save(Archive& ar, const wxColour& c, unsigned int version)
{
    ar << (uint32_t)c.GetRGBA();
}
template <class Archive>
void load(Archive& ar, wxColour& c, unsigned int version)
{
    uint32_t x;
    ar >> x;
    c.SetRGBA(x);
}
BOOST_SERIALIZATION_SPLIT_FREE(wxColour)

// Serialization for wxPoint
namespace boost
{
namespace serialization
{
template <class Archive>
void serialize(Archive& ar, wxPoint& p, const unsigned int version)
{
    ar& p.x;
    ar& p.y;
}
} // namespace serialization
} // namespace boost