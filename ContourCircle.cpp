#include "ContourCircle.h"
#include "ComplexPlane.h"
#include "ContourPolygon.h"
#include "LinkedTextCtrl.h"
#include "ToolPanel.h"

ContourCircle::ContourCircle(cplx c, double r, wxColor col, std::string n)
    : radius(r) {
   points.push_back(c);
   color = col;
   name  = n;
}

void ContourCircle::Draw(wxDC* dc, ComplexPlane* canvas) {
   wxPoint p = canvas->ComplexToScreen(points[0]);
   dc->DrawCircle(p, (wxCoord)canvas->LengthToScreen(radius));
   DrawCtrlPoint(dc, p);
}

void ContourCircle::moveCtrlPoint(cplx mousePos, int ptIndex) {
   points[0] = mousePos;
}

void ContourCircle::ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked) {
   radius = abs(points[0] - mousePos);
}

bool ContourCircle::IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                     int pixPrecision) {
   return abs(abs(points[0] - pt) - radius) <
          canvas->ScreenToLength(pixPrecision);
}

int ContourCircle::OnCtrlPoint(cplx pt, ComplexPlane* canvas,
                               int pixPrecision) {
   // returns 1-1=0 if the mouse is on the control point (referring to
   // control point 0. Returns 0-1=-1 if not, with -1
   // meaning no contour found.
   return (abs(points[0] - pt) < canvas->ScreenToLength(pixPrecision)) - 1;
}

cplx ContourCircle::Interpolate(double t) {
   return points[0] +
          cplx(radius * cos(2 * M_PI * t), radius * sin(2 * M_PI * t));
}

ContourPolygon* ContourCircle::Subdivide(int res) {
   ContourPolygon* D = new ContourPolygon();
   for (int i = 0; i <= res; i++) {
      double t = (double)i / res;
      D->AddPoint(Interpolate(t));
   }
   D->color = color;
   return D;
}

wxSize ContourCircle::PopulateSupplementalMenu(ToolPanel* TP, wxPoint UL) {
   TP->AddDecoration(new wxStaticText(TP, wxID_ANY, wxString("Radius: "),
                                      wxDefaultPosition + wxSize(12, UL.y),
                                      TP->TEXTBOX_SIZE));
   TP->AddLinkedTextCtrl(
       new LinkedDoubleTextCtrl(TP, wxID_ANY, wxString(std::to_string(radius)),
                          wxDefaultPosition + wxPoint(12, UL.y + 18),
                          TP->TEXTBOX_SIZE, wxTE_PROCESS_ENTER, &radius));
   return wxSize(0, 48);
}
