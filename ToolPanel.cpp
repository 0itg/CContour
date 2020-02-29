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

wxBEGIN_EVENT_TABLE(AxisAndCtrlPointPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_PAINT(ToolPanel::OnPaintEvent)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(VariableEditPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_PAINT(VariableEditPanel::OnPaintEvent)
wxEND_EVENT_TABLE();
// clang-format on

ToolPanel::ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
    : wxVScrolledWindow(parent, ID_ToolPanel, pos, size) {
   SetRowCount(20);
}

void ToolPanel::OnTextEntry(wxCommandEvent& event) {
   for (auto ctrl : controls) {
      if (ctrl->GetId() == event.GetId()) {
         ctrl->WriteLinked();
      }
   }
   RefreshLinked();
}

void ToolPanel::OnPaintEvent(wxPaintEvent& event) {
   if (NeedsUpdate()) // && !controls.empty())
   {
      for (auto ctrl : controls) {
         ctrl->ReadLinked();
      }
   }
}

void AxisAndCtrlPointPanel::PopulateAxisTextCtrls() {
   Freeze();

   ClearPanel();
   SetRowCount(4 * (1 + outputs.size()) + 5);
   int distFromTop = 18;

   std::string buttonText[] = {
       "Real Min:", "Real Max:", "Imag Min:", "Imag Max:"};

   if (input != nullptr) {
      decorations.push_back(new wxStaticText(
          this, wxID_ANY, wxString(input->GetName() + ":"),
          wxDefaultPosition + wxSize(12, distFromTop), TEXTBOX_SIZE));
      distFromTop += SPACING;

      for (int i = 0; i < 4; i++) {
         double c = input->axes.c[i];
         decorations.push_back(new wxStaticText(
             this, wxID_ANY, wxString(buttonText[i]),
             wxDefaultPosition + wxSize(12, distFromTop - 18), TEXTBOX_SIZE));
         controls.push_back(new LinkedDoubleTextCtrl(
             this, wxID_ANY, wxString(std::to_string(c)),
             wxDefaultPosition + wxPoint(12, distFromTop), TEXTBOX_SIZE,
             wxTE_PROCESS_ENTER, input->axes.c + i));
         distFromTop += SPACING;
      }
   }
   if (!outputs.empty()) {
      for (int i = 0; i < 4 * outputs.size(); i++) {
         if (i % 4 == 0) {
            decorations.push_back(new wxStaticText(
                this, wxID_ANY, wxString(outputs[i]->GetName() + ":"),
                wxDefaultPosition + wxPoint(12, distFromTop), TEXTBOX_SIZE));
            distFromTop += SPACING;
         }
         double c = outputs[i / 4]->axes.c[i % 4];
         decorations.push_back(new wxStaticText(
             this, wxID_ANY, wxString(buttonText[i % 4]),
             wxDefaultPosition + wxSize(12, distFromTop - 18), TEXTBOX_SIZE));
         controls.push_back(new LinkedDoubleTextCtrl(
             this, wxID_ANY, wxString(std::to_string(c)),
             wxDefaultPosition + wxPoint(12, distFromTop), TEXTBOX_SIZE,
             wxTE_PROCESS_ENTER, outputs[i / 4]->axes.c + (i % 4)));
         distFromTop += SPACING;
      }
   }
   Thaw();
}

void AxisAndCtrlPointPanel::PopulateContourTextCtrls(Contour* C) {
   Freeze();
   ClearPanel();
   SetRowCount(2 * C->GetPointCount());
   C->PopulateMenu(this);
   Thaw();
}

bool AxisAndCtrlPointPanel::NeedsUpdate() {
   return (input->GetState() > -1 || input->movedViewPort == true);
}

void AxisAndCtrlPointPanel::RefreshLinked() {
   input->Update();
   input->Refresh();
   for (auto out : outputs) {
      out->MarkAllForRedraw();
      out->Update();
      out->Refresh();
   }
}

void ToolPanel::ClearPanel() {
   for (auto C : controls) {
      C->Destroy();
   }
   for (auto D : decorations) {
      D->Destroy();
   }
   controls.clear();
   decorations.clear();
}

void VariableEditPanel::RefreshLinked() {
   output->MarkAllForRedraw();
   output->Update();
   output->Refresh();
   output->movedViewPort = true;
}

void VariableEditPanel::PopulateVarTextCtrls(ParsedFunc<cplx>& F) {
   Freeze();
   ClearPanel();
   int distFromTop = 18;
   AddDecoration(new wxStaticText(this, wxID_ANY, wxString("Parameters :"),
                                  wxDefaultPosition + wxSize(12, distFromTop),
                                  wxDefaultSize));
   distFromTop += 24;

   std::vector<Symbol<cplx>*> vars = F.GetVars();
   SetRowCount(2 * vars.size());

   for (auto v : vars) {
      // "z" still hardcoded to be the independent variable.
      if (v->GetToken() == "z")
         continue;
      cplx val = v->eval().getVal();
      std::string c =
          std::to_string(val.real()) + " + " + std::to_string(val.imag()) + "i";
      AddDecoration(new wxStaticText(
          this, wxID_ANY, wxString(v->GetToken()),
          wxDefaultPosition + wxSize(12, distFromTop), TEXTBOX_SIZE));
      AddLinkedTextCtrl(new LinkedVarTextCtrl(
          this, wxID_ANY, c, wxDefaultPosition + wxPoint(12, distFromTop + 18),
          TEXTBOX_SIZE, wxTE_PROCESS_ENTER, v));
      distFromTop += SPACING;
   }
   Thaw();
}

void VariableEditPanel::OnPaintEvent(wxPaintEvent& event) {
   for (auto ctrl : controls) {
      ctrl->ReadLinked();
   }
}
