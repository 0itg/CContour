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

class Contour;
class InputPlane;
class OutputPlane;
class LinkedTextCtrl;

class ToolPanel : public wxVScrolledWindow {
 public:
   ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size);
   // void PopulateCtrlPoints(Contour* contour);
   // void ClearControlPoints();
   void OnTextEntry(wxCommandEvent& event);
   void OnPaintEvent(wxPaintEvent& event);
   void PopulateAxisTextCtrls();
   void PopulateContourTextCtrls(Contour* C);
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

   void SetInputPlane(InputPlane* in) {
      input = in;
   }
   void SetOutputPlane(OutputPlane* out) {
      outputs.push_back(out);
   }
   const int SPACING = 48;

 private:
   std::vector<wxControl*> decorations;
   std::vector<LinkedTextCtrl*> controls;
   InputPlane* input;
   std::vector<OutputPlane*> outputs;
   // Contour* menuContour;

   wxDECLARE_EVENT_TABLE();
};

// Wrapper for wxTextCtrl which stores a pointer to its input's
// intended destination.
// Lazy implementation. This should be an abstract base class, with
// double version inheriting from it. Later.
class LinkedTextCtrl : virtual public wxTextCtrl {
 public:
   LinkedTextCtrl() {}
   LinkedTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                  wxPoint defaultPos, wxSize defSize, int style, double* p)
       : wxTextCtrl(par, ID, str, defaultPos, defSize, style), data(p) {}
   virtual ~LinkedTextCtrl() {}

   virtual void Link(double* ptr) {
      data = ptr;
   }
   virtual void WriteLinked();
   virtual void ReadLinked() {
      ChangeValue(std::to_string(*data));
   }

 private:
   double* data;
};

//
class LinkedCtrlPointTextCtrl : public LinkedTextCtrl {
 public:
   LinkedCtrlPointTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                          wxPoint defaultPos, wxSize defSize, int style,
                          Contour* C, size_t index)
       : wxTextCtrl(par, ID, str, defaultPos, defSize, style),
         parent(C), i(index) {
   }
   //void Link(std::vector<std::complex<double>>& ref) {
   //   dataVec = ref;
   //}
   void WriteLinked();
   void ReadLinked();

 private:
   Contour* parent;
   size_t i;
   //std::vector<std::complex<double>>& dataVec;
};