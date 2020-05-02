#include "Contour.h"
#include "ContourPolygon.h"
#include "ToolPanel.h"
#include <numeric>
#include "LinkedCtrls.h"

void Contour::AddPoint(cplx mousePos)
{
    center *= points.size();
    center += mousePos;
    center /= points.size() + 1;
    points.push_back(mousePos);
}

void Contour::RemovePoint(int index)
{
    auto p = points.begin() + index;
    center *= points.size();
    center -= *p;
    center /= points.size() - 1;
    points.erase(p);
}

int Contour::OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision)
{
    for (int i = 0; i < points.size(); i++)
    {
        auto d = abs(points[i] - pt);
        if (d < canvas->ScreenXToLength(pixPrecision) ||
            d < canvas->ScreenYToLength(pixPrecision))
            return i;
    }
    return -1;
}

cplx Contour::GetCtrlPoint(int index)
{
    return index > -1 ? points[index] : center;
}

void Contour::SetCtrlPoint(int index, cplx c)
{
    center -= points[index] / (double)points.size();
    points[index] = c;
    center += points[index] / (double)points.size();
}

void Contour::PopulateMenu(ToolPanel* TP)
{
    auto panel = TP->intermediate;

    auto sizer = new wxFlexGridSizer(1, 0, 0);
    sizer->SetFlexibleDirection(wxHORIZONTAL);

    auto sizerFlags = wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 3);
    TP->intermediate->SetSizer(sizer);
    wxFont normalFont = TP->intermediate->GetFont();

    panel->SetFont(normalFont.Bold());
    TP->AddwxCtrl(new wxStaticText(panel, wxID_ANY, wxString(GetName() + ":"),
                                   wxDefaultPosition, wxDefaultSize));
    panel->SetFont(normalFont);
    sizer->Add(TP->GetwxCtrl(0), sizerFlags);

    std::tuple<int, int, int> sup = PopulateSupplementalMenu(TP);

    int ptCount = GetPointCount();
    for (int i = 0; i < ptCount; i++)
    {
        std::string c = std::to_string(GetCtrlPoint(i).real()) + " + " +
                        std::to_string(GetCtrlPoint(i).imag()) + "i";
        TP->AddwxCtrl(new wxStaticText(
            panel, wxID_ANY, wxString("Ctrl Point " + std::to_string(i)),
            wxDefaultPosition, wxDefaultSize));
        TP->AddLinkedCtrl(new LinkedCtrlPointTextCtrl(
            panel, wxID_ANY, c, wxDefaultPosition, wxDefaultSize,
            wxTE_PROCESS_ENTER, this, (size_t)i, TP->GetHistoryPtr()));

        sizer->Add(TP->GetwxCtrl(i + 1 + std::get<0>(sup)), sizerFlags);
        sizer->Add(TP->GetLinkedCtrl(i + std::get<1>(sup))->GetCtrlPtr(),
                   sizerFlags);
    }
    auto IsPathChkbox = new LinkedCheckBox(panel, "Hide from output",
                                           &isPathOnly, TP->GetHistoryPtr());
    TP->AddLinkedCtrl(IsPathChkbox);
    sizer->Add(IsPathChkbox->GetCtrlPtr(), sizerFlags);

    sizer->AddGrowableCol(0, 1);
    TP->FitInside();
}

Contour* Contour::Map(ParsedFunc<cplx>& f, int res)
{
    ContourPolygon* C = new ContourPolygon(color, "f(" + name + ")");
    C->Reserve(res + 1);
    double tStep = 1.0 / res;
    const double TOL = 1e-9;
    for (double t = 0; t < 1.0 + TOL; t += tStep)
    {
        C->AddPoint(f(Interpolate(t)));
    }
    return C;
}

void Contour::DrawCtrlPoint(wxDC* dc, wxPoint p)
{
    wxPen pen = dc->GetPen();
    pen.SetWidth(2);
    wxDCPenChanger temp(*dc, pen);

    dc->DrawCircle(p, 1);
}

cplx Contour::CalcCenter()
{
    if (points.size())
    {
        center = std::accumulate(points.begin(), points.end(), cplx(0));
        center /= points.size();
    }
    return center;
}

void Contour::Translate(cplx z2, cplx z1)
{
    cplx displacement = z2 - z1;
    for (auto& p : points)
    {
        p += displacement;
    }
    center += displacement;
}

void Contour::Rotate(double angle, cplx pivot)
{
    RotateAndScale(exp(cplx(0, 1) * angle), pivot);
}

void Contour::Scale(double factor, cplx pivot)
{
    RotateAndScale(factor, pivot);
}

void Contour::RotateAndScale(cplx V, cplx pivot)
{
    if (pivot == cplx(INFINITY, INFINITY)) pivot = center;
    Translate(0, pivot);
    for (auto& z : points)
        z *= V;
    center *= V;
    Translate(pivot, 0);
}

void Contour::Rotate(cplx V, cplx pivot) { RotateAndScale(V / abs(V), pivot); }
