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
    Subdivide(res);
}

ContourPolygon* ContourParametric::Map(ParsedFunc<cplx>& g)
{
    ContourPolygon* C = new ContourPolygon(color, "f(" + name + ")");
    C->Reserve(points.size());

    for (auto& z : points)
        C->AddPoint(g(z));
    return C;
}

void ContourParametric::Subdivide(int res)
{
    points.clear();
    double tStep = (tEnd - tStart) / res;
    auto tEnd2   = tEnd + 0.000001; // To avoid FP rounding issues;
    for (double t = tStart; t <= tEnd2; t += tStep)
        ContourPolygon::AddPoint(f(t));
}

cplx ContourParametric::Interpolate(double t)
{
    return f(t * tEnd + (1 - t) * tStart);
}

// CommandContourTranslate* ContourParametric::CreateActionCommand(cplx c)
//{
//    return new CommandContourTranslate(this, c, c);
//}

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

    auto tStartTextBox = new LinkedDoubleTextCtrl(
        panel, wxID_ANY, wxString(std::to_string(tStart)), wxDefaultPosition,
        wxDefaultSize, wxTE_PROCESS_ENTER, &tStart);
    TP->AddLinkedCtrl(tStartTextBox);
    sizer->Add(tStartTextBox->GetCtrlPtr(), sizerFlags);

    auto tEndLabel = new wxStaticText(panel, wxID_ANY, wxString("t start"),
                                      wxDefaultPosition, wxDefaultSize);
    TP->AddwxCtrl(tEndLabel);
    sizer->Add(tEndLabel, sizerFlags);

    auto tEndTextBox = new LinkedDoubleTextCtrl(
        panel, wxID_ANY, wxString(std::to_string(tEnd)), wxDefaultPosition,
        wxDefaultSize, wxTE_PROCESS_ENTER, &tEnd);
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
