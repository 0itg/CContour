#include "LinkedTextCtrl.h"
#include "Contour.h"
#include "Parser.h"
#include "ToolPanel.h"
#include "InputPlane.h"
#include "Animation.h"
#include "Commands.h"

#include <wx/richtooltip.h>

void LinkedCtrlPointTextCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        cplx val = parser.Parse(textCtrl->GetValue()).eval();
        src->SetCtrlPoint(i, val);
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

void LinkedCtrlPointTextCtrl::ReadLinked()
{
    textCtrl->ChangeValue(std::to_string(src->GetCtrlPoint(i).real()) + " + " +
                          std::to_string(src->GetCtrlPoint(i).imag()) + "i");
}

void LinkedCtrlPointTextCtrl::Add(cplx c)
{
    src->SetCtrlPoint(i, src->GetCtrlPoint(i) + c);
}

void LinkedDoubleTextCtrl::WriteLinked()
{
    Parser<double> parser;
    try
    {
        // Our own parser is more powerful than the default input handling.
        double val = parser.Parse(textCtrl->GetText()->GetValue()).eval();
        *src       = val;
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

LinkedVarTextCtrl::LinkedVarTextCtrl(wxWindow* par, wxStandardID ID,
                                     wxString str, wxPoint defaultPos,
                                     wxSize defSize, int style,
                                     Symbol<cplx>* sym)
    : src(sym), LinkedCplxSpinCtrl(par, ID, str, defaultPos, defSize, style)
{
}

void LinkedVarTextCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        cplx val = parser.Parse(textCtrl->GetValue()).eval();
        src->SetVal(val);
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

void LinkedVarTextCtrl::ReadLinked()
{
    textCtrl->ChangeValue(std::to_string(src->eval().GetVal().real()) + " + " +
                          std::to_string(src->eval().GetVal().imag()) + "i");
}

void LinkedVarTextCtrl::Add(cplx c)
{
    auto d = src->GetVal();
    src->SetVal(c + d);
}

LinkedCplxSpinCtrl::LinkedCplxSpinCtrl(wxWindow* par, wxStandardID ID,
                                       wxString str, wxPoint defaultPos,
                                       wxSize defSize, int style)
{
    panel    = new wxPanel(par, ID, defaultPos, defSize);
    textCtrl = new wxTextCtrl(panel, ID, str, defaultPos, defSize, style);
    auto h   = textCtrl->GetSize().y;
    reSpin   = new wxSpinButton(panel, -1, defaultPos, wxSize(h / 2, h), style);
    imSpin   = new wxSpinButton(panel, -1, defaultPos, wxSize(h / 2, h), style);

    constexpr int min = std::numeric_limits<int>::min();
    constexpr int max = std::numeric_limits<int>::max();
    reSpin->SetRange(min, max);
    imSpin->SetRange(min, max);
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(reSpin);
    sizer->Add(textCtrl, wxSizerFlags(1).Expand());
    sizer->Add(imSpin);
    panel->SetSizer(sizer);
}

LinkedFuncCtrl::LinkedFuncCtrl(ToolPanel* par, wxStandardID ID, wxString str,
    wxPoint defaultPos, wxSize defSize, int style, ParsedFunc<cplx>* f)
    : src(f), TP(par)
{
    textCtrl = new wxTextCtrl(par->intermediate, ID, str, defaultPos, defSize, style);
}

void LinkedFuncCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        *src = parser.Parse(textCtrl->GetValue());
        TP->RePopulate();
    }
    catch (std::invalid_argument & func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

void LinkedFuncCtrl::ReadLinked()
{
    textCtrl->ChangeValue(src->GetInputText());
}


AnimCtrl::AnimCtrl(wxWindow* parent, InputPlane* in, Animation* a) :
    anim(a), input(in)
{
    panel = new wxPanel(parent);
    contourChoices = input->GetContourNames();
    handleChoices.Add("Center");
    auto C = input->GetContour(0);
    if (C)
        for (int i = 0; i < input->GetContour(0)->GetPointCount(); i++)
        {
            handleChoices.Add("Ctrl Point " + std::to_string(i));
        }
    auto panelID = panel->GetId();
    auto subjectLabel = new wxStaticText(panel, panelID, "Subject: ");
    subjectMenu = new wxComboBox(panel, panelID,
        "", wxDefaultPosition, wxDefaultSize, contourChoices, wxCB_READONLY);
    auto commandLabel = new wxStaticText(panel, panelID, "Command: ");
    commandMenu = new wxComboBox(panel, panelID, "", wxDefaultPosition,
        wxDefaultSize, commandChoices, wxCB_READONLY);
    auto handleLabel = new wxStaticText(panel, panelID, "Handle: ");
    handleMenu = new wxComboBox(panel, panelID, "", wxDefaultPosition,
        wxDefaultSize, handleChoices, wxCB_READONLY);
    auto durationLabel = new wxStaticText(panel, panelID, "Duration: ");
    durationCtrl = new LinkedDoubleTextCtrl(panel, panelID,
        "1.0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, &dur, 0.1, 1.0);

    sizer = new wxBoxSizer(wxHORIZONTAL);
    wxSizerFlags flags(1);
    flags.Border(wxALL, 3).Proportion(1);
    wxSizerFlags labelFlags(1);
    labelFlags.Border(wxALL, 3).Proportion(0);
    sizer->Add(subjectLabel, labelFlags);
    sizer->Add(subjectMenu, flags);
    sizer->Add(commandLabel, labelFlags);
    sizer->Add(commandMenu, flags);
    sizer->Add(handleLabel, labelFlags);
    sizer->Add(handleMenu, flags);
    sizer->Add(durationLabel, labelFlags);
    sizer->Add(durationCtrl->GetCtrlPtr(), flags);
    panel->SetSizer(sizer);
    panel->Fit();
}

void AnimCtrl::WriteLinked()
{
    anim->ClearCommands();
    int subj = subjectMenu->GetSelection();
    int com = commandMenu->GetSelection();
    int handle = handleMenu->GetSelection();
    durationCtrl->WriteLinked();
    if (subj > -1 && com > -1 && handle > -1)
    {
        handle--;
        anim->duration_ms = 1000 * dur;

        auto C = input->GetContour(subj);
        anim->SetFunction([=](double t) {
            auto ptr = input->GetContour(com);
            return ptr->Interpolate(t);
            });
        anim->AddCommand(std::make_unique<CommandContourPlaceAt>(C, 0, handle));
        anim->AddCommand(
            std::make_unique<CommandContourSubdivide>(C, input->GetRes()));
    }
}

void AnimCtrl::ReadLinked()
{
    UpdateComboBoxes();
}

void AnimCtrl::UpdateComboBoxes()
{
    contourChoices = input->GetContourNames();
    subjectMenu->Set(contourChoices);
    commandMenu->Set(contourChoices);
    handleMenu->Set(handleChoices);
}
