#pragma once

#include "ComplexPlane.h"
#include "Parser.h"
#include "ToolPanel.h"

#include <complex>
#include <wx/spinctrl.h>

class InputPlane;
class ContourPolygon;
class TransformedGrid;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

typedef std::complex<double> cplx;

class OutputPlane : public ComplexPlane {
   friend class InputPlane;

 public:
   OutputPlane(wxWindow* parent, InputPlane* In,
               const std::string& name = "output");
   ~OutputPlane();
   void OnMouseLeftUp(wxMouseEvent& mouse);
   // void OnMouseRightUp(wxMouseEvent& mouse);
   // void OnMouseRightDown(wxMouseEvent& mouse);
   void OnMouseMoving(wxMouseEvent& mouse);
   void OnPaint(wxPaintEvent& paint);
   void OnGridResCtrl(wxSpinEvent& event);
   void OnGridResCtrl(wxCommandEvent& event);
   void OnFunctionEntry(wxCommandEvent& event);

   void MarkAllForRedraw();
   void SetFuncInput(wxTextCtrl* fIn) {
      funcInput = fIn;
   }
   void SetVarPanel(VariableEditPanel* var) {
      varPanel = var;
   }

   int GetRes();

   ParsedFunc<cplx> f;

 private:
   Parser<cplx> parser;
   InputPlane* in;
   std::vector<ContourPolygon*>& inputContours;
   TransformedGrid* tGrid;
   wxTextCtrl* funcInput;
   VariableEditPanel* varPanel;
   wxDECLARE_EVENT_TABLE();
};
