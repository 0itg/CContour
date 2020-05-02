#include "ContourLine.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ContourLine)

ContourLine::ContourLine(cplx c, wxColor col, std::string n) noexcept
{
    points.push_back(c);
    points.push_back(c);
    center = c;
    color  = col;
    name   = n;
}

ContourLine::ContourLine(cplx c, cplx d, wxColor col, std::string n) noexcept
{
    points.push_back(c);
    points.push_back(d);
    center = (c + d) / 2.0;
    color  = col;
    name   = n;
}

ContourLine::ContourLine(wxColor col) noexcept
{
    color = wxColor(rand() % 255, rand() % 255, rand() % 255);
    name  = "Line";
}

void ContourLine::Draw(wxDC* dc, ComplexPlane* canvas)
{
    dc->DrawLine(canvas->ComplexToScreen(points[0]),
                 canvas->ComplexToScreen(points[1]));
}

CommandContourTranslate* ContourLine::CreateActionCommand(cplx c)
{
    return new CommandContourTranslate(this, c, c);
}

bool ContourLine::IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                   int pixPrecision)
{
    if (DistancePointToLine(pt, points[0], points[1]) <
            canvas->ScreenXToLength(pixPrecision) &&
        IsInsideBox(pt, points[0], points[1]))
        return true;
    else
        return false;
}

cplx ContourLine::Interpolate(double t)
{
    return points[0] * t + points[1] * (1 - t);
}
