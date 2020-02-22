#pragma once
#include "ContourPolygon.h"

class ContourRect : public ContourPolygon {
 public:
   ContourRect(std::complex<double> c, wxColor col = wxColor(255, 255, 255),
               std::string n = "Rectangle");
   ContourRect* Clone() {
      return new ContourRect(*this);
   };

   void Draw(wxDC* dc, ComplexPlane* canvas);
   void RemovePoint(int index);
   void moveCtrlPoint(std::complex<double> mousePos, int ptIndex = -1);
   bool IsDone() {
      return true;
   }
   void Finalize(){};

   //virtual void PopulateMenu(ToolPanel* TP);

 private:
   std::vector<double> sideLengths;
};
