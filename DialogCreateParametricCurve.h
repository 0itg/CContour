#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <string>
#include <wx/spinctrl.h>

class DialogCreateParametricCurve : public wxDialog
{
    public:
    DialogCreateParametricCurve(wxWindow* parent,
                                std::string defName = "Parametric Curve");
    ~DialogCreateParametricCurve() { SetSizer(NULL, false); }

    wxBoxSizer sizer;
    wxTextCtrl nameCtrl, funcCtrl;
    wxSpinCtrlDouble tStartCtrl, tEndCtrl;
    wxStaticText enterName, enterFunc, enterTStart, enterTEnd;
};