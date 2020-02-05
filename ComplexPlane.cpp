#include "ComplexPlane.h"
#include "OutputPanel.h"
#include "ContourCircle.h"
#include "ContourPolygon.h"
#include "ContourRect.h"
#include <complex>

void Axes::Draw(wxDC* dc)
{
    using namespace std::complex_literals;
    wxPoint center = parent->ComplexToScreen(0);
    wxPen pen(2);
    pen.SetColour(*wxBLACK);
    dc->SetPen(pen);
    dc->CrossHair(center);

    wxSize size = parent->GetClientSize();
    std::complex<double> cMark = 0;
    wxPoint mark;
    while (cMark.real() < realMax)
    {
        cMark += reStep;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x, mark.y + HASH_WIDTH / 2),
            wxPoint(mark.x, mark.y - HASH_WIDTH / 2));
    }
    cMark = 0;
    while (cMark.real() > realMin)
    {
        cMark -= reStep;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x, mark.y + HASH_WIDTH / 2),
            wxPoint(mark.x, mark.y - HASH_WIDTH / 2));
    }
    cMark = 0;
    while (cMark.imag() < imagMax)
    {
        cMark += imStep * 1i;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x + HASH_WIDTH / 2, mark.y),
            wxPoint(mark.x - HASH_WIDTH / 2, mark.y));
    }
    cMark = 0;
    while (cMark.imag() > imagMin)
    {
        cMark -= imStep * 1i;
        mark = parent->ComplexToScreen(cMark);
        dc->DrawLine(wxPoint(mark.x + HASH_WIDTH / 2, mark.y),
            wxPoint(mark.x - HASH_WIDTH / 2, mark.y));
    }
}

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
