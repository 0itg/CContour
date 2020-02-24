#include "OutputPlane.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Event_IDs.h"
#include "Grid.h"
#include "InputPlane.h"
#include "Parser.h"
#include "Token.h"
#include "ToolPanel.h"

#include <wx/dcgraph.h>
#include <wx/richtooltip.h>

#include <algorithm>

// clang-format off
wxBEGIN_EVENT_TABLE(OutputPlane, wxPanel)
EVT_LEFT_UP(OutputPlane::OnMouseLeftUp)
EVT_RIGHT_UP(ComplexPlane::OnMouseRightUp)
EVT_RIGHT_DOWN(ComplexPlane::OnMouseRightDown)
EVT_MOUSEWHEEL(ComplexPlane::OnMouseWheel)
EVT_MOTION(OutputPlane::OnMouseMoving)
EVT_PAINT(OutputPlane::OnPaint)
EVT_LEAVE_WINDOW(ComplexPlane::OnMouseLeaving)
EVT_MOUSE_CAPTURE_LOST(ComplexPlane::OnMouseCapLost)
wxEND_EVENT_TABLE();
// clang-format on

OutputPlane::OutputPlane(wxWindow* parent, InputPlane* In, const std::string& n)
    : ComplexPlane(parent, n), in(In), inputContours((In->subDivContours)) {
   f = parser.Parse("z*z");
   In->outputs.push_back(this);
   tGrid = new TransformedGrid(this);
};

OutputPlane::~OutputPlane() {
   delete tGrid;
}

void OutputPlane::OnMouseLeftUp(wxMouseEvent& mouse) {
   if (panning)
      state = STATE_IDLE;
}

void OutputPlane::OnMouseMoving(wxMouseEvent& mouse) {
   cplx outCoord           = (ScreenToComplex(mouse.GetPosition()));
   std::string inputCoord  = "f(z) = " + f.str();
   std::string outputCoord = "f(z) = " + std::to_string(outCoord.real()) +
                             " + " + std::to_string(outCoord.imag()) + "i";
   statBar->SetStatusText(inputCoord, 0);
   statBar->SetStatusText(outputCoord, 1);

   if (panning)
      Pan(mouse.GetPosition());
   lastMousePos = ScreenToComplex(mouse.GetPosition());
   Highlight(mouse.GetPosition());
   int temp               = in->highlightedContour;
   in->highlightedContour = highlightedContour;
   if (temp != highlightedContour) {
      in->Refresh();
      in->Update();
   }
}

void OutputPlane::OnPaint(wxPaintEvent& paint) {
   wxAutoBufferedPaintDC pdc(this);
   wxGCDC dc(pdc);
   // wxDCClipper(dc, GetClientSize());
   dc.Clear();
   wxPen pen(tGrid->color, 1);
   wxBrush brush(*wxTRANSPARENT_BRUSH);
   // wxBrush brush(wxColor(255, 0, 0, 128), wxBRUSHSTYLE_SOLID);
   dc.SetPen(pen);
   dc.SetBrush(brush);

   // Only recalculate the mapping if the viewport changed.
   if (movedViewPort)
      tGrid->MapGrid(in->grid, f);

   if (showGrid)
      tGrid->Draw(&dc, this);

   // for (auto C : contours) delete C;
   // contours.clear();
   // for (auto C : in->subDivContours) {
   // contours.push_back(C->ApplyToClone(f)); }

   for (int i = 0; i < inputContours.size(); i++) {
      if (inputContours[i]->markedForRedraw) {
         delete contours[i];
         contours[i]                       = inputContours[i]->ApplyToClone(f);
         inputContours[i]->markedForRedraw = false;
      }
   }
   pen.SetWidth(2);

   for (auto C : contours) {
      pen.SetColour(C->color);
      dc.SetPen(pen);
      C->Draw(&dc, this);
   }
   if (highlightedContour > -1) {
      pen.SetColour(contours[highlightedContour]->color);
      pen.SetWidth(3);
      dc.SetPen(pen);
      contours[highlightedContour]->Draw(&dc, this);
   }

   if (showAxes)
      axes.Draw(&dc);
   movedViewPort = false;

   toolPanel->Refresh();
   toolPanel->Update();
}

void OutputPlane::OnGridResCtrl(wxSpinEvent& event) {
   tGrid->res = resCtrl->GetValue();
}

void OutputPlane::OnGridResCtrl(wxCommandEvent& event) {
   tGrid->res    = resCtrl->GetValue();
   movedViewPort = true;
   Refresh();
   Update();
}

void OutputPlane::OnFunctionEntry(wxCommandEvent& event) {
   ParsedFunc g = f;
   try {
      f = parser.Parse(funcInput->GetLineText(0));
      f.eval();
   } catch (std::invalid_argument& func) {
      f = g;
      wxRichToolTip errormsg(wxT("Invalid Function"), func.what());
      errormsg.ShowFor(funcInput);
   }
   movedViewPort = true;
   varPanel->PopulateVarTextCtrls(f);
   Refresh();
   Update();
}

void OutputPlane::MarkAllForRedraw() {
   // std::transform(inputContours.begin(), inputContours.end(), contours)
   for (auto C : contours)
      delete C;
   in->RecalcAll();
   for (int i = 0; i < contours.size(); i++) {
      contours[i] = inputContours[i]->ApplyToClone(f);
   }
}

int OutputPlane::GetRes() {
   return tGrid->res;
}
