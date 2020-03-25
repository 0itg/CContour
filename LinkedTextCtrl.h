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
#include <wx/spinctrl.h>
#include <wx/vscroll.h>

#include <complex>

typedef std::complex<double> cplx;

class Contour;
class ContourCircle;
class ContourParametric;
class ComplexPlane;
class InputPlane;
class ToolPanel;
class Animation;
class CommandHistory;
template <class T> class Symbol;
template <class T> class Parser;
template <class T> class ParsedFunc;

// Interface for GUI controls which write/read their values to/from a specified
// destination, for example, a wxTextCtrl which displays the current value of a
// parameter in the text box and allows the user to change it.
class LinkedCtrl
{
  public:
    virtual ~LinkedCtrl(){};
    virtual void WriteLinked()     = 0;
    virtual void ReadLinked()      = 0;
    virtual wxWindowID GetId()     = 0;
    virtual bool Destroy()         = 0;
    virtual wxWindow* GetCtrlPtr() = 0;
    virtual void UpdateCtrl()      = 0;
};

// Base class for LinkedCtrls which contain a wxTextCtrl.
class LinkedTextCtrl : public LinkedCtrl
{
  public:
    virtual void WriteLinked() = 0;
    virtual void ReadLinked()  = 0;
    virtual wxWindowID GetId() { return textCtrl->GetId(); }
    virtual bool Destroy()
    {
        bool res = textCtrl->Destroy();
        delete this;
        return res;
    }
    virtual wxTextCtrl* GetCtrlPtr() { return textCtrl; }
    virtual void UpdateCtrl() {}
    wxTextCtrl* textCtrl;
};

// Base class for LinkedCtrls which contain a wxSpinCtrlDouble.
class LinkedSpinCtrlDouble : public LinkedCtrl
{
  public:
    virtual void WriteLinked() = 0;
    virtual void ReadLinked()  = 0;
    virtual void UpdateCtrl() {}
    virtual wxWindowID GetId() { return textCtrl->GetId(); }
    virtual bool Destroy()
    {
        bool res = textCtrl->Destroy();
        delete this;
        return res;
    }
    virtual wxSpinCtrlDouble* GetCtrlPtr() { return textCtrl; }
    wxSpinCtrlDouble* textCtrl;
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
    virtual wxWindowID GetId() { return textCtrl->GetId(); }
    virtual wxWindowID GetIdReSpin() { return reSpin->GetId(); }
    virtual wxWindowID GetIdImSpin() { return imSpin->GetId(); }
    virtual wxPanel* GetCtrlPtr() { return panel; }
    virtual void Add(cplx c) = 0;
    virtual void UpdateCtrl() {}
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

  protected:
    virtual void RecordCommand(cplx c) {}
};

// Links to a single double.
class LinkedDoubleTextCtrl : public LinkedSpinCtrlDouble
{
  public:
    LinkedDoubleTextCtrl(wxWindow* par, wxWindowID ID, wxString str,
                         wxPoint defaultPos, wxSize defSize, int style,
                         double* p, double step = 0.1, double init = 0,
                         double min = -1e10, double max = 1e10)
        : src(p)
    {
        textCtrl = new wxSpinCtrlDouble(par, ID, str, defaultPos, defSize,
                                        style, min, max, init, step);
    }
    virtual void WriteLinked();
    virtual void ReadLinked() { textCtrl->SetValue(*src); }
    virtual double GetVal() { return *src; }

  protected:
    // Override in order to record a command in the history
    virtual void RecordCommand(cplx c) {}
    // Override if the command needs to be modified after entry.
    virtual void UpdateCommand(cplx c){};
    double* src;
};

// Links to a Contour control point, given the a pointer to the Contour
// and the index of the point in its points vector.
class LinkedCtrlPointTextCtrl : public LinkedCplxSpinCtrl
{
  public:
    LinkedCtrlPointTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                            wxPoint defaultPos, wxSize defSize, int style,
                            Contour* C, size_t index, CommandHistory* ch)
        : src(C), i(index), history(ch),
          LinkedCplxSpinCtrl(par, ID, str, defaultPos, defSize, style)
    {
    }
    void WriteLinked();
    void ReadLinked();
    virtual void Add(cplx c);

  private:
    virtual void RecordCommand(cplx c);
    Contour* src;
    size_t i;
    CommandHistory* history;
};

// Links to a ParsedFunc variable. Takes a pointer to the symbol representing
// that variable. The pointer may be found in ParsedFunc.symbolStack (vector)
// or ParsedFunc.tokens (map).
class LinkedVarTextCtrl : public LinkedCplxSpinCtrl
{
  public:
    LinkedVarTextCtrl(wxWindow* par, wxStandardID ID, wxString str,
                      wxPoint defaultPos, wxSize defSize, int style,
                      Symbol<cplx>* sym, CommandHistory* ch);
    void WriteLinked();
    void ReadLinked();

    virtual void Add(cplx c);

  private:
    Symbol<cplx>* src;
    CommandHistory* history;
};

class LinkedParametricFuncCtrl : public LinkedTextCtrl
{
  public:
    LinkedParametricFuncCtrl(ToolPanel* par, wxStandardID ID, wxString str,
                             wxPoint defaultPos, wxSize defSize, int style,
                             ContourParametric* cp);
    void WriteLinked();
    void ReadLinked();

  private:
    ToolPanel* TP;
    ContourParametric* C;
    ParsedFunc<cplx>* src;
};

// Specialized LinkedDoubleTextCtrl which records CommandAxesSet on entry
class LinkedAxisCtrl : public LinkedDoubleTextCtrl
{
  public:
    LinkedAxisCtrl(wxWindow* par, wxWindowID ID, wxString str,
                   wxPoint defaultPos, wxSize defSize, int style,
                   ComplexPlane* P, double* p, double step = 0.1,
                   double init = 0, double min = -1e10, double max = 1e10)
        : LinkedDoubleTextCtrl(par, ID, str, defaultPos, defSize, style, p,
                               step, init, min, max),
          plane(P)
    {
    }
    // void WriteLinked();

  private:
    virtual void RecordCommand(cplx c);
    virtual void UpdateCommand(cplx c);
    ComplexPlane* plane;
};

// Specialized LinkedDoubleTextCtrl which records
// CommandContourEditRadius on entry
class LinkedRadiusCtrl : public LinkedDoubleTextCtrl
{
  public:
    LinkedRadiusCtrl(wxWindow* par, wxWindowID ID, wxString str,
                     wxPoint defaultPos, wxSize defSize, int style,
                     ContourCircle* P, CommandHistory* ch, double* p,
                     double step = 0.1, double init = 0, double min = -1e10,
                     double max = 1e10)
        : LinkedDoubleTextCtrl(par, ID, str, defaultPos, defSize, style, p,
                               step, init, min, max),
          C(P), history(ch)
    {
    }

  private:
    virtual void RecordCommand(cplx c);
    ContourCircle* C;
    CommandHistory* history;
};

class AnimCtrl : public LinkedCtrl
{
  public:
    AnimCtrl(wxWindow* parent, InputPlane* input, Animation* a);
    ~AnimCtrl()
    {
        delete durationCtrl;
        delete durOffsetCtrl;
    }

    virtual void WriteLinked();
    virtual void ReadLinked();
    virtual wxWindowID GetId() { return panel->GetId(); };
    virtual bool Destroy()
    {
        bool res = panel->Destroy();
        delete this;
        return res;
    }
    virtual wxWindow* GetCtrlPtr() { return panel; };
    virtual void UpdateCtrl();

  private:
    void PopulateHandleMenu();
    wxPanel* panel;
    wxBoxSizer* sizer;
    wxComboBox* commandMenu;
    wxComboBox* subjectMenu;
    wxComboBox* pathMenu;
    wxComboBox* handleMenu;
    LinkedDoubleTextCtrl* durationCtrl;
    LinkedDoubleTextCtrl* durOffsetCtrl;
    wxCheckBox* reverseCtrl;
    wxCheckBox* bounceCtrl;
    wxButton* removeButton;
    wxArrayString contourChoices;
    wxArrayString commandChoices;
    wxArrayString handleChoices;

    InputPlane* input;
    Animation* anim;
    double dur    = 3.0;
    double offset = 0;
    int reverse   = 1; // set to -1 to reverse t;
};