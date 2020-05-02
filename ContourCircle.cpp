#include "ContourCircle.h"
#include "ToolPanel.h"
#include "LinkedCtrls.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ContourCircle)

ContourCircle::ContourCircle(cplx c, double r, wxColor col,
                             std::string n) noexcept
    : radius(r)
{
    points.push_back(c);
    center = c;
    color  = col;
    name   = n;
}

void ContourCircle::Draw(wxDC* dc, ComplexPlane* canvas)
{
    wxPoint p = canvas->ComplexToScreen(points[0]);
    auto r1   = (wxCoord)canvas->LengthXToScreen(radius);
    auto r2   = (wxCoord)canvas->LengthYToScreen(radius);
    dc->DrawEllipse(p.x - r1, p.y - r2, 2 * r1, 2 * r2);
    DrawCtrlPoint(dc, p);
}

int ContourCircle::ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked)
{
    radius = abs(points[0] - mousePos);
    return ACTION_EDIT_RADIUS;
}

CommandContourEditRadius* ContourCircle::CreateActionCommand(cplx c)
{
    return new CommandContourEditRadius(this, radius);
}

bool ContourCircle::IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                     int pixPrecision)
{
    return abs(abs(points[0] - pt) - radius) < std::max(
           canvas->ScreenXToLength(pixPrecision),
           canvas->ScreenYToLength(pixPrecision));
}

int ContourCircle::OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision)
{
    // returns 1-1=0 if the mouse is on the control point (referring to
    // control point 0. Returns 0-1=-1 if not, with -1
    // meaning no contour found.
    return (abs(points[0] - pt) < canvas->ScreenXToLength(pixPrecision)) - 1;
}

cplx ContourCircle::Interpolate(double t)
{
    return points[0] +
           cplx(radius * cos(2 * M_PI * t), radius * sin(2 * M_PI * t));
}

void ContourCircle::RotateAndScale(cplx V, cplx pivot)
{
    Contour::RotateAndScale(V, pivot);
    radius *= abs(V);
}

std::tuple<int, int, int> ContourCircle::PopulateSupplementalMenu(ToolPanel* TP)
{
    auto RadiusText =
        new wxStaticText(TP->intermediate, wxID_ANY, wxString("Radius: "),
                         wxDefaultPosition, wxDefaultSize);
    auto RadiusCtrl = new LinkedRadiusCtrl(
        TP->intermediate, wxID_ANY, wxString(std::to_string(radius)),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, this,
        TP->GetHistoryPtr(), &radius);
    auto sizerFlags = wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 3);
    TP->AddwxCtrl(RadiusText);
    TP->AddLinkedCtrl(RadiusCtrl);
    auto sizer = TP->intermediate->GetSizer();
    sizer->Add(RadiusText, sizerFlags);
    sizer->Add(RadiusCtrl->GetCtrlPtr(), sizerFlags);
    return std::make_tuple(1, 1, 2 * TP->ROW_HEIGHT);
}
