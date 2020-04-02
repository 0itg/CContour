#include "Utilities.h"
#include "ComplexPlane.h"

double DistancePointToLine(cplx pt, cplx z1, cplx z2)
{
    double n = abs((z2.imag() - z1.imag()) * pt.real() -
                   (z2.real() - z1.real()) * pt.imag() + z2.real() * z1.imag() -
                   z1.real() * z2.imag());
    double d = abs(z2 - z1);
    if (d != 0)
        return n / d;
    else
        return abs(z2 - pt);
};

bool IsInsideBox(cplx pt, cplx z1, cplx z2)
{
    double Dz1z2 = abs(z2 - z1);
    double Dz1pt = abs(pt - z1);
    double Dz2pt = abs(pt - z2);
    return (Dz1pt < Dz1z2 && Dz2pt < Dz1z2);
};

// Drawing too far outside the screen incurs an unacceptable performance
// cost, so if necessary this can be used to
// draw lines clipped to panel boundaries.

void DrawClippedLine(wxPoint p1, wxPoint p2, wxDC* dc, ComplexPlane* canvas)
{
    const int PADDING = 4;
    wxPoint size(canvas->GetClientSize().x + PADDING,
                 canvas->GetClientSize().y + PADDING);

    auto IsLineOffscreen = [&](wxPoint p1, wxPoint p2) {
        return ((p1.x < -PADDING && p2.x < -PADDING) ||
                (p1.y < -PADDING && p2.y < -PADDING) ||
                (p1.x > size.x && p2.x > size.x) ||
                (p1.y > size.y && p2.y > size.y));
    };

    if (!IsLineOffscreen(p1, p2))
    {
        double dx         = p2.x - p1.x;
        double dy         = p2.y - p1.y;
        double slope      = NAN;
        double slopeRecip = NAN;

        if (dx)
        {
            slope = dy / dx;
            if (slope) slopeRecip = 1.0 / slope;
        }

        for (auto p : {p1, p2})
        {
            if (!isnan(slope))
            {
                if (p.x < -PADDING)
                {
                    p.y += slope * (-PADDING - p.x);
                    p.x = -PADDING;
                }
                else if (p.x > size.x)
                {
                    p.y += slope * (p.x - size.x);
                    p.x = size.x;
                }
            }
            else
            {
                if (p.y < -PADDING)
                    p.y = -PADDING;
                else if (p.y > size.y)
                    p.y = size.y;
            }
            if (!isnan(slopeRecip))
            {
                if (p.y < -PADDING)
                {
                    p.x += slopeRecip * (-PADDING - p.y);
                    p.y = -PADDING;
                }
                else if (p.y > size.y)
                {
                    p.x += slopeRecip * (p.y - size.y);
                    p.y = size.y;
                }
            }
            else
            {
                if (p.x < -PADDING)
                    p.x = -PADDING;
                else if (p.x > size.x)
                    p.x = size.x;
            }
        }
    }
    if (!IsLineOffscreen(p1, p2)) { dc->DrawLine(p1, p2); }
}

std::string removeExt(const std::string& str)
{
    size_t lastindex = str.find_last_of(".");
    if (lastindex != std::string::npos)
        return str.substr(0, lastindex);
    else
        return str;
};