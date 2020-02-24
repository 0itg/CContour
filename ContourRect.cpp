#include "ContourRect.h"
#include "ComplexPlane.h"
#include "ToolPanel.h"

ContourRect::ContourRect(cplx c, wxColor col, std::string n)
    : ContourPolygon(c, col) {
   // Two corner points define the rectangle, but adding the other two
   // points allows all corners to be used for resizing it
   points.push_back(c);
   points.push_back(c);
   closed = true;
   name   = n;
}

void ContourRect::Draw(wxDC* dc, ComplexPlane* canvas) {
   int width = (int)canvas->LengthToScreen(points[2].real() - points[0].real());
   int height =
       (int)-canvas->LengthToScreen(points[2].imag() - points[0].imag());
   dc->DrawRectangle(canvas->ComplexToScreen(points[0]), wxSize(width, height));
}

void ContourRect::RemovePoint(int index) {
   points.clear(); // Removing a point means it's no longer a rectangle
}

void ContourRect::moveCtrlPoint(cplx mousePos, int ptIndex) {
   points[ptIndex] = mousePos;
   // When moving one point, automatically move the adjacent points to
   // maintain rectangular shape.
   if (ptIndex % 2) {
      points[2] = cplx(points[3].real(), points[1].imag());
      points[0] = cplx(points[1].real(), points[3].imag());
   } else {
      points[1] = cplx(points[0].real(), points[2].imag());
      points[3] = cplx(points[2].real(), points[0].imag());
   }
}

// void ContourRect::PopulateMenu(ToolPanel* TP) {
//   int distFromTop = 18;
//   TP->AddDecoration(new wxStaticText(
//       TP, wxID_ANY, wxString(GetName() + ":"),
//       wxDefaultPosition + wxSize(12, distFromTop), wxDefaultSize));
//   distFromTop += 24;
//
//   distFromTop += PopulateSupplementalMenu(TP, wxPoint(0, distFromTop)).y;
//
//   for (int i = 0; i < GetPointCount(); i += 2) {
//      std::string c = std::to_string(GetCtrlPoint(i).real()) + " + " +
//                      std::to_string(GetCtrlPoint(i).imag()) + "i";
//      TP->AddDecoration(new wxStaticText(
//          TP, wxID_ANY, wxString("Ctrl Point " + std::to_string(i)),
//          wxDefaultPosition + wxSize(12, distFromTop), wxDefaultSize));
//      TP->AddLinkedTextCtrl(new LinkedCtrlPointTextCtrl(
//          TP, wxID_ANY, c, wxDefaultPosition + wxPoint(12, distFromTop + 18),
//          wxDefaultSize, wxTE_PROCESS_ENTER, points, (size_t)i));
//      distFromTop += TP->SPACING;
//   }
//}
