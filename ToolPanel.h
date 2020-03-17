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
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/vscroll.h>

#include <complex>
#include <functional>

#include "Animation.h"

typedef std::complex<double> cplx;

class Contour;
class InputPlane;
class OutputPlane;
class LinkedCtrl;
template <class T> class Symbol;
template <class T> class ParsedFunc;

// Sidebar menu base class, intended for menus which can be dynamically
// populated with "decorations" (intended mainly for static text, but any
// wxControl-derived class could be stored there) and "controls" (must derive
// from LinkedCtrl. Outside classes may handle the population of controls, or a
// derived class may be used to provide that function.
class ToolPanel : public wxHVScrolledWindow
{
  public:
    ToolPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size);
    virtual ~ToolPanel();
    virtual void OnTextEntry(wxCommandEvent& event);
    virtual void OnSpinButtonUp(wxSpinEvent& event);
    virtual void OnSpinButtonDown(wxSpinEvent& event);
    void OnPaintEvent(wxPaintEvent& event);
    void OnSpinCtrlTextEntry(wxSpinDoubleEvent& event) { OnTextEntry(event); }
    void ClearPanel();

    void AddDecoration(wxWindow* D)
    {
        wxCtrls.push_back(D);
    }
    void AddLinkedCtrl(LinkedCtrl* L)
    {
        linkedCtrls.push_back(L);
    }
    auto GetDecoration(size_t i)
    {
        return wxCtrls[i];
    }
    auto GetLinkedCtrl(size_t i)
    {
        return linkedCtrls[i];
    }

    virtual wxCoord OnGetRowHeight(size_t row) const
    {
        return ROW_HEIGHT;
    }
    virtual wxCoord OnGetColumnWidth(size_t row) const
    {
        return ROW_HEIGHT;
    }

    virtual bool NeedsUpdate()   = 0;
    virtual void RefreshLinked() = 0;

    virtual void RePopulate() { lastPopulateFn(); }

    static constexpr int ROW_HEIGHT = 24;
    static constexpr int SPACING    = 2 * ROW_HEIGHT;
    const wxSize TEXTBOX_SIZE =
        wxSize(this->GetTextExtent("0.00000000 + 0.00000000i").x + 20,
               wxDefaultSize.y);

    wxPanel* intermediate;
    // wxDECLARE_EVENT_TABLE();
  protected:
    std::vector<wxWindow*> wxCtrls;
    std::vector<LinkedCtrl*> linkedCtrls;
    std::function<void(void)> lastPopulateFn = 0;
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
    void PopulateAxisTextCtrls();
    void PopulateContourTextCtrls(Contour* C);

    bool NeedsUpdate();
    void RefreshLinked();

    void SetInputPlane(InputPlane* in)
    {
        input = in;
    }
    void SetOutputPlane(OutputPlane* out)
    {
        outputs.push_back(out);
    }

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

    bool NeedsUpdate()
    {
        return true;
    }
    void RefreshLinked();

    void SetOutputPlane(OutputPlane* out)
    {
        output = out;
    }

    wxDECLARE_EVENT_TABLE();

  private:
    OutputPlane* output;
};

class AnimPanel : public ToolPanel
{
  public:
    AnimPanel(wxWindow* parent, int ID, wxPoint pos, wxSize size,
              InputPlane* in = nullptr)
        : ToolPanel(parent, ID, pos, size), input(in)
    {
    }
    void SetInputPlane(InputPlane* in)
    {
        input = in;
    }

    bool NeedsUpdate()
    {
        return true;
    }
    void RefreshLinked() {};

    void AddAnimCtrl();
    void OnAddAnimCtrl(wxCommandEvent& event) { AddAnimCtrl(); }
    void PopulateAnimCtrls();
    void UpdateComboBoxes();
    void FinishLayout();

    wxDECLARE_EVENT_TABLE();

  private:
    InputPlane* input;
};