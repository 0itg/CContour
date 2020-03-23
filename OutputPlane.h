#pragma once

#include "ComplexPlane.h"
#include "Grid.h"
#include "Parser.h"
#include "ToolPanel.h"

#include <complex>
#include <wx/spinctrl.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

class InputPlane;
class ContourPolygon;
class TransformedGrid;

// Right Panel in UI. Displays the mapping of input point sets under
// some complex function.

typedef std::complex<double> cplx;

class OutputPlane : public ComplexPlane
{
    friend class boost::serialization::access;
    friend class InputPlane;

  public:
    OutputPlane() {}
    OutputPlane(wxWindow* parent, InputPlane* In,
                const std::string& name = "Output");

    void OnMouseLeftUp(wxMouseEvent& mouse);
    // void OnMouseRightUp(wxMouseEvent& mouse);
    // void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);
    void OnGridResCtrl(wxSpinEvent& event);
    void OnGridResCtrl(wxCommandEvent& event);
    void OnFunctionEntry(wxCommandEvent& event);

    void Pan(wxPoint mousePos);
    void Zoom(wxPoint mousePos, int zoomSteps);

    void EnterFunction(std::string s);

    void MarkAllForRedraw();
    void SetFuncInput(wxTextCtrl* fIn) { funcInput = fIn; }
    void RefreshFuncText() { funcInput->SetValue(f.GetInputText()); }
    void SetVarPanel(VariableEditPanel* var) { varPanel = var; }

    int GetRes();

    ParsedFunc<cplx> f;

  private:
    Parser<cplx> parser;
    TransformedGrid tGrid;

    InputPlane* in;
    wxTextCtrl* funcInput;
    VariableEditPanel* varPanel;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<ComplexPlane>(*this);
        ar& parser;
        ar& tGrid;
        ar& f;
    }
    wxDECLARE_EVENT_TABLE();
};