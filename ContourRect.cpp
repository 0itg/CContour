#include "ContourRect.h"

ContourRect::ContourRect(std::complex<double> c) : ContourPolygon(c)
{
	// Two corner points define the rectangle, but adding the other two
	// points allows all corners to be used for resizing it
	points.push_back(c);
	points.push_back(c);
	closed = true;
}

void ContourRect::Draw(wxDC* dc, ComplexPlane* canvas, windowSettings axes)
{
	int width = canvas->LengthToScreen(points[2].real() - points[0].real());
	int height = canvas->LengthToScreen(points[2].imag() - points[0].imag());
	dc->DrawRectangle(canvas->
		ComplexToScreen(points[0]), wxSize(width, height));
}

void ContourRect::moveCtrlPoint(std::complex<double> mousePos, int ptIndex)
{
	points[ptIndex] = mousePos;
	// When moving one point, automatically move the adjacent points to
	// maintain rectangular shape.
	if (ptIndex % 2)
	{
		points[2] = std::complex<double>(points[3].real(), points[1].imag());
		points[0] = std::complex<double>(points[1].real(), points[3].imag());
	}
	else
	{
		points[1] = std::complex<double>(points[0].real(), points[2].imag());
		points[3] = std::complex<double>(points[2].real(), points[0].imag());
	}
}