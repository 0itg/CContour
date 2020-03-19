#include "ContourPolygon.h"
#include "ComplexPlane.h"
#include "Parser.h"
#include <numeric>
#include <thread>

BOOST_CLASS_EXPORT_IMPLEMENT(ContourPolygon)

ContourPolygon::ContourPolygon(cplx c, wxColor col, std::string n) noexcept
{
    points.push_back(c);
    points.push_back(c);
    center = c;
    color  = col;
    name   = n;
}
ContourPolygon::ContourPolygon(wxColor col, std::string n) noexcept
{
    color = col;
    name  = n;
}

void ContourPolygon::Draw(wxDC* dc, ComplexPlane* canvas)
{
    // Create a vector of screen points from the mathematical ones.
    std::vector<wxPoint> screenPoints;
    screenPoints.resize(points.size());
    std::transform(points.begin(), points.end(), screenPoints.begin(),
                   [canvas](cplx z) { return canvas->ComplexToScreen(z); });
    auto screenW = abs(
        canvas->LengthXToScreen(canvas->axes.realMax - canvas->axes.realMin));
    auto screenH = abs(
        canvas->LengthYToScreen(canvas->axes.imagMax - canvas->axes.imagMin));

    // If the distance between two points is greater than the distance across
    // the screen, assume it really should be a discontinuity and skip it.
    for (auto pt = screenPoints.begin(); pt != screenPoints.end() - 1; pt++)
    {
        auto p1 = *pt;
        auto p2 = *(pt + 1);
        if ((abs(p2.x - p1.x) < screenW) && (abs(p2.y - p1.y) < screenH))
            DrawClippedLine(p1, p2, dc, canvas);
    }
    if (closed)
        DrawClippedLine(screenPoints.back(), screenPoints.front(), dc, canvas);
}

bool ContourPolygon::ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked)
{
    Translate(mousePos, lastPointClicked);
    return true;
}

inline bool ContourPolygon::IsDone()
{
    if (closed) return true;
    return abs(points[points.size() - 1] - points[0]) < 0.3;
}

bool ContourPolygon::IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                      int pixPrecision)
{
    // Check each line segment of the polygon until the distance to the point
    // is within pixPrecision. This could probably be made more efficient.
    auto checkDist = [&](cplx pt, int a, int b) {
        auto d = DistancePointToLine(pt, points[a], points[b]);
        return ((d < canvas->ScreenXToLength(pixPrecision) ||
                 d < canvas->ScreenYToLength(pixPrecision)) &&
                IsInsideBox(pt, points[a], points[b]));
    };
    int i;
    for (i = 0; i < points.size() - 1; i++)
    {
        if (checkDist(pt, i + 1, i)) return true;
    }
    // Check the line from last point to first
    if (checkDist(pt, 0, i))
        return true;
    else
        return false;
}

inline void ContourPolygon::Finalize()
{
    // Mark the polygon as closed and pop the last point, because during
    // editing, closing the polygon would make the last point a
    // duplicate of the first.
    if (!closed)
    {
        closed = true;
        points.pop_back();
        markedForRedraw = true;
        CalcCenter();
    }
}

void ContourPolygon::CalcSideLengths()
{
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

cplx ContourPolygon::Interpolate(double t)
{
    t = fmod(t, 1.0);
    if (t < 0) t++;

    int sideIndex          = 0;
    double lengthTraversed = 0;
    CalcSideLengths();
    while (lengthTraversed < t * perimeter)
    {
        lengthTraversed += sideLengths[sideIndex];
        sideIndex++;
    }
    sideIndex--;
    if (sideIndex > -1)
    {
        double sideParam =
            abs(t * perimeter - lengthTraversed) / sideLengths[sideIndex];
        if (sideIndex < points.size() - 1)
            return points[(__int64)sideIndex + 1] * (1 - sideParam) +
                   points[sideIndex] * sideParam;
        else
            return points[0] * (1 - sideParam) + points[sideIndex] * sideParam;
    }
    else
        return points[0];
}

void ContourPolygon::Subdivide(int res)
{
    subDiv.clear();
    subDiv.reserve(res + points.size());
    res           = (int)(std::max(points.size(), res - points.size()));
    int sideIndex = 0;
    double t      = 0;
    CalcSideLengths();
    double lengthTraversed = sideLengths[0];

    // Parameterize the contour in terms of perimeter, e.g. t is the
    // proportion of the contour traversed from the first point.
    for (double i = 0; i <= res; i++)
    {
        t = i / res;
        if (lengthTraversed < t * perimeter)
        {
            // t just passed the edge of one side.
            sideIndex++;
            lengthTraversed += sideLengths[sideIndex];
            subDiv.push_back(
                points[sideIndex]); // add the endpoint of each segment
        }
        // Within one side, linearly interpolate the points
        // such that we get res points in total.
        if (sideIndex < sideLengths.size())
            if (sideLengths[sideIndex] > 0)
            {
                double sideParam = abs(t * perimeter - lengthTraversed) /
                                   sideLengths[sideIndex];
                if (sideIndex < points.size() - 1)
                    subDiv.push_back(points[(__int64)sideIndex + 1] *
                                         (1 - sideParam) +
                                     points[sideIndex] * sideParam);
                else
                    subDiv.push_back(points[0] * (1 - sideParam) +
                                     points[sideIndex] * sideParam);
            }
    }
    // Degenerate polygons may occur but are discarded by the code above.
    // In that case, put in two points so the drawing
    // functions have what they expect.
    if (subDiv.size() == 0)
    {
        subDiv.push_back(points[0]);
        subDiv.push_back(points[0]);
    }
    markedForRedraw = true;
}