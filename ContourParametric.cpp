#include "ContourParametric.h"
#include "ToolPanel.h"
#include "LinkedCtrls.h"

BOOST_CLASS_EXPORT_IMPLEMENT(ContourParametric)

ContourParametric::ContourParametric(std::string func, int res, wxColor col,
                                     std::string n, double tS, double tE)
    : tStart(tS), tEnd(tE)
{
    color = col;
    name  = n;
    f     = parser.Parse(func);
    f.SetIV("t");
}

void ContourParametric::Draw(wxDC* dc, ComplexPlane* canvas)
{
    double tStep = 1.0 / canvas->GetRes();
    const double TOL = 1e-9;
    double tLast = tStart;
    for (double t = tStart + tStep; t < tEnd + TOL; t += tStep)
    {
        DrawClippedLine(canvas->ComplexToScreen(f(tLast)),
            canvas->ComplexToScreen(f(t)), dc, canvas);
        tLast = t;
    }
}

bool ContourParametric::IsPointOnContour(cplx pt, ComplexPlane* canvas, int pixPrecision)
{
    auto checkDist = [&](cplx pt, double t1, double t2) {
        auto pt1 = f(t1);
        auto pt2 = f(t2);
        auto d = DistancePointToLine(pt, pt1, pt2);
        return ((d < canvas->ScreenXToLength(pixPrecision) ||
            d < canvas->ScreenYToLength(pixPrecision)) &&
            IsInsideBox(pt, pt1, pt2));
    };

    double tStep = 1.0 / canvas->GetRes();
    const double TOL = 1e-9;
    double tLast = tStart;
    for (double t = tStart + tStep; t < tEnd + TOL; t += tStep)
    {
        if (checkDist(pt, tLast, t))
            return true;
        tLast = t;
    }
    return false;
}

cplx ContourParametric::Interpolate(double t)
{
    return f(t * tEnd + (1 - t) * tStart);
}

void ContourParametric::PopulateMenu(ToolPanel* TP)
{
    auto panel = TP->intermediate;
    auto sizer = new wxFlexGridSizer(1, 0, 0);
    sizer->SetFlexibleDirection(wxHORIZONTAL);
    auto sizerFlags = wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 3);
    panel->SetSizer(sizer);
    wxFont normalFont = panel->GetFont();

    panel->SetFont(normalFont.Bold());
    TP->AddwxCtrl(new wxStaticText(panel, wxID_ANY, wxString(GetName() + ":"),
                                   wxDefaultPosition, wxDefaultSize));
    panel->SetFont(normalFont);
    sizer->Add(TP->GetwxCtrl(0), sizerFlags);

    auto fnEdit = new LinkedParametricFuncCtrl(TP, wxID_ANY, f.GetInputText(),
                                               wxDefaultPosition, wxDefaultSize,
                                               wxTE_PROCESS_ENTER, this);
    TP->AddLinkedCtrl(fnEdit);
    sizer->Add(fnEdit->GetCtrlPtr(), sizerFlags);

    auto tStartLabel = new wxStaticText(panel, wxID_ANY, wxString("t start"),
                                        wxDefaultPosition, wxDefaultSize);
    TP->AddwxCtrl(tStartLabel);
    sizer->Add(tStartLabel, sizerFlags);

    auto tStartTextBox = new LinkedContourParamCtrl(
        panel, wxID_ANY, wxString(std::to_string(tStart)), wxDefaultPosition,
        wxDefaultSize, wxTE_PROCESS_ENTER, &tStart, this);
    TP->AddLinkedCtrl(tStartTextBox);
    sizer->Add(tStartTextBox->GetCtrlPtr(), sizerFlags);

    auto tEndLabel = new wxStaticText(panel, wxID_ANY, wxString("t start"),
                                      wxDefaultPosition, wxDefaultSize);
    TP->AddwxCtrl(tEndLabel);
    sizer->Add(tEndLabel, sizerFlags);

    auto tEndTextBox = new LinkedContourParamCtrl(
        panel, wxID_ANY, wxString(std::to_string(tEnd)), wxDefaultPosition,
        wxDefaultSize, wxTE_PROCESS_ENTER, &tEnd, this);
    TP->AddLinkedCtrl(tEndTextBox);
    sizer->Add(tEndTextBox->GetCtrlPtr(), sizerFlags);

    int rowCount = 6;

    std::vector<Symbol<cplx>*> vars = f.GetVars();
    rowCount += 2 * (vars.size() - 1);

    for (auto v : vars)
    {
        if (v->GetToken() == f.GetIV()) continue;
        cplx val      = v->eval().GetVal();
        std::string c = std::to_string(val.real()) + " + " +
                        std::to_string(val.imag()) + "i";
        auto vName = new wxStaticText(TP->intermediate, wxID_ANY,
                                      wxString(v->GetToken()),
                                      wxDefaultPosition, wxDefaultSize);
        TP->AddwxCtrl(vName);
        auto vTextBox = new LinkedVarTextCtrl(
            TP->intermediate, wxID_ANY, c, wxDefaultPosition, wxDefaultSize,
            wxTE_PROCESS_ENTER, v, TP->GetHistoryPtr());
        TP->AddLinkedCtrl(vTextBox);
        sizer->Add(vName, sizerFlags);
        sizer->Add(vTextBox->GetCtrlPtr(), sizerFlags);
    }

    auto IsPathChkbox = new LinkedCheckBox(panel, "Hide from output",
        &isPathOnly, TP->GetHistoryPtr());
    TP->AddLinkedCtrl(IsPathChkbox);
    sizer->Add(IsPathChkbox->GetCtrlPtr(), sizerFlags);

    sizer->AddGrowableCol(0, 1);
    panel->SetVirtualSize(sizer->GetSize());
    panel->Layout();
}
