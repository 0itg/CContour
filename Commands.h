#pragma once
#pragma warning(disable : 4996)
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>

typedef std::complex<double> cplx;

// Commands intended for use in animation and a future undo/redo
// system. Basically just packages up a void function and its arguments
// in a polymorphic object. Derived classes can specify a destination and
// enclose a function which returns to that in exex() if necessary.
// SetPositionParam() is used to to pass a cplx to the command, intended
// For updating the animation parameter, but maybe useful for user editing.

class Contour;
class InputPlane;
class OutputPlane;
struct Axes;

class Command
{
  public:
    virtual ~Command() {}
    virtual void exec() = 0;
    virtual void SetPositionParam(cplx c) {};
};

class CommandContourTranslate : public Command
{
  public:
    CommandContourTranslate(Contour* s, cplx n, cplx o)
        : newPos(n), oldPos(o), subject(s)
    {
    }

    void exec();
    void SetPositionParam(cplx c)
    {
        newPos = c;
    }

  private:
    cplx newPos;
    cplx oldPos;
    Contour* subject;
};

// Places the specified contour at the specified coordinates.
// Defaults to moving the center to that point, but any other point may
// be specified by its index.
class CommandContourPlaceAt : public Command
{
  public:
    CommandContourPlaceAt(Contour* s, cplx n, int pt = -1)
        : newPos(n), subject(s), point(pt)
    {
    }

    void exec();
    void SetPositionParam(cplx c)
    {
        newPos = c;
    }

  private:
    int point;
    cplx newPos;
    Contour* subject;
};

class CommandContourMovePoint : public Command
{
  public:
    CommandContourMovePoint(Contour* s, cplx n, int i)
        : newPos(n), index(i), subject(s)
    {
    }

    void exec();
    void SetPositionParam(cplx c)
    {
        newPos = c;
    }

  private:
    cplx newPos;
    int index;
    Contour* subject;
};

class CommandContourAddPoint : public Command
{
  public:
    CommandContourAddPoint(Contour* s, cplx n) : mPos(n), subject(s) {}

    void exec();
    void SetPositionParam(cplx c)
    {
        mPos = c;
    }

  private:
    cplx mPos;
    Contour* subject;
};

class CommandContourRemovePoint : public Command
{
  public:
    CommandContourRemovePoint(Contour* s, int i) : index(i), subject(s) {}

    void exec();

  private:
    int index;
    Contour* subject;
};

class CommandContourSubdivide : public Command
{
  public:
    CommandContourSubdivide(Contour* s, int i) : res(i), subject(s) {}

    void exec();

  private:
    int res;
    Contour* subject;
};

class CommandInputPlaneCreateContour : public Command
{
  public:
    CommandInputPlaneCreateContour(InputPlane* s, wxPoint p)
        : mPos(p), subject(s)
    {
    }

    void exec();

  private:
    wxPoint mPos;
    InputPlane* subject;
};

class CommandAxesSet : public Command
{
  public:
    CommandAxesSet(Axes* a, double rMin, double rMax, double iMin, double iMax)
        : subject(a), realMin(rMin), realMax(rMax), imagMin(iMin), imagMax(iMax)
    {
    }

    void exec();

  private:
    Axes* subject;
    double realMin, realMax, imagMin, imagMax;
};

class CommandAxesReset : public Command
{
  public:
    CommandAxesReset(Axes* a) : subject(a) {}

    void exec();

  private:
    Axes* subject;
};

class CommandOutputPlaneFuncEntry : public Command
{
  public:
    CommandOutputPlaneFuncEntry(OutputPlane* a, std::string s) : subject(a), func(s) {}

    void exec();

  private:
    OutputPlane* subject;
    std::string func;
};