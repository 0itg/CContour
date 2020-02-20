#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/display.h>
//#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/vscroll.h>

class Contour;
class InputPlane;
class OutputPlane;
template <typename T> class LinkedTextCtrl;

class ToolPanel : public wxVScrolledWindow {
 public:
   ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size);
   // void PopulateCtrlPoints(Contour* contour);
   // void ClearControlPoints();
   void OnTextEntry(wxCommandEvent& event);
   void OnPaintEvent(wxPaintEvent& event);
   void PopulateAxisTextCtrls();
   virtual wxCoord OnGetRowHeight(size_t row) const {
      return 24;
   }

   void SetInputPlane(InputPlane* in) {
      input = in;
   }
   void SetOutputPlane(OutputPlane* out) {
      outputs.push_back(out);
   }

 private:
   std::vector<wxControl*> decorations;
   std::vector<LinkedTextCtrl<double>*> controls;
   InputPlane* input;
   std::vector<OutputPlane*> outputs;
   // Contour* menuContour;

   wxDECLARE_EVENT_TABLE();
};

// Wrapper for wxTextCtrl which stores a pointer to its input's
// intended destination. Template currently useless, but maybe if I
// figure out a convenient way to convert a string to a generic type...
template <typename T> class LinkedTextCtrl : public wxTextCtrl {
 public:
   LinkedTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                  wxPoint defaultPos, wxSize defSize, int style, T* p)
       : wxTextCtrl(par, ID, str, defaultPos, defSize, style), data(p) {}
   void Link(T* ptr) {
      data = ptr;
   }
   void WriteLinked() {
      *data = std::stod((std::string)GetValue());
   }
   void ReadLinked() {
      ChangeValue(std::to_string(*data));
   }

 private:
   T* data;
};
