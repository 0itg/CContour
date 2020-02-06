#include "GridLine.h"

GridLine::GridLine(std::complex<double> c, int t) : type(t)
{
	points.push_back(c);
	color = wxColor(128,128,128);
}

void GridLine::Draw(wxDC* dc, ComplexPlane* canvas)
{
	switch (type)
	{
	case GRIDLINE_HORIZ:
		if (points[0].imag() > canvas->axes.imagMin &&
			points[0].imag() < canvas->axes.imagMax)
		{
			dc->DrawLine(wxPoint(0, canvas->ComplexToScreen(points[0]).y),
				wxPoint(canvas->GetClientSize().x,
					canvas->ComplexToScreen(points[0]).y));
		}
		break;
	case GRIDLINE_VERT:
		if (points[0].real() > canvas->axes.realMin &&
			points[0].real() < canvas->axes.realMax)
		{
			dc->DrawLine(wxPoint(canvas->ComplexToScreen(points[0]).x, 0),
				wxPoint(canvas->ComplexToScreen(points[0]).x,
				canvas->GetClientSize().y));
		}
		break;
	}
}
