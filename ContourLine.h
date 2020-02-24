#pragma once
#include "Contour.h"

typedef std::complex<double> cplx;

class ContourLine : public Contour {
 public:
   ContourLine(cplx c, wxColor col = wxColor(255, 255, 255),
               std::string n = "Line");
   ContourLine(cplx c, cplx d, wxColor col = wxColor(255, 255, 255),
               std::string n = "Line");
   ContourLine(wxColor col = wxColor(255, 255, 255));
   virtual ContourLine* Clone() {
      return new ContourLine(*this);
   };

   virtual void Draw(wxDC* dc, ComplexPlane* canvas);
   virtual void AddPoint(cplx mousePos){};
   virtual void ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked) {
      Translate(mousePos, lastPointClicked);
   };
   virtual bool IsDone() {
      return true;
   };
   virtual bool IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                 int pixPrecision = 3);
   virtual cplx Interpolate(double t);
   virtual ContourPolygon* Subdivide(int res);
};
