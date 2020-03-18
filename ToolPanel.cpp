#include "ToolPanel.h"
#include "Animation.h"
#include "Contour.h"
#include "InputPlane.h"
#include "LinkedTextCtrl.h"
#include "OutputPlane.h"
#include "Parser.h"
#include "Token.h"

#include <wx/richtooltip.h>

// clang-format off
//wxBEGIN_EVENT_TABLE(ToolPanel, wxVScrolledWindow)
//EVT_ERASE_BACKGROUND(ToolPanel::OnEraseEvent)
//wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(NumCtrlPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_SPINCTRLDOUBLE(wxID_ANY, ToolPanel::OnSpinCtrlTextEntry)
EVT_PAINT(ToolPanel::OnPaintEvent)
EVT_SPIN_UP(wxID_ANY, ToolPanel::OnSpinButtonUp)
EVT_SPIN_DOWN(wxID_ANY, ToolPanel::OnSpinButtonDown)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(VariableEditPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_SPINCTRLDOUBLE(wxID_ANY, ToolPanel::OnSpinCtrlTextEntry)
EVT_PAINT(VariableEditPanel::OnPaintEvent)
EVT_SPIN_UP(wxID_ANY, ToolPanel::OnSpinButtonUp)
EVT_SPIN_DOWN(wxID_ANY, ToolPanel::OnSpinButtonDown)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(AnimPanel, wxHVScrolledWindow)
EVT_COMBOBOX(wxID_ANY, ToolPanel::OnTextEntry)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_SPINCTRLDOUBLE(wxID_ANY, ToolPanel::OnSpinCtrlTextEntry)
EVT_BUTTON(ID_New_Anim, AnimPanel::OnAddAnimCtrl)
EVT_CHECKBOX(wxID_ANY, AnimPanel::OnTextEntry)
EVT_BUTTON(wxID_ANY, AnimPanel::OnRemoveAnim)
wxEND_EVENT_TABLE();
// clang-format on

ToolPanel::ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
    : wxHVScrolledWindow(parent, ID, pos, size)
{
    SetRowCount(20);
    wxBoxSizer* siz = new wxBoxSizer(wxVERTICAL);
    intermediate    = new wxPanel(this);
    siz->Add(intermediate, wxSizerFlags(1).Expand());
    SetSizer(siz);
    // wxFont font = intermediate->GetFont();
    // font.SetFamily(wxFONTFAMILY_MODERN);
    // intermediate->SetFont(font);
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

void NumCtrlPanel::PopulateAxisTextCtrls()
{
    lastPopulateFn = [&] { PopulateAxisTextCtrls(); };
    Freeze();

    ClearPanel();
    SetRowCount(20);
    std::string buttonText[] = {
        "Real Min:", "Real Max:", "Imag Min:", "Imag Max:"};
    auto sizer = new wxBoxSizer(wxVERTICAL);
    wxSizerFlags sizerFlags(1);
    sizerFlags.Expand().Border(wxLEFT | wxRIGHT, 3);
    wxFont normalFont = intermediate->GetFont();

    if (input != nullptr)
    {
        intermediate->SetFont(normalFont.Bold());
        wxCtrls.push_back(new wxStaticText(intermediate, wxID_ANY,
                                           wxString(input->GetName() + ":"),
                                           wxDefaultPosition, wxDefaultSize));
        sizer->Add(wxCtrls.back(), sizerFlags);
        intermediate->SetFont(normalFont);

        for (int i = 0; i < 4; i++)
        {
            double c = input->axes.c[i];
            AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                           wxString(buttonText[i]),
                                           wxDefaultPosition, wxDefaultSize));
            AddLinkedCtrl(new LinkedDoubleTextCtrl(
                intermediate, wxID_ANY, wxString(std::to_string(c)),
                wxDefaultPosition, TEXTBOX_SIZE, wxTE_PROCESS_ENTER,
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
                                     wxString(outputs[i]->GetName() + ":"),
                                     wxDefaultPosition, wxDefaultSize));
                sizer->Add(wxCtrls.back(), sizerFlags);
                intermediate->SetFont(normalFont);
            }
            double c = outputs[i / 4]->axes.c[i % 4];
            AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                           wxString(buttonText[i % 4]),
                                           wxDefaultPosition, wxDefaultSize));
            AddLinkedCtrl(new LinkedDoubleTextCtrl(
                intermediate, wxID_ANY, wxString(std::to_string(c)),
                wxDefaultPosition, TEXTBOX_SIZE, wxTE_PROCESS_ENTER,
                outputs[i / 4]->axes.c + (i % 4)));
            sizer->Add(wxCtrls.back(), sizerFlags);
            sizer->Add(linkedCtrls.back()->GetCtrlPtr(), sizerFlags);
        }
    }
    intermediate->SetSizer(sizer);
    // intermediate->SetMinClientSize(wxSize(-1, (4 * SPACING + ROW_HEIGHT) * (1
    // + outputs.size())));
    intermediate->SetMaxClientSize(wxSize(GetClientSize().x, -1));
    intermediate->Layout();
    intermediate->FitInside();
    intermediate->SetMaxClientSize(wxSize(
        -1, ROW_HEIGHT + (4 * SPACING + ROW_HEIGHT) * (1 + outputs.size())));
    SetVirtualSize(wxSize(-1, ROW_HEIGHT + (4 * SPACING + ROW_HEIGHT) *
                                               (1 + outputs.size())));

    Layout();
    Thaw();
}

void NumCtrlPanel::PopulateContourTextCtrls(Contour* C)
{
    lastPopulateFn = [=] { PopulateContourTextCtrls(C); };
    Freeze();
    ClearPanel();
    SetRowCount(2 * C->GetPointCount() + 1);
    C->PopulateMenu(this);

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

void VariableEditPanel::PopulateVarTextCtrls(ParsedFunc<cplx>& F)
{
    Freeze();
    ClearPanel();
    auto sizer = new wxBoxSizer(wxVERTICAL);
    wxSizerFlags sizerFlags(1);
    sizerFlags.Expand().Border(wxLEFT | wxRIGHT, 3);
    wxFont normalFont = intermediate->GetFont();

    intermediate->SetFont(normalFont.Bold());
    AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                   wxString("Parameters :"), wxDefaultPosition,
                                   wxDefaultSize));
    intermediate->SetFont(normalFont);

    sizer->Add(wxCtrls.back(), sizerFlags);

    std::vector<Symbol<cplx>*> vars = F.GetVars();
    SetRowCount(2 * vars.size());

    for (auto v : vars)
    {
        if (v->GetToken() == F.GetIV()) continue;
        cplx val      = v->eval().GetVal();
        std::string c = std::to_string(val.real()) + " + " +
                        std::to_string(val.imag()) + "i";
        AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                       wxString(v->GetToken()),
                                       wxDefaultPosition, wxDefaultSize));
        AddLinkedCtrl(new LinkedVarTextCtrl(intermediate, wxID_ANY, c,
                                            wxDefaultPosition, wxDefaultSize,
                                            wxTE_PROCESS_ENTER, v));
        sizer->Add(wxCtrls.back(), sizerFlags);
        sizer->Add(linkedCtrls.back()->GetCtrlPtr(), sizerFlags);
    }
    intermediate->SetMinClientSize(
        wxSize(GetClientSize().x, SPACING * (vars.size()) - ROW_HEIGHT));
    intermediate->SetMaxClientSize(
        wxSize(-1, SPACING * (vars.size()) - ROW_HEIGHT));
    SetVirtualSize(wxSize(-1, SPACING * (vars.size())));
    intermediate->SetSizer(sizer);
    intermediate->FitInside();
    intermediate->Layout();

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
    : ToolPanel(parent, ID, pos, size), input(in)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);
    intermediate->SetSizer(sizer);

    newAnimButton = new wxButton(intermediate, ID_New_Anim, "New Animation");
    AddDecoration(newAnimButton);
    sizer->Add(newAnimButton);
}

void AnimPanel::AddAnimCtrl()
{
    Freeze();
    input->animations.push_back(std::make_unique<Animation>());
    auto newAnim =
        new AnimCtrl(intermediate, input, input->animations.back().get());
    auto sizer = intermediate->GetSizer();
    wxSizerFlags sizerFlags(1);
    sizerFlags.Border(wxLEFT | wxRIGHT, 3);
    sizer->Insert(sizer->GetItemCount() - 1, newAnim->GetCtrlPtr(), sizerFlags);
    AddLinkedCtrl(newAnim);
    FinishLayout();
    Thaw();
}

void AnimPanel::OnRemoveAnim(wxCommandEvent& event)
{
    Freeze();
    size_t index = 0;
    for (auto A : linkedCtrls)
    {
        if (A->GetId() == event.GetId())
        {
            linkedCtrls[index]->Destroy();
            linkedCtrls.erase(linkedCtrls.begin() + index);
            input->animations[index].reset();
            input->animations.erase(input->animations.begin() + index);
            Fit();
            Layout();
            break;
        }
        index++;
    }
    FinishLayout();
    Thaw();
}

//void AnimPanel::PopulateAnimCtrls()
//{
//    Freeze();
//    ClearPanel();
//    // if (input->animations.empty())
//    //    input->animations.push_back(std::make_unique<Animation>());
//    auto sizer = new wxBoxSizer(wxVERTICAL);
//    intermediate->SetSizer(sizer);
//
//    newAnimButton = new wxButton(intermediate, ID_New_Anim, "New Animation");
//    AddDecoration(newAnimButton);
//    sizer->Add(newAnimButton);
//
//    AddAnimCtrl();
//
//    Thaw();
//}

void AnimPanel::UpdateComboBoxes()
{
    for (auto C : linkedCtrls)
        C->ReadLinked();
}

void AnimPanel::FinishLayout()
{
    intermediate->SetMinClientSize(wxSize(
        1100, ((wxCtrls.size() + linkedCtrls.size()) * (ROW_HEIGHT + 6))));
    intermediate->SetMaxClientSize(wxSize(
        1100, ((wxCtrls.size() + linkedCtrls.size()) * (ROW_HEIGHT + 6))));
    SetVirtualSize(1000, -1);

    auto scroll = GetVisibleBegin();
    ScrollToRowColumn(0, 0);

    auto RowCt = (wxCtrls.size() + linkedCtrls.size());
    SetRowCount(RowCt + RowCt / 4 + 1);

    FitInside();
    intermediate->FitInside();
    intermediate->Layout();
    SetColumnCount(40);
    Layout();
    ScrollToRowColumn(scroll);
}
