#pragma once
#include "ContourPolygon.h"
#include "Parser.h"

class ToolPanel;

typedef std::complex<double> cplx;

class ContourParametric : public ContourPolygon
{
public:
	ContourParametric() { }
	ContourParametric(std::string func, int res = 200, wxColor col = wxColor(0,0,0),
		std::string n = "Parametric Curve", double tS = 0, double tE = 1);
	ContourPolygon* Map(ParsedFunc<cplx>& g);

	// Mouse-driven editing is disabled.
	void AddPoint(cplx c) {}

	void Subdivide(int res);
	cplx Interpolate(double t);
	void SetFunction(std::string func) {
		f = parser.Parse(func);
	}
	void Finalize() { CalcCenter(); }

	// Mouse-driven editing is disabled.
	int OnCtrlPoint(cplx pt, ComplexPlane* canvas,
		int pixPrecision = 3) {
		return -1;
	}

	// Mouse-driven editing is disabled.
	void ActionNoCtrlPoint(cplx mousePos, cplx lastPointClicked) {}

	virtual void PopulateMenu(ToolPanel* TP);

private:
	double tStart = 0;
	double tEnd = 1;
	Parser<cplx> parser;
	ParsedFunc<cplx> f;
	//std::function<cplx(cplx)> f;
};

