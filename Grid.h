#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>
//#include "Contour.h"

class ContourLine;
class ContourPolygon;
class ComplexPlane;
template <class T> class Parser;

class Grid {
   friend class TransformedGrid;

 public:
   Grid(ComplexPlane* par) : parent(par) {}
   Grid(ComplexPlane* par, double h, double v) : hStep(h), vStep(v), parent(par)
   {
   }
   ~Grid();

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
   // std::vector<ContourLine*> horiz;
   std::vector<ContourLine*> lines;
};

class TransformedGrid : public Grid {
 public:
   TransformedGrid(ComplexPlane* par) : Grid(par) {}
   ~TransformedGrid();
   void Draw(wxDC* dc, ComplexPlane* canvas);

   // applies a function to the lines of the input grid and stores them
   // as open polygons in this object.
   void MapGrid(Grid* grid, Parser<std::complex<double>>& f);
   int res = 80;

 private:
   // std::vector<ContourPolygon*> horiz;
   std::vector<ContourPolygon*> lines;
};