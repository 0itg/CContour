#include "ComplexPlane.h"
#include "OutputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include <complex>

std::complex<double> ComplexPlane::ScreenToComplex(wxPoint P)
{
    return std::complex<double>((double)P.x /
        GetClientSize().x * (axes.realMax - axes.realMin)
        + axes.realMin, (double)P.y / GetClientSize().y
        * (axes.imagMax - axes.imagMin) + axes.imagMin);
}

wxPoint ComplexPlane::ComplexToScreen(std::complex<double> C)
{
    return wxPoint((C.real() - axes.realMin) / (axes.realMax - axes.realMin)
        * GetClientSize().x, (C.imag() - axes.imagMin) /
        (axes.imagMax - axes.imagMin) * GetClientSize().y);
}

double ComplexPlane::LengthToScreen(double r)
{
    return r * GetClientSize().x / (axes.realMax - axes.realMin);
}

double ComplexPlane::ScreenToLength(double r)
{
    return r * (axes.realMax - axes.realMin) / GetClientSize().x;
}

//double Dist(wxPoint X, wxPoint Y)
//{
//    double Xdist = X.x - Y.x;
//    double Ydist = X.y - Y.y;
//    return sqrt(Xdist * Xdist + Ydist * Ydist);
//}
