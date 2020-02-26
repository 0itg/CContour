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

#include <complex>

typedef std::complex<double> cplx;

class Contour;
class InputPlane;
class OutputPlane;
class LinkedTextCtrl;
template <class T> class Symbol;
template <class T> class ParsedFunc;

class ToolPanel : public wxVScrolledWindow {
 public:
   ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size);
   void OnTextEntry(wxCommandEvent& event);
   void OnPaintEvent(wxPaintEvent& event);
   void ClearPanel();

   void AddDecoration(wxControl* D) {
      decorations.push_back(D);
   }
   void AddLinkedTextCtrl(LinkedTextCtrl* L) {
      controls.push_back(L);
   }

   virtual wxCoord OnGetRowHeight(size_t row) const {
      return 24;
   }

   virtual bool NeedsUpdate()   = 0;
   virtual void RefreshLinked() = 0;
   //void OnEraseEvent(wxEraseEvent& erase){}

   static constexpr int SPACING = 48;
   const wxSize TEXTBOX_SIZE    = wxSize(
       this->GetTextExtent("0.000000 + 0.000000i").x + 20, wxDefaultSize.y);
   //wxDECLARE_EVENT_TABLE();
 protected:
   std::vector<wxControl*> decorations;
   std::vector<LinkedTextCtrl*> controls;
};

class AxisAndCtrlPointPanel : public ToolPanel {
 public:
   AxisAndCtrlPointPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
       : ToolPanel(parent, ID, pos, size) {}
   void PopulateAxisTextCtrls();
   void PopulateContourTextCtrls(Contour* C);

   bool NeedsUpdate();
   void RefreshLinked();

   void SetInputPlane(InputPlane* in) {
      input = in;
   }
   void SetOutputPlane(OutputPlane* out) {
      outputs.push_back(out);
   }

   wxDECLARE_EVENT_TABLE();

 private:
   InputPlane* input;
   std::vector<OutputPlane*> outputs;
};

class VariableEditPanel : public ToolPanel {
 public:
   VariableEditPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size)
       : ToolPanel(parent, ID, pos, size) {}
   void PopulateVarTextCtrls(ParsedFunc<cplx>& F);

   void OnPaintEvent(wxPaintEvent& event);

   bool NeedsUpdate() {
      return true;
   }
   void RefreshLinked();

   void SetOutputPlane(OutputPlane* out) {
      output = out;
   }

   wxDECLARE_EVENT_TABLE();

 private:
   OutputPlane* output;
};