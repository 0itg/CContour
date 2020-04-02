#pragma once

#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <complex>

#include "ContourLine.h"
#include "ContourPolygon.h"

typedef std::complex<double> cplx;

class ComplexPlane;
template <class T> class ParsedFunc;

class Grid
{
    friend class TransformedGrid;
    friend class boost::serialization::access;

    public:
    Grid() {}
    Grid(ComplexPlane* par) : parent(par) {}
    Grid(ComplexPlane* par, double h, double v)
        : hStep(h), vStep(v), parent(par)
    {
    }

    virtual void Draw(wxDC* dc, ComplexPlane* canvas);

    // Sets up the grid lines based on the current viewport
    // and stores them as ContourLines so TransformedGrid can use them.
    void CalcVisibleGrid();

    double hStep  = 1;
    double vStep  = 1;
    wxColor color = wxColor(216, 216, 216);

    protected:
    ComplexPlane* parent;

    private:
    std::vector<std::unique_ptr<ContourLine>> lines;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& parent;
        ar& hStep;
        ar& vStep;
    }
};

class TransformedGrid : public Grid
{
    friend class boost::serialization::access;

    public:
    TransformedGrid() {}
    TransformedGrid(ComplexPlane* par) : Grid(par) {}

    void Draw(wxDC* dc, ComplexPlane* canvas);

    // applies a function to the lines of the input grid and stores them
    // as open polygons in this object.
    void MapGrid(const Grid& grid, ParsedFunc<cplx>& f);
    int res = 200;

    private:
    std::vector<std::unique_ptr<ContourPolygon>> lines;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Grid>(*this);
        ar& res;
        ar& lines;
    }
};

BOOST_CLASS_EXPORT_KEY(TransformedGrid)