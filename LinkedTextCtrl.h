#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/aui/aui.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/display.h>
#include <wx/spinbutt.h>
#include <wx/vscroll.h>

#include <complex>

typedef std::complex<double> cplx;

class Contour;
template <class T> class Symbol;

// Interface for GUI controls which write/read their values to/from a specified
// destination, for example, a wxTextCtrl which displays the current value of a
// parameter in the text box and allows the user to change it.
class LinkedCtrl
{
  public:
    virtual void WriteLinked()     = 0;
    virtual void ReadLinked()      = 0;
    virtual wxWindowID GetId()     = 0;
    virtual bool Destroy()         = 0;
    virtual wxWindow* GetCtrlPtr() = 0;
};

// Base class for LinkedCtrls which contain a wxTextCtrl.
class LinkedTextCtrl : public LinkedCtrl
{
  public:
    virtual void WriteLinked() = 0;
    virtual void ReadLinked()  = 0;
    virtual wxWindowID GetId()
    {
        return textCtrl->GetId();
    }
    virtual bool Destroy()
    {
        bool res = textCtrl->Destroy();
        delete this;
        return res;
    }
    virtual wxTextCtrl* GetCtrlPtr()
    {
        return textCtrl;
    }
    wxTextCtrl* textCtrl;
};

// Base class for LinkedCtrls containing a custom SpinCtrl-style
// control. Spin buttons on the left side increment/decrement the
// real part, while spin buttons on the right do the same for the
// imaginary part.
class LinkedCplxSpinCtrl : public LinkedCtrl
{
  public:
    LinkedCplxSpinCtrl(wxWindow* par, wxStandardID ID, wxString str,
                      wxPoint defaultPos, wxSize defSize, int style);
    virtual wxWindowID GetId()
    {
        return textCtrl->GetId();
    }
    virtual wxWindowID GetIdReSpin()
    {
        return reSpin->GetId();
    }
    virtual wxWindowID GetIdImSpin()
    {
        return imSpin->GetId();
    }
    virtual wxPanel* GetCtrlPtr()
    {
        return panel;
    }
    virtual void Add(cplx c) = 0;
    virtual bool Destroy()
    {
        bool res = panel->Destroy();
        delete this;
        return res;
    }

    wxPanel* panel;
    wxTextCtrl* textCtrl;
    wxSpinButton* reSpin;
    wxSpinButton* imSpin;
};

// Links to a single double.
class LinkedDoubleTextCtrl : public LinkedTextCtrl
{
  public:
    LinkedDoubleTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                         wxPoint defaultPos, wxSize defSize, int style,
                         double* p)
        : src(p)
    {
        textCtrl = new wxTextCtrl(par, ID, str, defaultPos, defSize, style);
    }
    void WriteLinked();
    void ReadLinked()
    {
        textCtrl->ChangeValue(std::to_string(*src));
    }

  private:
    double* src;
};

// Links to a Contour control point, given the a pointer to the Contour
// and the index of the point in its points vector.
class LinkedCtrlPointTextCtrl : public LinkedCplxSpinCtrl
{
  public:
    LinkedCtrlPointTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                            wxPoint defaultPos, wxSize defSize, int style,
                            Contour* C, size_t index)
        : src(C), i(index),
          LinkedCplxSpinCtrl(par, ID, str, defaultPos, defSize, style)
    {
    }
    void WriteLinked();
    void ReadLinked();
    virtual void Add(cplx c);

  private:
    Contour* src;
    size_t i;
};

// Links to a ParsedFunc variable. Takes a pointer to the symbol representing
// that variable. The pointer may be found in ParsedFunc.symbolStack (vector)
// or ParsedFunc.tokens (map).
class LinkedVarTextCtrl : public LinkedCplxSpinCtrl
{
  public:
    LinkedVarTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                      wxPoint defaultPos, wxSize defSize, int style,
                      Symbol<cplx>* sym);
    void WriteLinked();
    void ReadLinked();

    virtual void Add(cplx c);

  private:
    Symbol<cplx>* src;
};