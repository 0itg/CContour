#include "LinkedTextCtrl.h"
#include "Contour.h"
#include "Parser.h"
#include "wx/richtooltip.h"

void LinkedCtrlPointTextCtrl::WriteLinked() {
   Parser<cplx> parser;
   try {
      cplx val = parser.Parse(GetValue()).eval();
      src->moveCtrlPoint(val, i);
   } catch (std::invalid_argument& func) {
      wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
      errormsg.ShowFor(this);
      ReadLinked();
   }
}

void LinkedCtrlPointTextCtrl::ReadLinked() {
   ChangeValue(std::to_string(src->GetCtrlPoint(i).real()) + " + " +
               std::to_string(src->GetCtrlPoint(i).imag()) + "i");
}

void LinkedTextCtrl::WriteLinked() {
   Parser<double> parser;
   try {
      double val = parser.Parse(GetValue()).eval();
      *src       = val;
   } catch (std::invalid_argument& func) {
      wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
      errormsg.ShowFor(this);
      ReadLinked();
   }
}

void LinkedVarTextCtrl::WriteLinked() {
   Parser<cplx> parser;
   try {
      cplx val = parser.Parse(GetValue()).eval();
      src->SetVal(val);
   } catch (std::invalid_argument& func) {
      wxRichToolTip errormsg(wxT("Invalid Input"), func.what());
      errormsg.ShowFor(this);
      ReadLinked();
   }
}

void LinkedVarTextCtrl::ReadLinked() {
   ChangeValue(std::to_string(src->eval().getVal().real()) + " + " +
               std::to_string(src->eval().getVal().imag()) + "i");
}