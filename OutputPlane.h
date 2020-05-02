#pragma once

#include "ComplexPlane.h"
#include "Grid.h"
#include "Parser.h"
#include "ToolPanel.h"
#include "ContourPoint.h"

#include <complex>
//#include <atomic>
#include <wx/spinctrl.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

class InputPlane;
class ContourPolygon;
class ContourPoint;
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
    //void OnMouseRightUp(wxMouseEvent& mouse);
    // void OnMouseRightDown(wxMouseEvent& mouse);
    void OnMouseMoving(wxMouseEvent& mouse);
    void OnPaint(wxPaintEvent& paint);
    void OnGridResCtrl(wxSpinEvent& event);
    void OnGridResCtrl(wxCommandEvent& event);
    void OnFunctionEntry(wxCommandEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    void SetInputPlane(InputPlane* input) { in = input; }

    void Pan(wxPoint mousePos);
    void Zoom(wxPoint mousePos, int zoomSteps);

    void EnterFunction(std::string s);
    void CopyFunction(ParsedFunc<cplx> g);
    auto GetFunc() { return f; }

    void MarkAllForRedraw();
    void SetFuncInput(wxTextCtrl* fIn) { funcInput = fIn; }
    auto GetFuncInput() { return funcInput; }
    void RefreshFuncText() { funcInput->SetValue(f.GetInputText()); }
    void SetVarPanel(VariableEditPanel* var) { varPanel = var; }

    void CalcZerosAndPoles();

    // t = -1 means don't use the parameter.
    bool DrawFrame(wxBitmap& image, double t = -1);

    int GetRes();
    Grid* GetGrid() { return &tGrid; }

    ParsedFunc<cplx> f;

    //std::atomic_bool calculating_zeros;
private:
    Parser<cplx> parser;
    TransformedGrid tGrid;

    InputPlane* in;
    wxTextCtrl* funcInput;
    VariableEditPanel* varPanel;

    std::vector<std::unique_ptr<ContourPoint>> zerosAndPoles;

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

//class atomic_bool_setter
//{
//public:
//    atomic_bool_setter(std::atomic_bool& a) : b(a) { b.store(true); }
//    ~atomic_bool_setter() { b.store(false); }
//private:
//    std::atomic_bool& b;
//};