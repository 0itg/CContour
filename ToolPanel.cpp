#include "ToolPanel.h"
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
EVT_PAINT(ToolPanel::OnPaintEvent)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(VariableEditPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_PAINT(VariableEditPanel::OnPaintEvent)
wxEND_EVENT_TABLE();
// clang-format on

ToolPanel::ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
    : wxVScrolledWindow(parent, ID, pos, size)
{
    SetRowCount(20);
    wxBoxSizer* siz = new wxBoxSizer(wxVERTICAL);
    intermediate    = new wxPanel(this);
    siz->Add(intermediate, wxSizerFlags(1).Expand());
    SetSizer(siz);
}

ToolPanel::~ToolPanel()
{
    intermediate->Destroy();
    for (auto C : controls)
        delete C;
}

void ToolPanel::OnTextEntry(wxCommandEvent& event)
{
    for (auto ctrl : controls)
    {
        if (ctrl->GetId() == event.GetId())
        {
            ctrl->WriteLinked();
        }
    }
    RefreshLinked();
}

void ToolPanel::OnPaintEvent(wxPaintEvent& event)
{
    if (NeedsUpdate()) // && !controls.empty())
    {
        for (auto ctrl : controls)
        {
            ctrl->ReadLinked();
        }
    }
}

void NumCtrlPanel::PopulateAxisTextCtrls()
{
    Freeze();

    ClearPanel();
    SetRowCount(20);
    std::string buttonText[] = {
        "Real Min:", "Real Max:", "Imag Min:", "Imag Max:"};
    auto sizer = new wxBoxSizer(wxVERTICAL);
    wxSizerFlags sizerFlags(1);
    sizerFlags.Expand().Border(wxLEFT | wxRIGHT, 3);

    if (input != nullptr)
    {
        decorations.push_back(new wxStaticText(
            intermediate, wxID_ANY, wxString(input->GetName() + ":"),
            wxDefaultPosition, wxDefaultSize));
        sizer->Add(decorations.back(), sizerFlags);

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
            sizer->Add(decorations.back(), sizerFlags);
            sizer->Add(controls.back()->GetCtrlPtr(), sizerFlags);
        }
    }
    if (!outputs.empty())
    {
        for (int i = 0; i < 4 * outputs.size(); i++)
        {
            if (i % 4 == 0)
            {
                decorations.push_back(
                    new wxStaticText(intermediate, wxID_ANY,
                                     wxString(outputs[i]->GetName() + ":"),
                                     wxDefaultPosition, wxDefaultSize));
                sizer->Add(decorations.back(), sizerFlags);
            }
            double c = outputs[i / 4]->axes.c[i % 4];
            AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                           wxString(buttonText[i % 4]),
                                           wxDefaultPosition, wxDefaultSize));
            AddLinkedCtrl(new LinkedDoubleTextCtrl(
                intermediate, wxID_ANY, wxString(std::to_string(c)),
                wxDefaultPosition, TEXTBOX_SIZE, wxTE_PROCESS_ENTER,
                outputs[i / 4]->axes.c + (i % 4)));
            sizer->Add(decorations.back(), sizerFlags);
            sizer->Add(controls.back()->GetCtrlPtr(), sizerFlags);
        }
    }
    intermediate->SetSizer(sizer);
    // intermediate->SetMinClientSize(wxSize(-1, (4 * SPACING + ROW_HEIGHT) * (1
    // + outputs.size())));
    intermediate->SetMaxClientSize(wxSize(GetClientSize().x, -1));
    intermediate->Layout();
    intermediate->FitInside();
    intermediate->SetMaxClientSize(
        wxSize(-1, (4 * SPACING + ROW_HEIGHT) * (1 + outputs.size())));
    SetVirtualSize(
        wxSize(-1, (4 * SPACING + ROW_HEIGHT) * (1 + outputs.size())));

    Layout();
    Thaw();
}

void NumCtrlPanel::PopulateContourTextCtrls(Contour* C)
{
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
    for (auto C : controls)
    {
        C->Destroy();
    }
    for (auto D : decorations)
    {
        D->Destroy();
    }
    controls.clear();
    decorations.clear();
    if (intermediate != nullptr)
    {
        if (auto size = intermediate->GetSizer(); size != nullptr)
        {
            intermediate->SetSizer(NULL, true);
        }
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
    AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                   wxString("Parameters :"), wxDefaultPosition,
                                   wxDefaultSize));
    sizer->Add(decorations.back(), sizerFlags);

    std::vector<Symbol<cplx>*> vars = F.GetVars();
    SetRowCount(2 * vars.size());

    for (auto v : vars)
    {
        if (v->GetToken() == "z")
            continue;
        cplx val      = v->eval().GetVal();
        std::string c = std::to_string(val.real()) + " + " +
                        std::to_string(val.imag()) + "i";
        AddDecoration(new wxStaticText(intermediate, wxID_ANY,
                                       wxString(v->GetToken()),
                                       wxDefaultPosition, wxDefaultSize));
        AddLinkedCtrl(new LinkedVarTextCtrl(intermediate, wxID_ANY, c,
                                            wxDefaultPosition, wxDefaultSize,
                                            wxTE_PROCESS_ENTER, v));
        sizer->Add(decorations.back(), sizerFlags);
        sizer->Add(controls.back()->GetCtrlPtr(), sizerFlags);
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
    for (auto ctrl : controls)
    {
        ctrl->ReadLinked();
    }
}
