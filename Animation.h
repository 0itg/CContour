#pragma once
#include <complex>
#include <functional>

#include "Commands.h"

typedef std::complex<double> cplx;

class Command;

// Executes Commands parameterized by a complex function f(t). There is no
// particular restriction on what the commands may do, but the animation system
// assumes that executing with a given value of t will produce the same result
// each time. The user is responsible for managing the state of any objects
// modified during the animation.

class Animation
{
  public:
    Animation() : f([](cplx c) { return c; }) {}
    Animation(std::function<cplx(double)> func) : f(func) {}
    void FrameAt(int t);
    void Reset();
    void AddCommand(std::unique_ptr<Command> C)
    {
        commands.push_back(std::move(C));
    }
    void ClearCommands() { commands.clear(); }
    void SetFunction(std::function<cplx(double)> func) { f = func; }
    bool IsEmpty() { return commands.empty(); }

    // if bounce, animation will go from t=0 to t=1, then t=1 to t=0, then
    // repeat. else, it will go from t=0 to t=1, then repeat.
    bool bounce     = false;
    int duration_ms = 1000;

  private:
    std::vector<std::unique_ptr<Command>> commands;

    // Function must be parameterized by a variable called 't', assumed to
    // range between 0 and 1 for now;
    std::function<cplx(double)> f;
};
