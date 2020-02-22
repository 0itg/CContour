#include "ComplexPlane.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include "Grid.h"
#include "OutputPlane.h"

#include "Event_IDs.h"

#include <complex>
#include <iomanip>
#include <sstream>

ComplexPlane::ComplexPlane(wxWindow* parent, std::string n)
    : axes(this), wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                          wxFULL_REPAINT_ON_RESIZE),
      resCtrl(nullptr), statBar(nullptr), toolPanel(nullptr), name(n) {
   SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

ComplexPlane::~ComplexPlane() {
   for (auto C : contours) delete C;
}

std::complex<double> ComplexPlane::ScreenToComplex(wxPoint P) {
   return std::complex<double>(
       (double)P.x / GetClientSize().x * (axes.realMax - axes.realMin) +
           axes.realMin,
       (1 - (double)P.y / GetClientSize().y) * (axes.imagMax - axes.imagMin) +
           axes.imagMin);
}

wxPoint ComplexPlane::ComplexToScreen(std::complex<double> C) {
   return wxPoint((C.real() - axes.realMin) / (axes.realMax - axes.realMin) *
                      GetClientSize().x,
                  (axes.imagMin - C.imag()) / (axes.imagMax - axes.imagMin) *
                          GetClientSize().y +
                      GetClientSize().y);
}

double ComplexPlane::LengthToScreen(double r) {
   return r * GetClientSize().x / (axes.realMax - axes.realMin);
}

double ComplexPlane::ScreenToLength(double r) {
   return r * (axes.realMax - axes.realMin) / GetClientSize().x;
}

void ComplexPlane::OnMouseWheel(wxMouseEvent& mouse) {
   int rot = mouse.GetWheelRotation() / mouse.GetWheelDelta();
   Zoom(mouse.GetPosition(), rot);
}

void ComplexPlane::OnMouseRightUp(wxMouseEvent& mouse) {
   ReleaseMouseIfAble();
   panning = false;
}

void ComplexPlane::OnMouseRightDown(wxMouseEvent& mouse) {
   CaptureMouseIfAble();
   panning = true;
}

void ComplexPlane::OnMouseCapLost(wxMouseCaptureLostEvent& mouse) {
   ReleaseMouseIfAble();
   panning = false;
}

void ComplexPlane::OnMouseLeaving(wxMouseEvent& mouse) {
   statBar->SetStatusText("", 0);
   statBar->SetStatusText("", 1);
}

void ComplexPlane::OnShowAxes_ShowGrid(wxCommandEvent& event) {
   switch (event.GetId()) {
   case ID_Show_Axes:
      showAxes = !showAxes;
      break;
   case ID_Show_Grid:
      showGrid = !showGrid;
   }
   Refresh();
   Update();
}

void ComplexPlane::Highlight(wxPoint mousePos) {
   bool notOnAnyContour = true;
   int lastHC           = highlightedContour;
   int lastHCP          = highlightedCtrlPoint;

   for (int i = 0; i < contours.size(); i++) {
      int CtrlPtIndex =
          contours[i]->OnCtrlPoint(ScreenToComplex(mousePos), this);
      if (CtrlPtIndex > -1) {
         notOnAnyContour      = false;
         highlightedCtrlPoint = CtrlPtIndex;
         highlightedContour   = i;
      } else if (contours[i]->IsPointOnContour(ScreenToComplex(mousePos), this)) {
         notOnAnyContour      = false;
         highlightedContour   = i;
         highlightedCtrlPoint = -1;
      }
   }
   // Unhighlight the previously highlighted contour if the mouse
   // is not over one anymore.
   if (highlightedContour > -1 && notOnAnyContour) {
      highlightedContour   = -1;
      highlightedCtrlPoint = -1;
   }
   // Only update the screen if different things are highlighted.
   // Theoretically more efficient.
   if (highlightedContour != lastHC || highlightedCtrlPoint != lastHCP) {
      Refresh();
      Update();
   }
}

void ComplexPlane::Pan(wxPoint mousePos) {
   std::complex<double> displacement = lastMousePos - ScreenToComplex(mousePos);
   axes.realMax += displacement.real();
   axes.realMin += displacement.real();
   axes.imagMax += displacement.imag();
   axes.imagMin += displacement.imag();
   movedViewPort = true;
   Refresh();
   Update();
}

// void ComplexPlane::InversePan(wxPoint mousePos)
//{
//    std::complex<double> displacement =
//        lastMousePos - ScreenToComplex(mousePos);
//    axes.realMax -= displacement.real();
//    axes.realMin -= displacement.real();
//    axes.imagMax -= displacement.imag();
//    axes.imagMin -= displacement.imag();
//    Refresh();
//    Update();
//}

void ComplexPlane::Zoom(wxPoint mousePos, int zoomSteps) {
   std::complex<double> zoomCenter = ScreenToComplex(mousePos);
   // Zoom around the mouse position. To this, first translate the viewport
   // so mousePos is at the origin, then apply the zoom, then translate back.
   axes.realMax -= zoomCenter.real();
   axes.realMin -= zoomCenter.real();
   axes.imagMax -= zoomCenter.imag();
   axes.imagMin -= zoomCenter.imag();

   for (int i = 0; i < 4; i++) { axes.c[i] *= pow(zoomFactor, zoomSteps); }

   axes.realMax += zoomCenter.real();
   axes.realMin += zoomCenter.real();
   axes.imagMax += zoomCenter.imag();
   axes.imagMin += zoomCenter.imag();

   // If the user zooms in or symbolStack too far, the tick marks will get too
   // far apart or too close together. Rescale when they are more than twice
   // as far apart or half as far apart.

   int MaxMark       = GetClientSize().x / (axes.TARGET_TICK_COUNT / 2);
   const int MinMark = GetClientSize().x / (axes.TARGET_TICK_COUNT * 2);

   if (LengthToScreen(axes.reStep) < MinMark) {
      axes.reStep *= 2;
   } else if (LengthToScreen(axes.reStep) > MaxMark) {
      axes.reStep /= 2;
   }
   if (LengthToScreen(axes.imStep) < MinMark) {
      axes.imStep *= 2;
   } else if (LengthToScreen(axes.imStep) > MaxMark) {
      axes.imStep /= 2;
   }
   movedViewPort = true;
   Refresh();
   Update();
}

void Axes::Draw(wxDC* dc) {
   using namespace std::complex_literals;
   wxPoint center = parent->ComplexToScreen(0);
   wxPen pen(2);
   pen.SetColour(*wxBLACK);
   dc->SetPen(pen);
   // CrossHair = axes through the given point
   dc->CrossHair(center);

   wxFont font = wxFont(wxFontInfo(6));

   // Draw the tick marks on the axes.
   wxSize size = parent->GetClientSize();
   wxPoint mark;

   // Get offset and adjust tick mark count so text aligns with zero.
   int count                  = realMin / reStep;
   std::complex<double> cMark = realMin - fmod(realMin, reStep);

   while (cMark.real() < realMax) {
      count++;
      cMark += reStep;
      mark = parent->ComplexToScreen(cMark);

      wxSize textOffset = font.GetPixelSize();
      int textTopEdge   = -TICK_WIDTH;
      int textBottomEdge =
          parent->GetClientSize().y - TICK_WIDTH - font.GetPixelSize().y;

      // Draw the labels near the axis if it's on screen. Otherwise, draw
      // them at the top or bottom, depending on which side is closer
      // to the axis.
      if (center.y > 0 && center.y < parent->GetClientSize().y) {
         dc->DrawLine(mark.x, mark.y + TICK_WIDTH / 2, mark.x,
                      mark.y - TICK_WIDTH / 2);
      }
      if (center.y <= textTopEdge + LABEL_PADDING) {
         mark.y = textTopEdge + LABEL_PADDING;
      } else if (center.y > textBottomEdge - LABEL_PADDING) {
         mark.y = textBottomEdge - LABEL_PADDING;
      }

      if (count % LABEL_SPACING == 0 && count != 0) {
         std::ostringstream oss;
         oss << std::setprecision(4) << std::noshowpoint << cMark.real();
         std::string label = oss.str();
         dc->DrawText(label, mark.x - parent->GetTextExtent(label).x / 2,
                      mark.y + TICK_WIDTH / 2 + 1);
      }
   }

   // Treat the vertical axis similarly.

   count = imagMin / imStep;
   cMark = (imagMin - fmod(imagMin, imStep)) * 1i;

   while (cMark.imag() < imagMax) {
      count++;
      cMark += imStep * 1i;
      mark = parent->ComplexToScreen(cMark);
      std::ostringstream oss;
      oss << std::setprecision(4) << std::noshowpoint << cMark.imag() << 'i';
      std::string label = oss.str();
      wxSize textOffset = dc->GetTextExtent(label);

      int textLeftEdge  = textOffset.x + TICK_WIDTH;
      int textRightEdge = parent->GetClientSize().x + TICK_WIDTH;

      if (center.x > 0 && center.x < parent->GetClientSize().x) {
         dc->DrawLine(wxPoint(mark.x + TICK_WIDTH / 2, mark.y),
                      wxPoint(mark.x - TICK_WIDTH / 2, mark.y));
      }
      if (center.x <= textLeftEdge + LABEL_PADDING) {
         mark.x = textLeftEdge + LABEL_PADDING;
      } else if (center.x > textRightEdge - LABEL_PADDING) {
         mark.x = textRightEdge - LABEL_PADDING;
      }

      if (count % LABEL_SPACING == 0 && count != 0) {
         dc->DrawText(label, mark.x - textLeftEdge, mark.y - textOffset.y / 2);
      }
   }
}
