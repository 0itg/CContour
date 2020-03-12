#pragma once
#include <complex>
#include <functional>

#include "Commands.h"

typedef std::complex<double> cplx;

//class Contour;

// Executes Commands parameterized by a complex function f(t) every frame,
// advancing t each time. There is no particular restriction on what the 
// commands may do. The user is responsible for managing the state of any
// objects modified during the animation.

class Animation
{
  public:
    Animation(/*Contour* C, */ std::function<cplx(cplx)> func) : f(func) {}
    void NextFrame();
    void FrameAt(cplx c);
    void Reset()
    {
        FrameAt(0);
    };
    void AddCommand(std::unique_ptr<Command> C)
    {
        commands.push_back(std::move(C));
    }
  private:
    //Contour* contour;

    std::vector<std::unique_ptr<Command>> commands;
    
    // Function must be parameterized by a variable called 't', assumed to
    // range between 0 and 1 for now; 
	std::function<cplx(cplx)> f;

    // Parameter. Intended to be used as a real number, but the parser wants
    // a cplx anyway. Complex steps may be convenient for some things, like
    // Linear motion.
    cplx t; 
    cplx tStep = 1.0 / 600;
    cplx tStart = 0;
    cplx tEnd   = 1;

    // if bounce, animation will go from t=0 to t=1, then t=1 to t=0, then
    // repeat. else, it will go from t=0 to t=1, then repeat.
    bool bounce = false;
};
