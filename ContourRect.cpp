#include "ContourRect.h"
#include "ComplexPlane.h"
#include "ToolPanel.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ContourRect)

ContourRect::ContourRect(cplx c, wxColor col, std::string n) noexcept
    : ContourPolygon(c, col)
{
    // Two corner points define the rectangle, but adding the other two
    // points allows all corners to be used for resizing it
    points.push_back(c);
    points.push_back(c);
    closed = true;
    name   = n;
}

void ContourRect::Draw(wxDC* dc, ComplexPlane* canvas)
{
    int width =
        (int)canvas->LengthXToScreen(points[2].real() - points[0].real());
    int height =
        (int)-canvas->LengthYToScreen(points[2].imag() - points[0].imag());
    dc->DrawRectangle(canvas->ComplexToScreen(points[0]),
                      wxSize(width, height));
}

void ContourRect::RemovePoint(int index)
{
    points.clear(); // Removing a point means it's no longer a rectangle
}

void ContourRect::moveCtrlPoint(cplx mousePos, int ptIndex)
{
    points[ptIndex] = mousePos;
    // When moving one point, automatically move the adjacent points to
    // maintain rectangular shape.
    if (ptIndex % 2)
    {
        points[2] = cplx(points[3].real(), points[1].imag());
        points[0] = cplx(points[1].real(), points[3].imag());
    }
    else
    {
        points[1] = cplx(points[0].real(), points[2].imag());
        points[3] = cplx(points[2].real(), points[0].imag());
    }
}
