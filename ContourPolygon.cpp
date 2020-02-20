#include "ContourPolygon.h"
#include "ComplexPlane.h"
#include "Parser.h"
#include <numeric>

ContourPolygon::ContourPolygon(std::complex<double> c, wxColor col) {
   points.push_back(c);
   points.push_back(c);
   color = col;
}
ContourPolygon::ContourPolygon(wxColor col) {
   color = wxColor(rand() % 255, rand() % 255, rand() % 255);
}

void ContourPolygon::Draw(wxDC* dc, ComplexPlane* canvas) {
   // Create a vector of screen points from the mathematical ones.
   std::vector<wxPoint> screenPoints;
   screenPoints.resize(points.size());
   std::transform(
       points.begin(), points.end(), screenPoints.begin(),
       [canvas](std::complex<double> z) { return canvas->ComplexToScreen(z); });

   for (auto pt = screenPoints.begin(); pt != screenPoints.end() - 1; pt++) {
      DrawClippedLine(*pt, *(pt + 1), dc, canvas);
   }
   if (closed)
      DrawClippedLine(screenPoints.back(), screenPoints.front(), dc, canvas);
}

void ContourPolygon::ActionNoCtrlPoint(std::complex<double> mousePos,
                                       std::complex<double> lastPointClicked) {
   Translate(mousePos, lastPointClicked);
}

bool ContourPolygon::IsDone() {
   if (closed) return true;
   return abs(points[points.size() - 1] - points[0]) < 0.3;
}

bool ContourPolygon::IsOnContour(std::complex<double> pt, ComplexPlane* canvas,
                                 int pixPrecision) {
   // Check each line segment of the polygon until the distance to the point.
   // is within pixPrecision. This could probably be made more efficient.
   int i;
   for (i = 0; i < points.size() - 1; i++) {
      if (DistancePointToLine(pt, points[(__int64)i + 1], points[i]) <
              canvas->ScreenToLength(pixPrecision) &&
          IsInsideBox(pt, points[(__int64)i + 1], points[i]))
         return true;
   }
   // Check the line from last point to first
   if (DistancePointToLine(pt, points[0], points[i]) <
           canvas->ScreenToLength(pixPrecision) &&
       IsInsideBox(pt, points[0], points[i]))
      return true;
   else
      return false;
}

void ContourPolygon::Finalize() {
   // Mark the polygon as closed and pop the last point, because during
   // editing, closing the polygon would make the last point a
   // duplicate of the first.
   if (!closed) {
      closed = true;
      points.pop_back();
   }
}

void ContourPolygon::CalcSideLengths() {
   sideLengths.clear();
   for (int i = 0; i < points.size() - 1; i++)
      sideLengths.push_back(abs(points[(__int64)i + 1] - points[i]));
   if (closed)
      sideLengths.push_back(abs(points[0] - points[points.size() - 1]));
   perimeter = std::accumulate(sideLengths.begin(), sideLengths.end(), 0.0);
}

// Interpolates points of the polygon with parameter t in respect to
// arc length, e.g. the path from t = 0 to t = 0.5 would be the same
// length as t = .5 to t = 1.

std::complex<double> ContourPolygon::Interpolate(double t) {
   int sideIndex          = 0;
   double lengthTraversed = 0;
   CalcSideLengths();
   while (lengthTraversed < t * perimeter) {
      lengthTraversed += sideLengths[sideIndex];
      sideIndex++;
   }
   sideIndex--;
   if (sideIndex > -1) {
      double sideParam =
          abs(t * perimeter - lengthTraversed) / sideLengths[sideIndex];
      if (sideIndex < points.size())
         return points[(__int64)sideIndex + 1] * (1 - sideParam) +
                points[sideIndex] * sideParam;
      else
         return points[0] * (1 - sideParam) + points[sideIndex] * sideParam;
   } else
      return points[0];
}

ContourPolygon* ContourPolygon::Subdivide(int res) {
   ContourPolygon* D = new ContourPolygon();
   res               = (int)(std::max(points.size(), res - points.size()));
   int sideIndex     = 0;
   double t          = 0;
   CalcSideLengths();
   double lengthTraversed = sideLengths[0];

   // Parameterize the contour in terms of perimeter, e.g. t is the
   // proportion of the contour traversed from the first point.
   for (double i = 0; i <= res; i++) {
      t = i / res;
      if (lengthTraversed < t * perimeter) {
         // t just passed the edge of one side.
         sideIndex++;
         lengthTraversed += sideLengths[sideIndex];
         D->AddPoint(points[sideIndex]); // add the endpoint of each segment
      }
      // Within one side, linearly interpolate the points
      // such that we get res points in total.
      if (sideIndex < sideLengths.size())
         if (sideLengths[sideIndex] > 0) {
            double sideParam =
                abs(t * perimeter - lengthTraversed) / sideLengths[sideIndex];
            if (sideIndex < points.size() - 1)
               D->AddPoint(points[(__int64)sideIndex + 1] * (1 - sideParam) +
                           points[sideIndex] * sideParam);
            else
               D->AddPoint(points[0] * (1 - sideParam) +
                           points[sideIndex] * sideParam);
         }
   }
   D->color = color;
   // Degenerate polygons may occur but are discarded by the code above.
   // In that case, put in two points so the drawing
   // functions have what they expect.
   if (D->points.size() == 0) {
      D->AddPoint(points[0]);
      D->AddPoint(points[0]);
   }
   if (closed) {
      D->AddPoint(0); // Because ContourPolygon's Finalize() pops last point.
      D->Finalize();
   }
   return D;
}

ContourPolygon* ContourPolygon::Apply(ParsedFunc<std::complex<double>>& f) {
   ContourPolygon* C = Clone();
   for (auto& z : C->points) z = f(z);
   return C;
}