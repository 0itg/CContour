#include "Grid.h"
#include "GridLine.h"

Grid::Grid(ComplexPlane* parent, double hStep, double vStep)
{
}

void Grid::Draw(wxDC* dc, ComplexPlane* canvas)
{
	for (auto h : horiz) h->Draw(dc, canvas);
	for (auto v : vert) v->Draw(dc, canvas);
}

std::complex<double> Grid::Interpolate(double t)
{
	return std::complex<double>();
}

ContourPolygon* Grid::Subdivide(int res)
{
	return nullptr;
}
