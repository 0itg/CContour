#include "ContourPolygon.h"
#include "ComplexPlane.h"
#include <numeric>


ContourPolygon::ContourPolygon(std::complex<double> c)
{
	points.push_back(c);
	points.push_back(c);
	color = wxColor(rand() % 255, rand() % 255, rand() % 255);
}
ContourPolygon::ContourPolygon()
{
	color = wxColor(rand() % 255, rand() % 255, rand() % 255);
}

void ContourPolygon::Draw(wxDC* dc, ComplexPlane* canvas)
{
	std::vector<wxPoint> p;
	p.resize(points.size());
	std::transform(points.begin(), points.end(), p.begin(),
		[canvas](std::complex<double> z) {return canvas->ComplexToScreen(z); });
	if (closed)
		dc->DrawPolygon(p.size(), &p[0]);
	else
		dc->DrawLines(p.size(), &p[0]);
}

void ContourPolygon::ActionNoCtrlPoint(
	std::complex<double> mousePos, std::complex<double> lastPointClicked)
{
	Translate(mousePos, lastPointClicked);
}

bool ContourPolygon::IsClosed() //Name Misleading; fix
{
	if (closed) return true;
	return abs(points[points.size() - 1] - points[0]) < 0.3;
}

bool ContourPolygon::IsOnContour(std::complex<double> pt,
	ComplexPlane* canvas, int pixPrecision)
{
	int i;
	for (i = 0; i < points.size() - 1; i++)
	{
		if (DistancePointToLine(pt, points[i + 1], points[i]) <
			canvas->ScreenToLength(pixPrecision) &&
			IsInsideLine(pt, points[i + 1], points[i]))
			return true;
	}
	// Check the line from last point to first
	if (DistancePointToLine(pt, points[0], points[i]) <
		canvas->ScreenToLength(pixPrecision) &&
		IsInsideLine(pt, points[0], points[i]))
		return true;
	else return false;
}

void ContourPolygon::Finalize()
{
	if (!closed)
	{
		closed = true;
		points.pop_back();
	}
}

void ContourPolygon::CalcSideLengths()
{
	sideLengths.clear();
	for (int i = 0; i < points.size() - 1; i++)
		sideLengths.push_back(abs(points[i + 1] - points[i]));
	if (closed)
		sideLengths.push_back(abs(points[0] - points[points.size()-1]));
	perimeter = std::accumulate(sideLengths.begin(), sideLengths.end(), 0.0);
}

// Interpolates points of the polygon with parameter t in respect to
// arc length, e.g. the path from t = 0 to t = 0.5 would be the same
// length as t = .5 to t = 1.

std::complex<double> ContourPolygon::Interpolate(double t)
{
	int sideIndex = 0;
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
		if (sideIndex < points.size())
			return points[sideIndex + 1] * (1 - sideParam) +
			points[sideIndex] * sideParam;
		else
			return points[0] * (1 - sideParam) +
			points[sideIndex] * sideParam;
	}
	else
		return points[0];
}

ContourPolygon* ContourPolygon::Subdivide(int res)
{
	ContourPolygon* D = new ContourPolygon();
	int sideIndex = 0;
	double t = 0;
	CalcSideLengths();
	double lengthTraversed = sideLengths[0];

	for (int i = 0; i <= res; i++)
	{
		t = (double)i / res;
		if (lengthTraversed < t * perimeter)
		{
			sideIndex++;
			lengthTraversed += sideLengths[sideIndex];
			if (sideIndex < points.size())
				D->AddPoint(points[sideIndex]); // add the endpoint of each segment
		}
		if (sideIndex < sideLengths.size())
			if (sideLengths[sideIndex] > 0)
			{
				double sideParam =
					abs(t * perimeter - lengthTraversed) / sideLengths[sideIndex];
				if (sideIndex < points.size()-1)
					D->AddPoint(points[sideIndex + 1] * (1 - sideParam) +
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
	if (D->points.size() == 0)
	{
		D->AddPoint(points[0]);
		D->AddPoint(points[0]);
	}
	if (closed)
	{
		D->AddPoint(0); // Because ContourPolygon's Finalize() pops last point.
		D->Finalize();
	}
	return D;
}

ContourPolygon* ContourPolygon::Apply(std::function<std::complex<double>(std::complex<double>)> f)
{
	ContourPolygon* C = Clone();
	for (auto& z : C->points)
		z = f(z);
	return C;
}
