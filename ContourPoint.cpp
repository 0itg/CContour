#include "ContourPoint.h"

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
	wxDCBrushChanger bc(*dc, wxBrush(color));
	dc->DrawCircle(canvas->ComplexToScreen(points[0]), POINT_RADIUS);
	//TODO: visually indicate order of point, particularly zero vs. pole 
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
		return abs(abs(points[0] - pt)) < std::max(
			canvas->ScreenXToLength(pixPrecision),
			canvas->ScreenYToLength(pixPrecision));
}

Contour* ContourPoint::Map(ParsedFunc<cplx>& f)
{
	return new ContourPoint(f(this->GetCtrlPoint(0)), color);
}
