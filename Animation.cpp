#include "Animation.h"

void Animation::NextFrame() {
    FrameAt(t);
    t += tStep;

    if (bounce && (abs(t - tStart) < 0.000001 || abs(t-tEnd) < 0.000001))
        tStep *= -1;
    else if (abs(t - tEnd) < 0.000001)
        t = tStart;
}

void Animation::FrameAt(cplx c)
{
    for (auto& C : commands)
    {
        C->SetPositionParam(f(c));
        C->exec();
    }
}
