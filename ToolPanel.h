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
#include <wx/spinctrl.h>
#include <wx/vscroll.h>

#include <complex>
#include <functional>

#include "Animation.h"

typedef std::complex<double> cplx;

class Contour;
class InputPlane;
class OutputPlane;
class LinkedCtrl;
class CommandHistory;
template <class T> class Symbol;
template <class T> class ParsedFunc;

// Sidebar menu base class, intended for menus which can be dynamically
// populated with "decorations" (intended mainly for static text, but any
// wxControl-derived class could be stored there) and "controls" (must derive
// from LinkedCtrl. Outside classes may handle the population of controls, or a
// derived class may be used to provide that function.
class ToolPanel : public wxScrolledWindow
{
  public:
    ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size);
    virtual ~ToolPanel();
    virtual void OnTextEntry(wxCommandEvent& event);
    virtual void OnSpinButtonUp(wxSpinEvent& event);
    virtual void OnSpinButtonDown(wxSpinEvent& event);
    void OnPaintEvent(wxPaintEvent& event);
    virtual void OnSpinCtrlTextEntry(wxSpinDoubleEvent& event) { OnTextEntry(event); }
    void ClearPanel();

    void AddwxCtrl(wxWindow* D) { wxCtrls.push_back(D); }
    void AddLinkedCtrl(LinkedCtrl* L) { linkedCtrls.push_back(L); }
    auto GetwxCtrl(size_t i) { return wxCtrls[i]; }
    auto GetLinkedCtrl(size_t i) { return linkedCtrls[i]; }

    virtual wxCoord OnGetRowHeight(size_t row) const { return ROW_HEIGHT; }
    virtual wxCoord OnGetColumnWidth(size_t row) const { return ROW_HEIGHT; }

    virtual bool NeedsUpdate()   = 0;
    virtual void RefreshLinked() = 0;

    virtual void RePopulate() { lastPopulateFn(); }
    void SetCommandHistory(CommandHistory* h) { history = h; }

    static constexpr int ROW_HEIGHT = 24;

    wxPanel* intermediate;
  protected:
    std::vector<wxWindow*> wxCtrls;
    std::vector<LinkedCtrl*> linkedCtrls;
    std::function<void(void)> lastPopulateFn = 0;
    CommandHistory* history;
};

// Panel which dynamically shows either Axis controls or Contour controls,
// Depending on whether or not a contour is selected.
class NumCtrlPanel : public ToolPanel
{
  public:
    NumCtrlPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size,
                 InputPlane* in = nullptr, OutputPlane* out = nullptr)
        : ToolPanel(parent, ID, pos, size), input(in)
    {
        SetOutputPlane(out);
    }
    void OnTextEntry(wxCommandEvent& event);

    void PopulateAxisTextCtrls();
    void PopulateContourTextCtrls(Contour* C);

    bool NeedsUpdate();
    void RefreshLinked();

    void SetInputPlane(InputPlane* in) { input = in; }
    void SetOutputPlane(OutputPlane* out) { outputs.push_back(out); }

    wxDECLARE_EVENT_TABLE();

  private:
    InputPlane* input;
    std::vector<OutputPlane*> outputs;
};

// Panel which shows all variables used in current function and allows
// Editing of the values. Variables are defined automatically when
// unrecognized text is used in a function.
class VariableEditPanel : public ToolPanel
{
  public:
    VariableEditPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size,
                      OutputPlane* out = nullptr)
        : ToolPanel(parent, ID, pos, size), output(out)
    {
    }

    void PopulateVarTextCtrls(ParsedFunc<cplx>& F);

    void OnPaintEvent(wxPaintEvent& event);

    bool NeedsUpdate() { return true; }
    void RefreshLinked();

    void SetOutputPlane(OutputPlane* out) { output = out; }

    wxDECLARE_EVENT_TABLE();

  private:
    OutputPlane* output;
};

class AnimPanel : public ToolPanel
{
  public:
      AnimPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size,
          InputPlane* in = nullptr);
    void SetInputPlane(InputPlane* in) { input = in; }

    bool NeedsUpdate() { return true; }
    void RefreshLinked(){};

    // Adds a new animation to input and creates an animCtrl for this panel;
    void AddAnimation();
    // Creates a new animCtrl, using the last item in input's animations vector.
    void AddAnimCtrl(int index = -1);
    void OnButtonNewAnim(wxCommandEvent& event) { AddAnimation(); }
    void OnRemoveAnim(wxCommandEvent& event);
    //void OnTextEntry(wxCommandEvent& event);
    //void OnSpinCtrlTextEntry(wxSpinDoubleEvent& event) { OnTextEntry(event); }
    void PopulateAnimCtrls();
    void UpdateComboBoxes();
    void FinishLayout();

    wxDECLARE_EVENT_TABLE();

  private:
    InputPlane* input;
    wxButton* newAnimButton;
};