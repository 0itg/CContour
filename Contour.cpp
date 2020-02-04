#include "Contour.h"
#include <numeric>
#include <algorithm>

void Contour::AddPoint(std::complex<double> mousePos)
{
	points.push_back(mousePos);
}

int Contour::OnCtrlPoint(std::complex<double> pt,
	ComplexPlane* canvas, int pixPrecision)
{
	for (int i = 0; i < points.size(); i++)
	{
		if ((abs(points[i] - pt) < canvas->ScreenToLength(pixPrecision)))
			return i;
	}
	return -1;
}

std::complex<double> Contour::GetCtrlPoint(int index)
{
	return points[index];
}

void Contour::DrawCtrlPoint(wxDC* dc, wxPoint p)
{
	wxPen pen = dc->GetPen();
	pen.SetWidth(2);
	wxDCPenChanger temp(*dc, pen);

	/*dc->DrawLine(p + wxPoint(3, 2), p - wxPoint(3, 2));
	dc->DrawLine(p + wxPoint(-3, 2), p - wxPoint(-3, 2));
	dc->DrawLine(p + wxPoint(0, 3), p - wxPoint(0, 3));*/
	dc->DrawCircle(p, 1);
}

void Contour::moveCtrlPoint(std::complex<double> mousePos, int ptIndex)
{
	points[ptIndex] = mousePos;
}

void Contour::Translate(std::complex<double> z2, std::complex<double> z1)
{
	std::complex<double> displacement = z2 - z1;
	for (auto& p : points)
	{
		p += displacement;
	}
}