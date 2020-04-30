#include "ContourPoint.h"
#include <wx/tipwin.h>

BOOST_CLASS_EXPORT_IMPLEMENT(ContourPoint)

ContourPoint::ContourPoint(cplx c, wxColor col, std::string n, int ord) noexcept
	: order(ord)
{
	points.push_back(c);
	center = c;
	color = col;
	name = n;
}

void ContourPoint::Draw(wxDC* dc, ComplexPlane* canvas)
{
	// Marks a zero (order > 0) or normal points (order == 0) with a filled dot.
	// Marks a pole (order < 0) with an "X".
	wxDCBrushChanger bc(*dc, wxBrush(color));
	auto pt = canvas->ComplexToScreen(points[0]);
	if (order >= 0)
		dc->DrawCircle(pt, POINT_RADIUS);
	else
	{
		//wxDCPenChanger pc(*dc, wxPen(color, 2));
		auto corner1 = wxPoint(POINT_RADIUS, POINT_RADIUS);
		auto corner2 = wxPoint(POINT_RADIUS, -POINT_RADIUS);
		dc->DrawLine(pt + corner1, pt - corner1);
		dc->DrawLine(pt + corner2, pt - corner2);
	}
	// Indicates the order of higher-order points with radial tick marks
	auto absOrder = abs(order);
	if (absOrder > 1)
	{
		double angleStep = 2 * M_PI / order;
		double angle = -M_PI / 2; // Starts at 12 o'clock.
		for (int i = 0; i < absOrder; i++)
		{
			dc->DrawCircle(pt + wxPoint(10 * cos(angle), 10 * sin(angle)), 1);
			angle += angleStep;
		}
	}
}

void ContourPoint::DrawLabel(wxDC* dc, ComplexPlane* canvas)
{
	auto text = name + "\n" + to_string_X_digits(points[0].real(), 8)
		+ " + " + to_string_X_digits(points[0].imag(), 8) + "i";
	auto location = canvas->ComplexToScreen(points[0]);
	auto rect = wxRect(location - wxPoint(3, 3), location + wxPoint(3, 3));
	canvas->tooltip = new wxTipWindow(canvas, text, 100, &canvas->tooltip,
		&rect);
	//canvas->tooltip->SetBoundingRect(rect);
}

int ContourPoint::ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked)
{
	Translate(mousePos, lastPointClicked);
	return ACTION_TRANSLATE;
}

CommandContourTranslate* ContourPoint::CreateActionCommand(cplx c)
{
	return new CommandContourTranslate(this, c, c);
}

bool ContourPoint::IsPointOnContour(cplx pt, ComplexPlane* canvas,
	int pixPrecision)
{ 
		return abs(points[0] - pt) < std::max(
			canvas->ScreenXToLength(pixPrecision),
			canvas->ScreenYToLength(pixPrecision));
}

Contour* ContourPoint::Map(ParsedFunc<cplx>& f)
{
	return new ContourPoint(f(this->GetCtrlPoint(0)), color);
}
