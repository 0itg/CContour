#pragma once
#include "ContourPolygon.h"

typedef std::complex<double> cplx;

class ContourRect : public ContourPolygon {
 public:
   ContourRect(cplx c, wxColor col = wxColor(255, 255, 255),
               std::string n = "Rectangle");
   ContourRect* Clone() {
      return new ContourRect(*this);
   };

   void Draw(wxDC* dc, ComplexPlane* canvas);
   void RemovePoint(int index);
   void moveCtrlPoint(cplx mousePos, int ptIndex = -1);
   bool IsDone() {
      return true;
   }
   void Finalize(){};

 private:
   std::vector<double> sideLengths;
};
