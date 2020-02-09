#include "ContourLine.h"
#include "ContourPolygon.h"
#include "ComplexPlane.h"


ContourLine::ContourLine(std::complex<double> c, wxColor col)
{
	points.push_back(c);
	points.push_back(c);
	color = col;
}

ContourLine::ContourLine(std::complex<double> c,
	std::complex<double> d, wxColor col)
{
	points.push_back(c);
	points.push_back(d);
	color = col;
}

ContourLine::ContourLine(wxColor col)
{
	color = wxColor(rand() % 255, rand() % 255, rand() % 255);
}

void ContourLine::Draw(wxDC* dc, ComplexPlane* canvas)
{
	dc->DrawLine(canvas->ComplexToScreen(points[0]),
		canvas->ComplexToScreen(points[1]));
}

bool ContourLine::IsOnContour(std::complex<double> pt, ComplexPlane* canvas, int pixPrecision)
{
	if (DistancePointToLine(pt, points[0], points[1]) <
		canvas->ScreenToLength(pixPrecision) &&
		IsInsideLine(pt, points[0], points[1]))
		return true;
	else return false;
}

std::complex<double> ContourLine::Interpolate(double t)
{
	return points[0] * t + points[1] * (1 - t);
}

ContourPolygon* ContourLine::Subdivide(int res)
{
	ContourPolygon* D = new ContourPolygon();
	std::complex<double> zStep((points[1].real() - points[0].real()) / res,
		(points[1].imag() - points[0].imag()) / res);
	for (int i = 0; i < res; i++)
		D->AddPoint(points[0] + (double)i * zStep);
	D->color = color;
	return D;
}
