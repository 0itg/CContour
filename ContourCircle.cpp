#include "ContourCircle.h"
#include "ComplexPlane.h"
#include "ContourPolygon.h"
#include "LinkedTextCtrl.h"
#include "ToolPanel.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ContourCircle)

ContourCircle::ContourCircle(cplx c, double r, wxColor col,
                             std::string n) noexcept
    : radius(r)
{
    points.push_back(c);
    center = c;
    color = col;
    name  = n;
}

void ContourCircle::Draw(wxDC* dc, ComplexPlane* canvas)
{
    wxPoint p = canvas->ComplexToScreen(points[0]);
    auto r1   = (wxCoord)canvas->LengthXToScreen(radius);
    auto r2   = (wxCoord)canvas->LengthYToScreen(radius);
    dc->DrawEllipse(p.x - r1, p.y - r2, 2 * r1, 2 * r2);
    DrawCtrlPoint(dc, p);
}

bool ContourCircle::ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked)
{
    radius = abs(points[0] - mousePos);
    return true;
}

bool ContourCircle::IsPointOnContour(cplx pt, ComplexPlane* canvas,
                                     int pixPrecision)
{
    return abs(abs(points[0] - pt) - radius) <
           canvas->ScreenXToLength(pixPrecision);
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

void ContourCircle::Subdivide(int res)
{
    subDiv.clear();
    subDiv.reserve(res);
    for (int i = 0; i <= res; i++)
    {
        double t = (double)i / res;
        subDiv.push_back(Interpolate(t));
    }
   markedForRedraw = true;
}

std::tuple<int, int, int> ContourCircle::PopulateSupplementalMenu(ToolPanel* TP)
{
    auto RadiusText =
        new wxStaticText(TP->intermediate, wxID_ANY, wxString("Radius: "),
                         wxDefaultPosition, TP->TEXTBOX_SIZE);
    auto RadiusCtrl = new LinkedDoubleTextCtrl(
        TP->intermediate, wxID_ANY, wxString(std::to_string(radius)),
        wxDefaultPosition, TP->TEXTBOX_SIZE, wxTE_PROCESS_ENTER, &radius);
    auto sizerFlags = wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 3);
    TP->AddDecoration(RadiusText);
    TP->AddLinkedCtrl(RadiusCtrl);
    auto sizer = TP->intermediate->GetSizer();
    sizer->Add(RadiusText, sizerFlags);
    sizer->Add(RadiusCtrl->GetCtrlPtr(), sizerFlags);
    return std::make_tuple(1, 1, 2 * TP->ROW_HEIGHT);
}
