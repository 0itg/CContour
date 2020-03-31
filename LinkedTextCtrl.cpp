#include "LinkedTextCtrl.h"
#include "ContourParametric.h"
#include "InputPlane.h"
#include "ToolPanel.h"

#include <wx/richtooltip.h>

void LinkedCtrlPointTextCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        cplx val = parser.Parse(textCtrl->GetValue()).eval();

        RecordCommand(val);

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

void LinkedCtrlPointTextCtrl::RecordCommand(cplx c)
{
    history->RecordCommand(
        std::make_unique<CommandContourSetPoint>(src, c, i));
}

void LinkedDoubleTextCtrl::WriteLinked()
{
    Parser<double> parser;
    try
    {
        // Our own parser is more powerful than the default input handling.
        double val = parser.Parse(textCtrl->GetText()->GetValue()).eval();

        RecordCommand(val);
        *src = val;
        UpdateCommand(val);
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
                                     Symbol<cplx>* sym, CommandHistory* ch)
    : src(sym), LinkedCplxSpinCtrl(par, ID, str, defaultPos, defSize, style), history(ch)
{
}

void LinkedVarTextCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        cplx val = parser.Parse(textCtrl->GetValue()).eval();
        history->RecordCommand(std::make_unique<CommandEditVar>(src->GetToken(), val, src->GetParent()));
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
    reSpin =
        new wxSpinButton(panel, -1, defaultPos, wxSize(3 * h / 5, h), style);
    imSpin =
        new wxSpinButton(panel, -1, defaultPos, wxSize(3 * h / 5, h), style);

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

LinkedParametricFuncCtrl::LinkedParametricFuncCtrl(
    ToolPanel* par, wxStandardID ID, wxString str, wxPoint defaultPos,
    wxSize defSize, int style, ContourParametric* cp)
    : src(cp->GetFunctionPtr()), TP(par), C(cp)
{
    textCtrl =
        new wxTextCtrl(par->intermediate, ID, str, defaultPos, defSize, style);
}

void LinkedParametricFuncCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        auto f = parser.Parse(textCtrl->GetValue());
        TP->GetHistoryPtr()->RecordCommand(
            std::make_unique<CommandParametricFuncEntry>(C, f));
        *src = f;
        TP->Populate();
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

void LinkedParametricFuncCtrl::ReadLinked()
{
    textCtrl->ChangeValue(src->GetInputText());
}

AnimCtrl::AnimCtrl(wxWindow* parent, InputPlane* in, std::shared_ptr<Animation> a)
    : anim(a), input(in)
{
    panel          = new wxPanel(parent);
    pathChoices = input->GetContourNames();
    commandChoices.Add("Translate");
    commandChoices.Add("Move Ctrl Point");
    commandChoices.Add("Parameterize Var");

    auto panelID        = panel->GetId();
    auto subjectLabel   = new wxStaticText(panel, panelID, "Subject: ");
    subjectMenu         = new MappedComboBox<int>(panel, panelID, "", wxDefaultPosition,
                                 wxDefaultSize, pathChoices, wxCB_READONLY);
    auto commandLabel   = new wxStaticText(panel, panelID, "Command: ");
    commandMenu         = new wxComboBox(panel, panelID, "", wxDefaultPosition,
                                 wxDefaultSize, commandChoices, wxCB_READONLY);
    auto pathLabel      = new wxStaticText(panel, panelID, "Path: ");
    pathMenu            = new wxComboBox(panel, panelID, "", wxDefaultPosition,
                              wxDefaultSize, pathChoices, wxCB_READONLY);
    auto handleLabel    = new wxStaticText(panel, panelID, "Point/Var: ");
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
    // removeButton->SetBitmap(wxBitmap("icons/close.png", wxBITMAP_TYPE_PNG));

    sizer = new wxBoxSizer(wxHORIZONTAL);
    wxSizerFlags flag1(1);
    flag1.Border(wxALL, 3).Proportion(1);
    wxSizerFlags flag0(1);
    flag0.Border(wxALL, 3).Proportion(0);

    sizer->Add(commandLabel, flag0);
    sizer->Add(commandMenu, flag1);
    sizer->Add(subjectLabel, flag0);
    sizer->Add(subjectMenu, flag1);
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
    PopulateSubjectMenu();
    subjectMenu->SetSelection(subj);

    int path = pathMenu->GetSelection();
    int com  = commandMenu->GetSelection();

    int handle = handleMenu->GetSelection();
    PopulateHandleMenu();
    if (handle < handleChoices.size())
        handleMenu->SetSelection(handle);
    else
    {
        handleMenu->SetSelection(-1);
        handle = -1;
    }

    reverse = reverseCtrl->IsChecked() ? -1 : 1;

    durationCtrl->WriteLinked();
    durOffsetCtrl->WriteLinked();

    if (subj > -1 && com > -1 && path > -1 && handle > -1)
    {
        anim->ClearCommands();
        dur               = dur ? dur : 1;

        auto C           = input->GetContour(subj);
        auto pathContour = input->GetContour(path);

        auto edit = std::make_unique<CommandEditAnim>(anim, 1000 * dur, reverse,
            offset / dur, bounceCtrl->GetValue(),
            subj, com, path, handle, pathContour);
        edit->exec();

        input->GetHistoryPtr()->RecordCommand(std::move(edit));
        auto command = commandMenu->GetSelection();
        if (C != pathContour && (C != nullptr || command == COMMAND_EDIT_VAR))
        {
            switch (command)
            {
            case COMMAND_PLACE_AT:
                // Center is first on list and has index -1, hence handle -1.
                anim->AddCommand(std::make_unique<CommandContourPlaceAt>(
                    C.get(), 0, handle - 1));
                anim->AddCommand(std::make_unique<CommandContourSubdivide>(
                    C.get(), input->GetRes()));
                break;
            case COMMAND_SET_PT:
                anim->AddCommand(std::make_unique<CommandContourSetPoint>(
                    C.get(), 0, handle));
                anim->AddCommand(std::make_unique<CommandContourSubdivide>(
                    C.get(), input->GetRes()));
                break;
            case COMMAND_EDIT_VAR:
                auto f = input->GetFunction();
                if (C && C->IsParametric())
                    f = reinterpret_cast<ContourParametric*>(C.get())->GetFunction();
                anim->AddCommand(std::make_unique<CommandEditVar>(
                    handleMenu->GetString(handle), 0, f));
                anim->animateGrid = true;
                break;
            }
        }
    }
}

void AnimCtrl::ReadLinked()
{
    UpdateCtrl();
    reverse = anim->reverse;
    reverseCtrl->SetValue(reverse == -1);
    bounceCtrl->SetValue(anim->bounce);
    dur    = anim->duration_ms / 1000.0;
    offset = anim->offset * dur;
    durationCtrl->ReadLinked();
    durOffsetCtrl->ReadLinked();

    commandMenu->SetSelection(anim->comSel);
    PopulateSubjectMenu();
    subjectMenu->SetSelection(anim->subjSel);
    pathMenu->SetSelection(anim->pathSel);
    PopulateHandleMenu();
    handleMenu->SetSelection(anim->handle);
}

void AnimCtrl::UpdateCtrl()
{
    pathChoices = input->GetContourNames();
    auto sel1      = subjectMenu->GetSelection();
    auto sel2      = pathMenu->GetSelection();
    auto sel3      = handleMenu->GetSelection();
    auto sel4      = durationCtrl->GetCtrlPtr()->GetValue();
    auto sel5      = bounceCtrl->GetValue();

    //subjectMenu->Set(pathChoices);
    pathMenu->Set(pathChoices);
    handleMenu->Set(handleChoices);

    PopulateSubjectMenu();
    subjectMenu->SetSelection(sel1);
    pathMenu->SetSelection(sel2);
    if (handleChoices.size() <= sel3)
        sel3 = -1;
    PopulateHandleMenu();
    handleMenu->SetSelection(sel3);
    durationCtrl->GetCtrlPtr()->SetValue(sel4);
    bounceCtrl->SetValue(sel5);
}

void AnimCtrl::PopulateHandleMenu()
{
    handleChoices.Clear();
    if (commandMenu)
    {
        auto cmd = commandMenu->GetSelection();
        auto subj = subjectMenu->GetSelection();
        auto C = input->GetContour(subj);
        switch (cmd)
        {
        case COMMAND_PLACE_AT:
            handleChoices.Add("Center");
        case COMMAND_SET_PT:
        {
            if (C)
                for (int i = 0; i < C->GetPointCount(); i++)
                {
                    handleChoices.Add("Ctrl Point " + std::to_string(i));
                }
            break;
        }
        case COMMAND_EDIT_VAR:
        {
            auto f = input->GetFunction();
            if (C && subj < std::numeric_limits<int>::max())
                f = reinterpret_cast<ContourParametric*>(C.get())->GetFunction();
            if (f)
            {
                auto varMap = f->GetVarMap();
                varMap.erase(f->GetIV());
                for (auto varName : varMap)
                {
                    handleChoices.Add(varName.first);
                }
            }
            break;
        }
        }
    }
    handleMenu->Set(handleChoices);
}

void AnimCtrl::PopulateSubjectMenu()
{
    if (commandMenu)
    {
        subjectChoices.clear();
        subjectMenu->Map.clear();
        auto cmd = commandMenu->GetSelection();
        bool parametric = false;

        int i = 0;

        if (cmd == COMMAND_EDIT_VAR)
        {
            parametric = true;
            subjectChoices.Add("f(z)");
            subjectMenu->Map[0] = std::numeric_limits<int>::max();
            i++;
        }

        subjectChoiceMap = input->GetParametricContours(parametric);

        for (auto&& s : subjectChoiceMap)
        {
            subjectChoices.Add(s.first);
            subjectMenu->Map[i] = s.second;
            i++;
        }
    }
    subjectMenu->Set(subjectChoices);
}

void LinkedAxisCtrl::RecordCommand(cplx c)
{
    plane->GetHistoryPtr()->RecordCommand(
        std::make_unique<CommandAxesSet>(plane));
}

void LinkedAxisCtrl::UpdateCommand(cplx c)
{
    plane->GetHistoryPtr()->UpdateLastCommand(c);
}

void LinkedRadiusCtrl::RecordCommand(cplx c)
{
    history->RecordCommand(
        std::make_unique<CommandContourEditRadius>(C, c.real()));
}
