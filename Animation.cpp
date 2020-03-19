#include "Animation.h"
#include "Commands.h"
#include "Contour.h"

void Animation::FrameAt(int t)
{
    if (bounce)
    {
        auto max = 2 * duration_ms;
        t %= max;
        if (t > duration_ms) t = max - t;
    }
    else
        t %= duration_ms;

    double T = (double)t / duration_ms;
    for (auto& C : commands)
    {
        C->SetPositionParam(f(T));
        C->exec();
    }
}

void Animation::Reset()
{
    // for (auto& C : commands)
    //    C->reset();
}

void Animation::SetPathContour(Contour* C)
{
    path = C;
    f = [&](double t) {
        return path->Interpolate(reverse * t + offset);
    };
}
