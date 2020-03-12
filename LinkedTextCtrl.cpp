#include "LinkedTextCtrl.h"
#include "Contour.h"
#include "Parser.h"
#include "wx/richtooltip.h"

void LinkedCtrlPointTextCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        cplx val = parser.Parse(textCtrl->GetValue()).eval();
        src->MoveCtrlPoint(val, i);
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

void LinkedCtrlPointTextCtrl::ReadLinked()
{
    textCtrl->ChangeValue(std::to_string(src->GetCtrlPoint(i).real()) + " + " +
                          std::to_string(src->GetCtrlPoint(i).imag()) + "i");
}

void LinkedCtrlPointTextCtrl::Add(cplx c)
{
    src->SetCtrlPoint(i, src->GetCtrlPoint(i) + c);
}

void LinkedDoubleTextCtrl::WriteLinked()
{
    Parser<double> parser;
    try
    {
        double val = parser.Parse(textCtrl->GetValue()).eval();
        *src       = val;
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

LinkedVarTextCtrl::LinkedVarTextCtrl(wxWindow* par, wxStandardID ID,
                                     wxString str, wxPoint defaultPos,
                                     wxSize defSize, int style,
                                     Symbol<cplx>* sym)
    : src(sym), LinkedCplxSpinCtrl(par, ID, str, defaultPos, defSize, style)
{
}

void LinkedVarTextCtrl::WriteLinked()
{
    Parser<cplx> parser;
    try
    {
        cplx val = parser.Parse(textCtrl->GetValue()).eval();
        src->SetVal(val);
    }
    catch (std::invalid_argument& func)
    {
        wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
        errormsg.ShowFor(textCtrl);
        ReadLinked();
    }
}

void LinkedVarTextCtrl::ReadLinked()
{
    textCtrl->ChangeValue(std::to_string(src->eval().GetVal().real()) + " + " +
                          std::to_string(src->eval().GetVal().imag()) + "i");
}

void LinkedVarTextCtrl::Add(cplx c)
{
    auto d = src->GetVal();
    src->SetVal(c + d);
}

LinkedCplxSpinCtrl::LinkedCplxSpinCtrl(wxWindow* par, wxStandardID ID,
                                       wxString str, wxPoint defaultPos,
                                       wxSize defSize, int style)
{
    panel    = new wxPanel(par, ID, defaultPos, defSize);
    textCtrl = new wxTextCtrl(panel, ID, str, defaultPos, defSize, style);
    auto h   = textCtrl->GetSize().y;
    reSpin   = new wxSpinButton(panel, -1, defaultPos, wxSize(h / 2, h), style);
    imSpin   = new wxSpinButton(panel, -1, defaultPos, wxSize(h / 2, h), style);

    constexpr int min = std::numeric_limits<int>::min();
    constexpr int max = std::numeric_limits<int>::max();
    reSpin->SetRange(min, max);
    imSpin->SetRange(min, max);
    auto sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(reSpin);
    sizer->Add(textCtrl, wxSizerFlags(1).Expand());
    sizer->Add(imSpin);
    panel->SetSizer(sizer);
}
