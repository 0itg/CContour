#pragma once

#include "ComplexPlane.h"
#include "Parser.h"

#include <complex>
#include <wx/spinctrl.h>

class InputPlane;
class Contour;
class TransformedGrid;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

class OutputPlane : public ComplexPlane {
   friend class InputPlane;

 public:
   OutputPlane(wxWindow* parent, InputPlane* In, std::string name = "output");
   ~OutputPlane();
   void OnMouseLeftUp(wxMouseEvent& mouse);
   // void OnMouseRightUp(wxMouseEvent& mouse);
   // void OnMouseRightDown(wxMouseEvent& mouse);
   void OnMouseMoving(wxMouseEvent& mouse);
   void OnPaint(wxPaintEvent& paint);
   void OnGridResCtrl(wxSpinEvent& event);
   void OnGridResCtrl(wxCommandEvent& event);
   void OnFunctionEntry(wxCommandEvent& event);

   void SetFuncInput(wxTextCtrl* fIn) {
      funcInput = fIn;
   }

   // int res = 200;

   ParsedFunc<std::complex<double>> f;

 private:
   Parser<std::complex<double>> parser;
   InputPlane* in;
   TransformedGrid* tGrid;
   wxTextCtrl* funcInput;
   wxDECLARE_EVENT_TABLE();
};
