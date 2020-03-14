#include "ContourParametric.h"
#include "ToolPanel.h"
#include "LinkedTextCtrl.h"

ContourParametric::ContourParametric(
    std::string func, int res, wxColor col, std::string n, double tS, double tE) :
    tStart(tS), tEnd(tE)
{
    color = col;
    name = n;
    f = parser.Parse(func);
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
    auto tEnd2 = tEnd + 0.000001; // To avoid FP rounding issues;
    for (double t = tStart; t <= tEnd2; t += tStep)
        ContourPolygon::AddPoint(f(t));
}

cplx ContourParametric::Interpolate(double t)
{
    return f(t * tEnd + (1 - t) * tStart);
}

void ContourParametric::PopulateMenu(ToolPanel* TP)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto sizerFlags = wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT, 3);
    TP->intermediate->SetSizer(sizer);
    wxFont normalFont = TP->intermediate->GetFont();

    TP->intermediate->SetFont(normalFont.Bold());
    TP->AddDecoration(new wxStaticText(TP->intermediate, wxID_ANY,
        wxString(GetName() + ":"),
        wxDefaultPosition, wxDefaultSize));
    TP->intermediate->SetFont(normalFont);
    sizer->Add(TP->GetDecoration(0), sizerFlags);

    auto fnEdit = new LinkedFuncCtrl(TP, wxID_ANY, f.GetInputText(),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER, &f);
    TP->AddLinkedCtrl(fnEdit);
    sizer->Add(fnEdit->GetCtrlPtr(), sizerFlags);

    auto tStartLabel = new wxStaticText(TP->intermediate, wxID_ANY,
        wxString("t start"),
        wxDefaultPosition, wxDefaultSize);
    TP->AddDecoration(tStartLabel);
    sizer->Add(tStartLabel, sizerFlags);

    auto tStartTextBox = new LinkedDoubleTextCtrl(
        TP->intermediate, wxID_ANY, wxString(std::to_string(tStart)),
        wxDefaultPosition, TP->TEXTBOX_SIZE, wxTE_PROCESS_ENTER,
        &tStart);
    TP->AddLinkedCtrl(tStartTextBox);
    sizer->Add(tStartTextBox->GetCtrlPtr(), sizerFlags);

    auto tEndLabel = new wxStaticText(TP->intermediate, wxID_ANY,
        wxString("t start"),
        wxDefaultPosition, wxDefaultSize);
    TP->AddDecoration(tEndLabel);
    sizer->Add(tEndLabel, sizerFlags);

    auto tEndTextBox = new LinkedDoubleTextCtrl(
        TP->intermediate, wxID_ANY, wxString(std::to_string(tEnd)),
        wxDefaultPosition, TP->TEXTBOX_SIZE, wxTE_PROCESS_ENTER,
        &tEnd);
    TP->AddLinkedCtrl(tEndTextBox);
    sizer->Add(tEndTextBox->GetCtrlPtr(), sizerFlags);

    int rowCount = 6;

    std::vector<Symbol<cplx>*> vars = f.GetVars();
    rowCount += 2 * (vars.size() - 1);

    for (auto v : vars)
    {
        if (v->GetToken() == f.GetIV())
            continue;
        cplx val = v->eval().GetVal();
        std::string c = std::to_string(val.real()) + " + " +
            std::to_string(val.imag()) + "i";
        auto vName = new wxStaticText(TP->intermediate, wxID_ANY,
            wxString(v->GetToken()),
            wxDefaultPosition, wxDefaultSize);
        TP->AddDecoration(vName);
        auto vTextBox = new LinkedVarTextCtrl(TP->intermediate, wxID_ANY, c,
            wxDefaultPosition, wxDefaultSize,
            wxTE_PROCESS_ENTER, v);
        TP->AddLinkedCtrl(vTextBox);
        sizer->Add(vName, sizerFlags);
        sizer->Add(vTextBox->GetCtrlPtr(), sizerFlags);
    }

    TP->SetRowCount(rowCount);

    auto vExtent = wxSize(-1, rowCount * TP->ROW_HEIGHT);

    auto panel = TP->intermediate;
    panel->Layout();
    panel->SetMinClientSize(wxSize(TP->GetClientSize().x, vExtent.y));
    panel->SetMaxClientSize(vExtent);
    panel->SetVirtualSize(vExtent);

    panel->Fit();
}
