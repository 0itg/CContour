#include "ToolPanel.h"
#include "Contour.h"
#include "InputPlane.h"
#include "OutputPlane.h"
#include "Parser.h"

#include <wx/richtooltip.h>

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
      if (ctrl->GetId() == event.GetId()) {
         ctrl->WriteLinked();
      }
   }
   input->Update();
   input->Refresh();
   for (auto out : outputs) {
      out->MarkAllForRedraw();
      out->Update();
      out->Refresh();
   }
}

void ToolPanel::OnPaintEvent(wxPaintEvent& event) {
   if (input->GetState() > -1 || input->movedViewPort == true)
       for (auto ctrl : controls) {
          ctrl->ReadLinked();
       }
}

void ToolPanel::PopulateAxisTextCtrls() {
   ClearPanel();
   SetRowCount(4 * (1 + outputs.size()) + 5);
   int distFromTop = 18;

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

void ToolPanel::PopulateContourTextCtrls(Contour* C) {
   ClearPanel();
   SetRowCount(2 * C->points.size());

   C->PopulateMenu(this);
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

void LinkedCtrlPointTextCtrl::WriteLinked() {
   Parser<std::complex<double>> parser;
   try {
      std::complex<double> val = parser.Parse(GetValue()).eval();
      parent->moveCtrlPoint(val, i);
   }
   catch (std::invalid_argument& func) {
      wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
      errormsg.ShowFor(this);
      ReadLinked();
   }
}

void LinkedCtrlPointTextCtrl::ReadLinked() {
   ChangeValue(std::to_string(parent->GetCtrlPoint(i).real()) + " + " +
               std::to_string(parent->GetCtrlPoint(i).imag()) + "i");
}

void LinkedTextCtrl::WriteLinked() {
   Parser<double> parser;
   try {
      double val = parser.Parse(GetValue()).eval();
      *data = val;
   }
   catch (std::invalid_argument& func) {
   wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
   errormsg.ShowFor(this);
   ReadLinked();
   }
}