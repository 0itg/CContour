#include "ToolPanel.h"
#include "InputPlane.h"
#include "OutputPlane.h"

// clang-format off
wxBEGIN_EVENT_TABLE(ToolPanel, wxVScrolledWindow)
EVT_TEXT_ENTER(wxID_ANY, ToolPanel::OnTextEntry)
EVT_PAINT(ToolPanel::OnPaintEvent)
wxEND_EVENT_TABLE();
// clang-format on

ToolPanel::ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
    : wxVScrolledWindow(parent, ID_ToolPanel, pos, size) {
   SetRowCount(20);
}

void ToolPanel::OnTextEntry(wxCommandEvent& event) {
   for (auto ctrl : controls) {
      if (ctrl->GetId() == event.GetId()) { ctrl->WriteLinked(); }
   }
   input->Update();
   input->Refresh();
   for (auto out : outputs) {
      out->Update();
      out->Refresh();
   }
}

void ToolPanel::OnPaintEvent(wxPaintEvent& event) {
   for (auto ctrl : controls) { ctrl->ReadLinked(); }
}

void ToolPanel::PopulateAxisTextCtrls() {
   int distFromTop   = 18;
   const int SPACING = 48;

   std::string buttonText[] = {
       "Real Min:", "Real Max:", "Imag Min:", "Imag Max:"};

   if (input != nullptr) {
      decorations.push_back(new wxStaticText(
          this, wxID_ANY, wxString(input->GetName() + ":"),
          wxDefaultPosition + wxSize(12, distFromTop), wxDefaultSize));
      distFromTop += SPACING;

      for (int i = 0; i < 4; i++) {
         double c = input->axes.c[i];
         decorations.push_back(new wxStaticText(
             this, wxID_ANY, wxString(buttonText[i]),
             wxDefaultPosition + wxSize(12, distFromTop - 18), wxDefaultSize));
         controls.push_back(new LinkedTextCtrl(
             this, wxID_ANY, wxString(std::to_string(c)),
             wxDefaultPosition + wxPoint(12, distFromTop), wxDefaultSize,
             wxTE_PROCESS_ENTER, input->axes.c + i));
         distFromTop += SPACING;
      }
   }
   if (!outputs.empty()) {
      for (int i = 0; i < 4 * outputs.size(); i++) {
         if (i % 4 == 0) {
            decorations.push_back(new wxStaticText(
                this, wxID_ANY, wxString(outputs[i]->GetName() + ":"),
                wxDefaultPosition + wxPoint(12, distFromTop), wxDefaultSize));
            distFromTop += SPACING;
         }
         double c = outputs[i / 4]->axes.c[i % 4];
         decorations.push_back(new wxStaticText(
             this, wxID_ANY, wxString(buttonText[i % 4]),
             wxDefaultPosition + wxSize(12, distFromTop - 18), wxDefaultSize));
         controls.push_back(new LinkedTextCtrl(
             this, wxID_ANY, wxString(std::to_string(c)),
             wxDefaultPosition + wxPoint(12, distFromTop), wxDefaultSize,
             wxTE_PROCESS_ENTER, outputs[i / 4]->axes.c + (i % 4)));
         distFromTop += SPACING;
      }
   }
}
