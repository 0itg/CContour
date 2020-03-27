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
    ContourPolygon* Map(ParsedFunc<cplx>& g);

    // Mouse-driven editing is disabled.
    void AddPoint(cplx c) {}

    void Subdivide(int res);
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