#include "Grid.h"
#include "ContourLine.h"
#include "ContourPolygon.h"
#include "ComplexPlane.h"

//Grid::Grid(ComplexPlane* parent, double hStep, double vStep)
//{
//}

Grid::~Grid()
{
	for (auto h : horiz) delete h;
	for (auto v : vert) delete v;
}

void Grid::Draw(wxDC* dc, ComplexPlane* canvas)
{
	CalcVisibleGrid();
	for (auto h : horiz) h->Draw(dc, canvas);
	for (auto v : vert) v->Draw(dc, canvas);
}

void Grid::CalcVisibleGrid()
{
	for (auto h : horiz) delete h;
	horiz.clear();
	for (auto v : vert) delete v;
	vert.clear();
	double hOffset = fmod(parent->ScreenToComplex(wxPoint(0,0)).real(), hStep);
	double vOffset = fmod(parent->ScreenToComplex(wxPoint(0,0)).imag(), vStep);
	wxPoint corner(parent->GetClientSize().x, parent->GetClientSize().y);

	for (double y = parent->axes.imagMin - vOffset;
		y < parent->axes.imagMax; y += vStep)
	{
		vert.push_back(new ContourLine(std::complex<double>(parent->
			ScreenToComplex(wxPoint(0, 0)).real(), y),
			std::complex<double>(parent->ScreenToComplex(corner).real(), y)));
	}
	for (double x = parent->axes.realMin - hOffset;
		x < parent->axes.realMax; x += hStep)
	{
		vert.emplace_back(new ContourLine(std::complex<double>(x,
			parent->ScreenToComplex(wxPoint(x,0)).imag()),
			std::complex<double>(x, parent->ScreenToComplex(corner).imag())));
	}
}

//std::complex<double> Grid::Interpolate(double t)
//{
//	return std::complex<double>();
//}
//
//ContourPolygon* Grid::Subdivide(int res)
//{
//	return nullptr;
//}

TransformedGrid::~TransformedGrid()
{
	for (auto h : horiz) delete h;
	for (auto v : vert) delete v;
	Grid::~Grid();
}

void TransformedGrid::Draw(wxDC* dc, ComplexPlane* canvas)
{
	for (auto h : horiz) h->Draw(dc, canvas);
	for (auto v : vert) v->Draw(dc, canvas);
}

void TransformedGrid::CalcVisibleGrid(Grid* grid,
	std::function<std::complex<double>(std::complex<double>)> f)
{
	for (auto h : horiz) delete h;
	horiz.clear();
	for (auto v : vert) delete v;
	vert.clear();
	for (auto h : grid->horiz)
	{
		horiz.push_back(new ContourPolygon());
		double t;
		for (double i = 0; i < res; i++)
		{
			t = i / res;
			horiz.back()->AddPoint(f(h->GetCtrlPoint(0) * t
				+ h->GetCtrlPoint(1) * (1 - t)));
		}
	}
	for (auto v : grid->vert)
	{
		vert.push_back(new ContourPolygon());
		double t;
		for (double i = 0; i <= res; i++)
		{
			t = i / res;
			vert.back()->AddPoint(f(v->GetCtrlPoint(0) * t
				+ v->GetCtrlPoint(1) * (1 - t)));
		}
	}
}
