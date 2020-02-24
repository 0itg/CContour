#pragma once
#include "Contour.h"

typedef std::complex<double> cplx;
template <class T> class ParsedFunc;

class ContourPolygon : public Contour {
 public:
   ContourPolygon(cplx c, wxColor col = wxColor(255, 255, 255),
                  std::string n = "Polygon");
   ContourPolygon(wxColor col   = wxColor(255, 255, 255),
                  std::string n = "Polygon");
   virtual ContourPolygon* Clone() {
      return new ContourPolygon(*this);
   };

   virtual void Draw(wxDC* dc, ComplexPlane* canvas);
   virtual void ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked);
   virtual bool IsDone();
   virtual bool IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                 int pixPrecision = 3);
   virtual void Finalize();
   cplx Interpolate(double t);
   ContourPolygon* Subdivide(int res);
   void Apply(ParsedFunc<cplx>& f);
   ContourPolygon* ApplyToClone(ParsedFunc<cplx>& f);

   // Used for deciding whether OutputPlane needs to recalculate curves.
   bool markedForRedraw = true;

 protected:
   bool closed = false;
   std::vector<double> sideLengths;
   void CalcSideLengths();

 private:
   double perimeter = 0;
};
