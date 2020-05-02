#pragma once
#include "ContourPolygon.h"
#include "Parser.h"

class ToolPanel;

typedef std::complex<double> cplx;

class ContourParametric : public ContourPolygon
{
    friend class boost::serialization::access;

public:
    ContourParametric() {}
    ContourParametric(std::string func, int res = 200,
                      wxColor col   = wxColor(0, 0, 0),
                      std::string n = "Parametric Curve", double tS = 0,
                      double tE = 1);
    Contour* Map(ParsedFunc<cplx>& g, int res) { return Contour::Map(g, res); }
    void Draw(wxDC* dc, ComplexPlane* canvas);
    bool IsPointOnContour(cplx pt, ComplexPlane* canvas, int pixPrecision = 4);

    // Mouse-driven editing is disabled.
    void AddPoint(cplx c) {}

    cplx Interpolate(double t);
    void SetFunction(std::string func) { f = parser.Parse(func); }
    auto GetFunctionPtr() { return &f; }
    void Finalize() { CalcCenter(); }
    bool IsDone() { return true; }

    // Mouse-driven editing is disabled.
    int OnCtrlPoint(cplx pt, ComplexPlane* canvas, int pixPrecision = 3)
    {
        return -1;
    }

    // Mouse-driven editing is disabled.
    int ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked)
    {
        return ACTION_IDLE;
    }
    // virtual CommandContourTranslate* CreateActionCommand(cplx c);

    virtual void PopulateMenu(ToolPanel* TP);
    virtual bool IsParametric() { return true; }
    ParsedFunc<cplx>* GetFunction() { return &f; }

    // Rotation and scaling are disabled, since the function would no longer
    // define the curve. The user can instead add a parameter to the function to
    // rotate or scale as desired.
    virtual bool RotationEnabled() { return false; }
    virtual bool ScalingEnabled() { return false; }
    virtual void Rotate(double angle, cplx pivot = cplx(INFINITY, INFINITY)) {}
    virtual void Rotate(cplx V, cplx pivot = cplx(INFINITY, INFINITY)) {}
    virtual void RotateAndScale(cplx V, cplx pivot = cplx(INFINITY, INFINITY))
    {
    }

private:
    double tStart = 0;
    double tEnd   = 1;
    Parser<cplx> parser;
    ParsedFunc<cplx> f;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Contour, ContourParametric>(
            *this);
        ar& parser;
        ar& tStart;
        ar& tEnd;
        ar& f;
    }
};

BOOST_CLASS_EXPORT_KEY(ContourParametric)