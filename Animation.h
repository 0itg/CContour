#pragma once
#include <complex>
#include <functional>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include "Commands.h"

typedef std::complex<double> cplx;

class Command;
class Contour;

// Executes Commands parameterized by a complex function f(t). There is no
// particular restriction on what the commands may do, but the animation system
// assumes that executing with a given value of t will produce the same result
// each time. The user is responsible for managing the state of any objects
// modified during the animation.

class Animation
{
    friend class boost::serialization::access;

  public:
    Animation() : f([](cplx c) { return c; }) {}
    void FrameAt(int t);
    void AddCommand(std::unique_ptr<Command> C)
    {
        commands.push_back(std::move(C));
    }
    void ClearCommands() { commands.clear(); }
    void SetFunction(std::function<cplx(double)> func) { f = func; }
    void SetPathContour(std::shared_ptr<Contour> C);
    auto GetPath() { return path; }
    bool IsEmpty() { return commands.empty(); }

    // if bounce, animation will go from t=0 to t=1, then t=1 to t=0, then
    // repeat. else, it will go from t=0 to t=1, then repeat.
    bool bounce     = false;
    int reverse     = 1;
    int duration_ms = 1000;
    // Stored internally as an offset from 0 to 1;
    double offset;

    // Stored menu selections for restoring the menus on load.
    int subjSel;
    int pathSel;
    int comSel;
    int handle;

    bool animateGrid = false;

  private:
    std::shared_ptr<Contour> path;
    std::vector<std::unique_ptr<Command>> commands;

    // Function must be parameterized by a variable 't', assumed to
    // range between 0 and 1.
    // std::function doesn't play well with serialization. A function can be
    // reconstructed from Contour* path, so we'll just do that for now.
    // That means any other type of function will be overwritten on save.
    std::function<cplx(double)> f;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& path;
        ar& bounce;
        ar& reverse;
        ar& duration_ms;
        ar& offset;
        ar& subjSel;
        ar& pathSel;
        ar& comSel;
        ar& handle;
        ar& animateGrid;

        SetPathContour(path);

        ar& commands;
    }
};
