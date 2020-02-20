#pragma once
#include <wx/aui/aui.h>
#include <wx/clrpicker.h>

class InputPlane;
class OutputPlane;

class MainWindowFrame : public wxFrame {
 public:
   MainWindowFrame(const wxString& title, const wxPoint& pos,
                   const wxSize& size,
                   const long style = wxDEFAULT_FRAME_STYLE);
   ~MainWindowFrame() {
      aui.UnInit();
   }

 private:
   InputPlane* input;
   OutputPlane* output;
   void OnExit(wxCommandEvent& event);
   void OnAbout(wxCommandEvent& event);
   void OnToolbarContourSelect(wxCommandEvent& event);
   void OnColorPicked(wxColourPickerEvent& col);
   void OnButtonColorRandomizer(wxCommandEvent& event);
   void OnButtonPaintbrush(wxCommandEvent& event);
   void OnFunctionEntry(wxCommandEvent& event);
   void OnGridResCtrl(wxSpinEvent& event);
   void OnGridResCtrl(wxCommandEvent& event);
   void OnContourResCtrl(wxSpinEvent& event);
   void OnContourResCtrl(wxCommandEvent& event);
   void OnShowAxes_ShowGrid(wxCommandEvent& event);

   wxAuiManager aui;

   wxDECLARE_EVENT_TABLE();
};
