#include "LinkedTextCtrl.h"
#include "Animation.h"
#include "Commands.h"
#include "Contour.h"
#include "InputPlane.h"
#include "Parser.h"
#include "ToolPanel.h"

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
    reSpin   = new wxSpinButton(panel, -1, defaultPos, wxSize(3 * h / 5, h), style);
    imSpin   = new wxSpinButton(panel, -1, defaultPos, wxSize(3 * h / 5, h), style);

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
                               wxPoint defaultPos, wxSize defSize, int style,
                               ParsedFunc<cplx>* f)
    : src(f), TP(par)
{
    textCtrl =
        new wxTextCtrl(par->intermediate, ID, str, defaultPos, defSize, style);
}

void LinkedFuncCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        *src = parser.Parse(textCtrl->GetValue());
        TP->RePopulate();
    }
    catch (std::invalid_argument& func)
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

AnimCtrl::AnimCtrl(wxWindow* parent, InputPlane* in, Animation* a)
    : anim(a), input(in)
{
    wxImage::AddHandler(new wxPNGHandler);
    panel          = new wxPanel(parent);
    contourChoices = input->GetContourNames();
    commandChoices.Add("Translate");
    commandChoices.Add("Move Ctrl Point");

    auto panelID      = panel->GetId();
    auto subjectLabel = new wxStaticText(panel, panelID, "Subject: ");
    subjectMenu       = new wxComboBox(panel, panelID, "", wxDefaultPosition,
                                 wxDefaultSize, contourChoices, wxCB_READONLY);
    auto commandLabel   = new wxStaticText(panel, panelID, "Command: ");
    commandMenu         = new wxComboBox(panel, panelID, "", wxDefaultPosition,
                                 wxDefaultSize, commandChoices, wxCB_READONLY);
    auto pathLabel      = new wxStaticText(panel, panelID, "Path: ");
    pathMenu            = new wxComboBox(panel, panelID, "", wxDefaultPosition,
                              wxDefaultSize, contourChoices, wxCB_READONLY);
    auto handleLabel    = new wxStaticText(panel, panelID, "Handle: ");
    handleMenu          = new wxComboBox(panel, panelID, "", wxDefaultPosition,
                                wxDefaultSize, handleChoices, wxCB_READONLY);
    auto durationLabel  = new wxStaticText(panel, panelID, "Duration: ");
    durationCtrl        = new LinkedDoubleTextCtrl(panel, panelID, "3.0",
                                            wxDefaultPosition, wxSize(48, -1),
                                            wxTE_PROCESS_ENTER, &dur, 0.1, 3.0);
    auto durOffsetLabel = new wxStaticText(panel, panelID, "Offset: ");
    durOffsetCtrl       = new LinkedDoubleTextCtrl(panel, panelID, "0.0",
                                             wxDefaultPosition, wxSize(48, -1),
                                             wxTE_PROCESS_ENTER, &offset, 0.1);
    reverseCtrl = new wxCheckBox(panel, panelID, "Reverse: ", wxDefaultPosition,
                                 wxDefaultSize, wxALIGN_RIGHT);
    bounceCtrl  = new wxCheckBox(panel, panelID, "Bounce: ", wxDefaultPosition,
                                wxDefaultSize, wxALIGN_RIGHT);
    removeButton = new wxButton(panel, panelID, "Remove");
    //removeButton->SetBitmap(wxBitmap("icons/close.png", wxBITMAP_TYPE_PNG));

    sizer = new wxBoxSizer(wxHORIZONTAL);
    wxSizerFlags flag1(1);
    flag1.Border(wxALL, 3).Proportion(1);
    wxSizerFlags flag0(1);
    flag0.Border(wxALL, 3).Proportion(0);

    sizer->Add(subjectLabel, flag0);
    sizer->Add(subjectMenu, flag1);
    sizer->Add(commandLabel, flag0);
    sizer->Add(commandMenu, flag1);
    sizer->Add(pathLabel, flag0);
    sizer->Add(pathMenu, flag1);
    sizer->Add(handleLabel, flag0);
    sizer->Add(handleMenu, flag1);
    sizer->Add(durationLabel, flag0);
    sizer->Add(durationCtrl->GetCtrlPtr(), flag0);
    sizer->Add(durOffsetLabel, flag0);
    sizer->Add(durOffsetCtrl->GetCtrlPtr(), flag0);
    sizer->Add(reverseCtrl, flag0);
    sizer->Add(bounceCtrl, flag0);
    sizer->Add(removeButton, flag0);
    panel->SetSizer(sizer);
    panel->Fit();
}

void AnimCtrl::WriteLinked()
{
    anim->ClearCommands();
    int subj = subjectMenu->GetSelection();
    int path  = pathMenu->GetSelection();
    int com = commandMenu->GetSelection();

    int handle = handleMenu->GetSelection();
    PopulateHandleMenu();
    handleMenu->SetSelection(handle);

    reverse = reverseCtrl->IsChecked() ? -1 : 1;

    durationCtrl->WriteLinked();
    durOffsetCtrl->WriteLinked();

    if (subj > -1 && com > -1 && path > -1 && handle > -1)
    {
        dur = dur ? dur : 1;
        anim->duration_ms = 1000 * dur;
        anim->reverse = reverse;
        anim->offset = offset / dur;
        anim->bounce = bounceCtrl->GetValue();
        anim->subjSel = subj;
        anim->comSel = com;
        anim->pathSel = path;
        anim->handle = handle;

        auto C = input->GetContour(subj);
        auto pathContour = input->GetContour(path);
        if (C != pathContour)
        {
            anim->SetPathContour(pathContour);

            switch (auto command = commandMenu->GetSelection())
            {
            case COMMAND_PLACE_AT:
                handle--; // Center is first on list and has index -1.
                anim->AddCommand(
                    std::make_unique<CommandContourPlaceAt>(C, 0, handle));
                break;
            case COMMAND_SET_PT:
                anim->AddCommand(
                    std::make_unique<CommandContourMovePoint>(C, 0, handle));
                break;
            }
            anim->AddCommand(
                std::make_unique<CommandContourSubdivide>(C, input->GetRes()));
        }
    }
}

void AnimCtrl::ReadLinked()
{
    UpdateCtrl();
    reverse = anim->reverse;
    reverseCtrl->SetValue(reverse == -1);
    bounceCtrl->SetValue(anim->bounce);
    dur = anim->duration_ms / 1000.0;
    offset = anim->offset * dur;
    durationCtrl->ReadLinked();
    durOffsetCtrl->ReadLinked();

    subjectMenu->SetSelection(anim->subjSel);
    pathMenu->SetSelection(anim->pathSel);
    commandMenu->SetSelection(anim->comSel);
    PopulateHandleMenu();
    handleMenu->SetSelection(anim->handle);
}

void AnimCtrl::UpdateCtrl()
{
    contourChoices = input->GetContourNames();
    auto sel1      = subjectMenu->GetSelection();
    auto sel2      = pathMenu->GetSelection();
    auto sel3      = handleMenu->GetSelection();
    auto sel4      = durationCtrl->GetCtrlPtr()->GetValue();
    auto sel5      = bounceCtrl->GetValue();

    subjectMenu->Set(contourChoices);
    pathMenu->Set(contourChoices);
    handleMenu->Set(handleChoices);

    subjectMenu->SetSelection(sel1);
    pathMenu->SetSelection(sel2);
    if (handleChoices.size() > sel3) handleMenu->SetSelection(sel3);
    else handleMenu->SetSelection(0);
    durationCtrl->GetCtrlPtr()->SetValue(sel4);
    bounceCtrl->SetValue(sel5);
}

void AnimCtrl::PopulateHandleMenu()
{
    handleChoices.Clear();
    if (commandMenu && commandMenu->GetSelection() != COMMAND_SET_PT)
        handleChoices.Add("Center");
    auto C = input->GetContour(subjectMenu->GetSelection());
    if (C)
        for (int i = 0; i < C->GetPointCount(); i++)
        {
            handleChoices.Add("Ctrl Point " + std::to_string(i));
        }
    handleMenu->Set(handleChoices);
}
