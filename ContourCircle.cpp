#include "ContourCircle.h"
#include "ContourPolygon.h"

class ComplexPlane;

ContourCircle::ContourCircle(std::complex<double> c, double r) : radius(r)
{
	points.push_back(c);
	color = wxColor(rand() % 255, rand() % 255, rand() % 255);
}

void ContourCircle::Draw(wxDC* dc, ComplexPlane* canvas)
{
	wxPoint p = canvas->ComplexToScreen(points[0]);
	dc->DrawCircle(p, canvas->LengthToScreen(radius));
	DrawCtrlPoint(dc, p);
}

void ContourCircle::moveCtrlPoint(std::complex<double> mousePos, int ptIndex)
{
	points[0] = mousePos;
}

void ContourCircle::ActionNoCtrlPoint(std::complex<double> mousePos,
	std::complex<double> lastPointClicked)
{
	radius = abs(points[0] - mousePos);
}

bool ContourCircle::IsOnContour(std::complex<double> pt,
	ComplexPlane* canvas, int pixPrecision)
{
	return abs(abs(points[0] - pt) - radius) <
		canvas->ScreenToLength(pixPrecision);
}

int ContourCircle::OnCtrlPoint(std::complex<double> pt,
	ComplexPlane* canvas, int pixPrecision)
{
	// returns 1-1=0 if the mouse is on the control point (referring to
	// control point 0. Returns 0-1=-1 if not, with -1
	// meaning no contour found.
	return (abs(points[0] - pt) < canvas->ScreenToLength(pixPrecision)) - 1;
}

std::complex<double> ContourCircle::Interpolate(double t)
{
	return points[0] + std::complex<double>(radius * cos(2 * M_PI * t),
		radius * sin(2 * M_PI * t));
}

ContourPolygon* ContourCircle::Subdivide(int res)
{
	ContourPolygon* D = new ContourPolygon();
	for (int i = 0; i <= res; i++)
	{
		double t = (double)i / res;
		D->AddPoint(Interpolate(t));
	}
	D->color = color;
	return D;
}