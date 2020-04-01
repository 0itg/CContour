#include "Animation.h"
#include "Contour.h"

void Animation::FrameAt(int t)
{
    if (path)
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
}

void Animation::SetPathContour(std::shared_ptr<Contour> C)
{
    path = C;
    f = [&](double t) {return path->Interpolate(reverse * t + offset); };
}
