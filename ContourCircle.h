#pragma once
#include "Contour.h"

typedef std::complex<double> cplx;

class ContourPolygon;

class ContourCircle : public Contour {
 public:
   ContourCircle(cplx c, double r = 0, wxColor col = wxColor(255, 255, 255),
                 std::string n = "Circle");
   virtual ContourCircle* Clone() {
      return new ContourCircle(*this);
   };

   void Draw(wxDC* dc, ComplexPlane* canvas);
   void moveCtrlPoint(cplx mousePos, int ptIndex = -1);
   // Changes the radius, rather than translating.
   void ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked);
   bool IsDone() {
      return true;
   }
   bool IsPointOnContour(cplx pt, ComplexPlane* canvas, int pixPrecision = 3);
   int OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision = 3);
   cplx Interpolate(double t);
   ContourPolygon* Subdivide(int res);

   virtual wxSize PopulateSupplementalMenu(ToolPanel* TP, wxPoint UL);

 private:
   double radius;
};