#include "DialogCreateParametricCurve.h"

DialogCreateParametricCurve::DialogCreateParametricCurve(wxWindow* parent,
                                                         std::string defName)
    : sizer(wxVERTICAL), wxDialog(parent, wxID_ANY, "Create a Parametric Curve")
{
    wxSizerFlags flags(1);
    flags.Expand().Border(wxALL, 3);

    enterName.Create(this, wxID_ANY, "Curve name");
    sizer.Add(&enterName, flags);
    nameCtrl.Create(this, wxID_ANY, defName);
    sizer.Add(&nameCtrl, flags);

    enterFunc.Create(this, wxID_ANY, "Function f(t)");
    sizer.Add(&enterFunc, flags);
    funcCtrl.Create(this, wxID_ANY, "4*t*exp(4pi*i*t)");
    sizer.Add(&funcCtrl, flags);

    enterTStart.Create(this, wxID_ANY, "t Start");
    sizer.Add(&enterTStart, flags);
    tStartCtrl.Create(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize,
                      wxSP_ARROW_KEYS, -1e10, 1e10, 0, 0.1);
    sizer.Add(&tStartCtrl, flags);

    enterTEnd.Create(this, wxID_ANY, "t End");
    sizer.Add(&enterTEnd, flags);
    tEndCtrl.Create(this, wxID_ANY, "1.0", wxDefaultPosition, wxDefaultSize,
                    wxSP_ARROW_KEYS, -1e10, 1e10, 1, 0.1);
    sizer.Add(&tEndCtrl, flags);
    sizer.Add(CreateButtonSizer(wxOK | wxCANCEL),
              wxSizerFlags(1).Border(wxALL, 3).CenterHorizontal());
    SetSizerAndFit(&sizer);
}
