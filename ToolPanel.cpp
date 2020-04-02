#include "ToolPanel.h"
#include "InputPlane.h"
#include "LinkedCtrls.h"
#include "OutputPlane.h"

// clang-format off

wxBEGIN_EVENT_TABLE(NumCtrlPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, NumCtrlPanel::OnTextEntry)
//EVT_SPINCTRLDOUBLE(wxID_ANY, ToolPanel::OnSpinCtrlTextEntry)
EVT_PAINT(ToolPanel::OnPaintEvent)
EVT_SPIN_UP(wxID_ANY, ToolPanel::OnSpinButtonUp)
EVT_SPIN_DOWN(wxID_ANY, ToolPanel::OnSpinButtonDown)
EVT_CHECKBOX(wxID_ANY, ToolPanel::OnCheckBox)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(VariableEditPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
//EVT_SPINCTRLDOUBLE(wxID_ANY, ToolPanel::OnSpinCtrlTextEntry)
EVT_PAINT(VariableEditPanel::OnPaintEvent)
EVT_SPIN_UP(wxID_ANY, ToolPanel::OnSpinButtonUp)
EVT_SPIN_DOWN(wxID_ANY, ToolPanel::OnSpinButtonDown)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(AnimPanel, wxHVScrolledWindow)
EVT_COMBOBOX(wxID_ANY, ToolPanel::OnTextEntry)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
//EVT_SPINCTRLDOUBLE(wxID_ANY, AnimPanel::OnSpinCtrlTextEntry)
EVT_BUTTON(ID_New_Anim, AnimPanel::OnButtonNewAnim)
EVT_CHECKBOX(wxID_ANY, ToolPanel::OnTextEntry)
EVT_BUTTON(wxID_ANY, AnimPanel::OnRemoveAnim)
wxEND_EVENT_TABLE();
// clang-format on

ToolPanel::ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
    : wxScrolledWindow(parent, ID, pos, size)
{
    wxBoxSizer* siz = new wxBoxSizer(wxVERTICAL);
    intermediate    = new wxPanel(this);
    SetScrollRate(30, 30);
    siz->Add(intermediate, wxSizerFlags(1).Expand());
    SetSizer(siz);
}

ToolPanel::~ToolPanel()
{
    intermediate->Destroy();
    for (auto C : linkedCtrls)
        delete C;
}

void ToolPanel::OnTextEntry(wxCommandEvent& event)
{
    for (auto ctrl : linkedCtrls)
    {
        if (ctrl->GetId() == event.GetId()) { ctrl->WriteLinked(); }
    }
    RefreshLinked();
}

void ToolPanel::OnSpinButtonUp(wxSpinEvent& event)
{
    for (auto ctrl : linkedCtrls)
    {
        auto spin = dynamic_cast<LinkedCplxSpinCtrl*>(ctrl);
        if (spin)
        {
            if (spin->GetIdReSpin() == event.GetId())
            {
                spin->Add(cplx(0.1, 0));
                spin->ReadLinked();
            }
            else if (spin->GetIdImSpin() == event.GetId())
            {
                spin->Add(cplx(0, 0.1));
                spin->ReadLinked();
            }
        }
    }
    RefreshLinked();
}

void ToolPanel::OnSpinButtonDown(wxSpinEvent& event)
{
    for (auto ctrl : linkedCtrls)
    {
        auto spin = dynamic_cast<LinkedCplxSpinCtrl*>(ctrl);
        if (spin)
        {
            if (spin->GetIdReSpin() == event.GetId())
            {
                spin->Add(cplx(-0.1, 0));
                spin->ReadLinked();
            }
            else if (spin->GetIdImSpin() == event.GetId())
            {
                spin->Add(cplx(0, -0.1));
                spin->ReadLinked();
            }
        }
    }
    RefreshLinked();
}

void ToolPanel::OnPaintEvent(wxPaintEvent& event)
{
    if (NeedsUpdate()) // && !controls.empty())
    {
        for (auto ctrl : linkedCtrls)
        {
            ctrl->ReadLinked();
        }
    }
}

void NumCtrlPanel::OnTextEntry(wxCommandEvent& event)
{
    ToolPanel::OnTextEntry(event);
    input->UpdateGrid();
    for (auto out : outputs)
        out->movedViewPort = true;
}

void NumCtrlPanel::PopulateAxisTextCtrls()
{
    lastPopulateFn = [&] { PopulateAxisTextCtrls(); };
    Freeze();

    ClearPanel();
    std::string buttonText[] = {
        "Real Min:", "Real Max:", "Imag Min:", "Imag Max:"};
    auto sizer = new wxFlexGridSizer(1, 0, 0);
    sizer->SetFlexibleDirection(wxHORIZONTAL);
    wxSizerFlags sizerFlags(1);
    sizerFlags.Expand().Border(wxLEFT | wxRIGHT, 3);
    wxFont normalFont = intermediate->GetFont();

    if (input != nullptr)
    {
        intermediate->SetFont(normalFont.Bold());
        wxCtrls.push_back(new wxStaticText(intermediate, wxID_ANY,
                                           wxString(input->GetName_() + ":"),
                                           wxDefaultPosition, wxDefaultSize));
        sizer->Add(wxCtrls.back(), sizerFlags);
        intermediate->SetFont(normalFont);

        for (int i = 0; i < 4; i++)
        {
            double c = input->axes.c[i];
            AddwxCtrl(new wxStaticText(intermediate, wxID_ANY,
                                       wxString(buttonText[i]),
                                       wxDefaultPosition, wxDefaultSize));
            AddLinkedCtrl(new LinkedAxisCtrl(
                intermediate, wxID_ANY, wxString(std::to_string(c)),
                wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, input,
                input->axes.c + i));
            sizer->Add(wxCtrls.back(), sizerFlags);
            sizer->Add(linkedCtrls.back()->GetCtrlPtr(), sizerFlags);
        }
    }
    sizer->AddSpacer(ROW_HEIGHT);
    if (!outputs.empty())
    {
        for (int i = 0; i < 4 * outputs.size(); i++)
        {
            if (i % 4 == 0)
            {
                intermediate->SetFont(normalFont.Bold());
                wxCtrls.push_back(
                    new wxStaticText(intermediate, wxID_ANY,
                                     wxString(outputs[i]->GetName_() + ":"),
                                     wxDefaultPosition, wxDefaultSize));
                sizer->Add(wxCtrls.back(), sizerFlags);
                intermediate->SetFont(normalFont);
            }
            double c = outputs[i / 4]->axes.c[i % 4];
            AddwxCtrl(new wxStaticText(intermediate, wxID_ANY,
                                       wxString(buttonText[i % 4]),
                                       wxDefaultPosition, wxDefaultSize));
            AddLinkedCtrl(new LinkedAxisCtrl(
                intermediate, wxID_ANY, wxString(std::to_string(c)),
                wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER,
                outputs[i / 4], outputs[i / 4]->axes.c + (i % 4)));
            sizer->Add(wxCtrls.back(), sizerFlags);
            sizer->Add(linkedCtrls.back()->GetCtrlPtr(), sizerFlags);
        }
    }
    sizer->AddGrowableCol(0, 1);
    intermediate->SetSizer(sizer);
    Layout();
    Thaw();
}

void NumCtrlPanel::PopulateContourTextCtrls(Contour* C)
{
    lastPopulateFn = [=] { PopulateContourTextCtrls(C); };
    Freeze();
    ClearPanel();
    if (C) // should never be nullptr, but there's a fallback option, anyway.
        C->PopulateMenu(this);
    else
        PopulateAxisTextCtrls();
    Layout();
    Thaw();
}

bool NumCtrlPanel::NeedsUpdate()
{
    return (input->GetState() > -1 || input->movedViewPort == true);
}

void NumCtrlPanel::RefreshLinked()
{
    input->Update();
    input->Refresh();
    for (auto out : outputs)
    {
        out->MarkAllForRedraw();
        out->Update();
        out->Refresh();
    }
}

void ToolPanel::ClearPanel()
{
    for (auto C : linkedCtrls)
    {
        C->Destroy();
    }
    for (auto D : wxCtrls)
    {
        D->Destroy();
    }
    linkedCtrls.clear();
    wxCtrls.clear();
    if (intermediate != nullptr)
    {
        if (auto size = intermediate->GetSizer(); size != nullptr)
        { intermediate->SetSizer(NULL, true); }
    }
}

void VariableEditPanel::RefreshLinked()
{
    output->MarkAllForRedraw();
    output->Update();
    output->Refresh();
    output->movedViewPort = true;
}

void VariableEditPanel::Populate(ParsedFunc<cplx>& F)
{
    Freeze();
    ClearPanel();
    lastFunc   = &F;
    auto sizer = new wxFlexGridSizer(1, 0, 0);
    sizer->SetFlexibleDirection(wxHORIZONTAL);
    wxSizerFlags sizerFlags(1);
    sizerFlags.Expand().Border(wxLEFT | wxRIGHT, 3);
    wxFont normalFont = intermediate->GetFont();

    intermediate->SetFont(normalFont.Bold());
    AddwxCtrl(new wxStaticText(intermediate, wxID_ANY, wxString("Parameters :"),
                               wxDefaultPosition, wxDefaultSize));
    intermediate->SetFont(normalFont);

    sizer->Add(wxCtrls.back(), sizerFlags);

    std::vector<Symbol<cplx>*> vars = F.GetVars();

    for (auto v : vars)
    {
        if (v->GetToken() == F.GetIV()) continue;
        cplx val      = v->eval().GetVal();
        std::string c = std::to_string(val.real()) + " + " +
                        std::to_string(val.imag()) + "i";
        AddwxCtrl(new wxStaticText(intermediate, wxID_ANY,
                                   wxString(v->GetToken()), wxDefaultPosition,
                                   wxDefaultSize));
        AddLinkedCtrl(new LinkedVarTextCtrl(intermediate, wxID_ANY, c,
                                            wxDefaultPosition, wxDefaultSize,
                                            wxTE_PROCESS_ENTER, v, history));
        sizer->Add(wxCtrls.back(), sizerFlags);
        sizer->Add(linkedCtrls.back()->GetCtrlPtr(), sizerFlags);
    }
    intermediate->SetSizer(sizer);
    intermediate->SetVirtualSize(sizer->GetSize());
    sizer->AddGrowableCol(0, 1);
    Layout();
    Thaw();
}

void VariableEditPanel::OnPaintEvent(wxPaintEvent& event)
{
    for (auto ctrl : linkedCtrls)
    {
        ctrl->ReadLinked();
    }
}

AnimPanel::AnimPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size,
                     InputPlane* in)
    : ToolPanel(parent, ID, pos, size)
{
    input      = in;
    auto sizer = new wxFlexGridSizer(1, 0, 0);
    sizer->SetFlexibleDirection(wxHORIZONTAL);

    newAnimButton = new wxButton(intermediate, ID_New_Anim, "New Animation");
    AddwxCtrl(newAnimButton);
    sizer->Add(newAnimButton);
    intermediate->SetSizer(sizer);
    sizer->AddGrowableCol(0, 1);
}

void AnimPanel::AddAnimation(int index, std::shared_ptr<Animation> ptr)
{
    if (ptr)
        input->InsertAnimation(index, ptr);
    else
        input->InsertAnimation(index, std::make_shared<Animation>());
    AddAnimCtrl(index);
}

void AnimPanel::AddAnimCtrl(int index)
{
    if (index < 0) index = input->AnimCount() - 1;
    Freeze();
    auto newAnim =
        new AnimCtrl(intermediate, input, input->GetAnimation(index));
    auto sizer = intermediate->GetSizer();
    wxSizerFlags sizerFlags(1);
    sizerFlags.Border(wxLEFT | wxRIGHT, 3).Expand();
    sizer->Insert(sizer->GetItemCount() - 1, newAnim->GetCtrlPtr(), sizerFlags);
    AddLinkedCtrl(newAnim);
    FinishLayout();
    Thaw();
}

void AnimPanel::OnButtonNewAnim(wxCommandEvent& event)
{
    AddAnimation();
    history->RecordCommand(
        std::make_unique<CommandAddAnim>(input->GetAnimation(-1), input));
}

void AnimPanel::OnRemoveAnim(wxCommandEvent& event)
{
    Freeze();
    size_t index = 0;
    for (auto A : linkedCtrls)
    {
        if (A->GetId() == event.GetId())
        {
            history->RecordCommand(
                std::make_unique<CommandRemoveAnim>(index, input));
            linkedCtrls[index]->Destroy();
            linkedCtrls.erase(linkedCtrls.begin() + index);
            input->RemoveAnimation(index);
            Layout();
            break;
        }
        index++;
    }
    FinishLayout();
    Thaw();
}

void AnimPanel::Populate()
{
    Freeze();
    ClearPanel();
    auto sizer = new wxFlexGridSizer(1, 0, 0);
    sizer->SetFlexibleDirection(wxHORIZONTAL);

    newAnimButton = new wxButton(intermediate, ID_New_Anim, "New Animation");
    AddwxCtrl(newAnimButton);
    sizer->Add(newAnimButton);
    intermediate->SetSizer(sizer);

    for (int i = 0; i < input->AnimCount(); i++)
    {
        AddAnimCtrl(i);
        linkedCtrls.back()->ReadLinked();
    }

    Thaw();
}

void AnimPanel::UpdateComboBoxes()
{
    for (auto C : linkedCtrls)
        C->UpdateCtrl();
}

void AnimPanel::FinishLayout()
{
    SetVirtualSize(GetSizer()->GetSize());
    FitInside();
    Layout();
}
