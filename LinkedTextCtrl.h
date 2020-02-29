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
template <class T> class Symbol;

// Wrapper for wxTextCtrl which stores a pointer to its input's
// intended destination.
class LinkedCtrl {
 public:
   virtual void WriteLinked() = 0;
   virtual void ReadLinked()  = 0;
   virtual wxWindowID GetId() = 0;
   virtual bool Destroy()     = 0;
   };

class LinkedTextCtrl : public LinkedCtrl, virtual public wxTextCtrl {
      virtual void WriteLinked() = 0;
      virtual void ReadLinked()  = 0;
      virtual wxWindowID GetId() {
         return wxTextCtrl::GetId();
      }
      virtual bool Destroy() {
         return wxTextCtrl::Destroy();
      }
   };

class LinkedDoubleTextCtrl : public LinkedTextCtrl {
 public:
   LinkedDoubleTextCtrl() {}
   LinkedDoubleTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                  wxPoint defaultPos, wxSize defSize, int style, double* p)
       : wxTextCtrl(par, ID, str, defaultPos, defSize, style), src(p) {}
   virtual ~LinkedDoubleTextCtrl() {}

   virtual void Link(double* ptr) {
      src = ptr;
   }
   void WriteLinked();
   void ReadLinked() {
      ChangeValue(std::to_string(*src));
   }

 private:
   double* src;
};

class LinkedCtrlPointTextCtrl : public LinkedTextCtrl {
 public:
   LinkedCtrlPointTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                           wxPoint defaultPos, wxSize defSize, int style,
                           Contour* C, size_t index)
       : wxTextCtrl(par, ID, str, defaultPos, defSize, style), src(C),
         i(index) {}
   // void Link(std::vector<cplx>& ref) {
   //   dataVec = ref;
   //}
   void WriteLinked();
   void ReadLinked();

 private:
   Contour* src;
   size_t i;
};

class LinkedVarTextCtrl : public LinkedTextCtrl {
 public:
   LinkedVarTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                     wxPoint defaultPos, wxSize defSize, int style,
                     Symbol<cplx>* sym)
       : wxTextCtrl(par, ID, str, defaultPos, defSize, style), src(sym) {}
   // void Link(std::vector<cplx>& ref) {
   //   dataVec = ref;
   //}
   void WriteLinked();
   void ReadLinked();

 private:
   Symbol<cplx>* src;
};