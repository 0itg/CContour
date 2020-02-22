#pragma once
#include "Contour.h"
class ContourLine : public Contour {
 public:
   ContourLine(std::complex<double> c, wxColor col = wxColor(255, 255, 255), std::string n = "Line");
   ContourLine(std::complex<double> c, std::complex<double> d,
               wxColor col = wxColor(255, 255, 255), std::string n = "Line");
   ContourLine(wxColor col = wxColor(255, 255, 255));
   virtual ContourLine* Clone() {
      return new ContourLine(*this);
   };

   virtual void Draw(wxDC* dc, ComplexPlane* canvas);
   virtual void AddPoint(std::complex<double> mousePos){};
   virtual void ActionNoCtrlPoint(std::complex<double> mousePos,
                                  std::complex<double> lastPointClicked) {
      Translate(mousePos, lastPointClicked);
   };
   virtual bool IsDone() {
      return true;
   };
   virtual bool IsPointOnContour(std::complex<double> pt, ComplexPlane* canvas,
                            int pixPrecision = 3);
   virtual std::complex<double> Interpolate(double t);
   virtual ContourPolygon* Subdivide(int res);
};
