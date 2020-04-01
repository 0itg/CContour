#pragma once
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/spinctrl.h>
#include <string>

class DialogCreateParametricCurve : public wxDialog
{
public:
	DialogCreateParametricCurve(wxWindow* parent);
	~DialogCreateParametricCurve() { SetSizer(NULL, false); }

	wxBoxSizer sizer;
	wxTextCtrl nameCtrl, funcCtrl;
	wxSpinCtrlDouble tStartCtrl, tEndCtrl;
	wxStaticText enterName, enterFunc, enterTStart, enterTEnd;
};