#include "Contour.h"
#include "ComplexPlane.h"
#include "LinkedTextCtrl.h"
#include "ToolPanel.h"
#include <algorithm>
#include <numeric>

void Contour::AddPoint(cplx mousePos) {
   points.push_back(mousePos);
}

void Contour::RemovePoint(int index) {
   points.erase(points.begin() + index);
}

int Contour::OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision) {
   for (int i = 0; i < points.size(); i++) {
      if ((abs(points[i] - pt) < canvas->ScreenToLength(pixPrecision)))
         return i;
   }
   return -1;
}

cplx Contour::GetCtrlPoint(int index) {
   return points[index];
}

void Contour::SetCtrlPoint(int index, cplx c) {
   points[index] = c;
}

void Contour::PopulateMenu(ToolPanel* TP) {
   auto sizer      = new wxBoxSizer(wxVERTICAL);
   auto sizerFlags = wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 3);
   TP->intermediate->SetSizer(sizer);
   TP->AddDecoration(new wxStaticText(TP->intermediate, wxID_ANY,
                                      wxString(GetName() + ":"),
                                      wxDefaultPosition, wxDefaultSize));
   sizer->Add(TP->GetDecoration(0), sizerFlags);

   std::tuple<int, int, int> sup = PopulateSupplementalMenu(TP);

       int ptCount = GetPointCount();
   for (int i = 0; i < ptCount; i++) {
      std::string c = std::to_string(GetCtrlPoint(i).real()) + " + " +
                      std::to_string(GetCtrlPoint(i).imag()) + "i";
      TP->AddDecoration(
          new wxStaticText(TP->intermediate, wxID_ANY,
                           wxString("Ctrl Point " + std::to_string(i)),
                           wxDefaultPosition, TP->TEXTBOX_SIZE));
      TP->AddLinkedCtrl(new LinkedCtrlPointTextCtrl(
          TP->intermediate, wxID_ANY, c, wxDefaultPosition, TP->TEXTBOX_SIZE,
          wxTE_PROCESS_ENTER, this, (size_t)i));
      sizer->Add(TP->GetDecoration(i + 1 + std::get<0>(sup)), sizerFlags);
      sizer->Add(TP->GetLinkedCtrl(i + std::get<1>(sup))->GetCtrlPtr(),
                 sizerFlags);
   }
   // TP->intermediate->Layout();
   // TP->intermediate->SetMinClientSize(wxSize(TP->GetClientSize().x, -1));
   auto panel = TP->intermediate;
   panel->Layout();
   panel->SetMinClientSize(
       wxSize(TP->GetClientSize().x, 48 * ptCount + std::get<2>(sup)));
   panel->SetMaxClientSize(
       wxSize(-1, 48 * ptCount + std::get<2>(sup)));
   panel->SetVirtualSize(
       wxSize(-1, 48 * ptCount + std::get<2>(sup)));

   panel->Fit();
}

void Contour::DrawCtrlPoint(wxDC* dc, wxPoint p) {
   wxPen pen = dc->GetPen();
   pen.SetWidth(2);
   wxDCPenChanger temp(*dc, pen);

   // Draws a sort of asterisk on the point. Didn't like it much.
   /*dc->DrawLine(p + wxPoint(3, 2), p - wxPoint(3, 2));
   dc->DrawLine(p + wxPoint(-3, 2), p - wxPoint(-3, 2));
   dc->DrawLine(p + wxPoint(0, 3), p - wxPoint(0, 3));*/
   dc->DrawCircle(p, 1);
}

void Contour::moveCtrlPoint(cplx mousePos, int ptIndex) {
   points[ptIndex] = mousePos;
}

void Contour::Translate(cplx z2, cplx z1) {
   cplx displacement = z2 - z1;
   for (auto& p : points) {
      p += displacement;
   }
}

double DistancePointToLine(cplx pt, cplx z1, cplx z2) {
   double n = abs((z2.imag() - z1.imag()) * pt.real() -
                  (z2.real() - z1.real()) * pt.imag() + z2.real() * z1.imag() -
                  z1.real() * z2.imag());
   double d = abs(z2 - z1);
   if (d != 0)
      return n / d;
   else
      return abs(z2 - pt);
};

bool IsInsideBox(cplx pt, cplx z1, cplx z2) {
   double Dz1z2 = abs(z2 - z1);
   double Dz1pt = abs(pt - z1);
   double Dz2pt = abs(pt - z2);
   return (Dz1pt < Dz1z2 && Dz2pt < Dz1z2);
};

// Drawing too far outside the screen incurs an unacceptable performance
// cost, so if necessary this can be used to
// draw lines clipped to panel boundaries.

void DrawClippedLine(wxPoint p1, wxPoint p2, wxDC* dc, ComplexPlane* canvas) {
   const int PADDING = 4;
   wxPoint size(canvas->GetClientSize().x + PADDING,
                canvas->GetClientSize().y + PADDING);

   auto IsLineOffscreen = [&](wxPoint p1, wxPoint p2) {
      return ((p1.x < -PADDING && p2.x < -PADDING) ||
              (p1.y < -PADDING && p2.y < -PADDING) ||
              (p1.x > size.x && p2.x > size.x) ||
              (p1.y > size.y && p2.y > size.y));
   };

   if (!IsLineOffscreen(p1, p2)) {
      double dx         = p2.x - p1.x;
      double dy         = p2.y - p1.y;
      double slope      = NAN;
      double slopeRecip = NAN;

      if (dx) {
         slope = dy / dx;
         if (slope)
            slopeRecip = 1.0 / slope;
      }

      for (auto p : {p1, p2}) {
         if (!isnan(slope)) {
            if (p.x < -PADDING) {
               p.y += slope * (-PADDING - p.x);
               p.x = -PADDING;
            } else if (p.x > size.x) {
               p.y += slope * (p.x - size.x);
               p.x = size.x;
            }
         } else {
            if (p.y < -PADDING)
               p.y = -PADDING;
            else if (p.y > size.y)
               p.y = size.y;
         }
         if (!isnan(slopeRecip)) {
            if (p.y < -PADDING) {
               p.x += slopeRecip * (-PADDING - p.y);
               p.y = -PADDING;
            } else if (p.y > size.y) {
               p.x += slopeRecip * (p.y - size.y);
               p.y = size.y;
            }
         } else {
            if (p.x < -PADDING)
               p.x = -PADDING;
            else if (p.x > size.x)
               p.x = size.x;
         }
      }
   }
   if (!IsLineOffscreen(p1, p2)) {
      dc->DrawLine(p1, p2);
   }
}