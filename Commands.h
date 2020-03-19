#pragma once
#pragma warning(disable : 4996)
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <complex>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

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

enum enum_commands
{
    COMMAND_PLACE_AT = 0,
    COMMAND_SET_PT
};

class Command
{
  public:
    virtual ~Command() {}
    virtual void exec() = 0;
    virtual void SetPositionParam(cplx c){};
    // virtual cplx GetPositionParam() { return 0; };
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {}
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Command)

class CommandContourTranslate : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourTranslate() = default;
    CommandContourTranslate(Contour* s, cplx n, cplx o)
        : newPos(n), oldPos(o), subject(s)
    {
    }

    void exec();
    void SetPositionParam(cplx c) { newPos = c; }

  private:
    cplx newPos;
    cplx oldPos;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Command>(*this);
        ar& newPos;
        ar& oldPos;
        ar& subject;
    }
};

// Places the specified contour at the specified coordinates.
// Defaults to moving the center to that point, but any other point may
// be specified by its index.
class CommandContourPlaceAt : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourPlaceAt() = default;
    CommandContourPlaceAt(Contour* s, cplx n = 0, int pt = -1)
        : newPos(n), subject(s), point(pt)
    {
    }

    void exec();
    void SetPositionParam(cplx c) { newPos = c; }
    // virtual cplx GetPositionParam()
    //{
    //    return newPos;
    //}

  private:
    int point;
    cplx newPos;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& newPos;
        ar& point;
        ar& subject;
    }
};

class CommandContourMovePoint : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourMovePoint() = default;
    CommandContourMovePoint(Contour* s, cplx n, int i)
        : newPos(n), index(i), subject(s)
    {
    }

    void exec();
    void SetPositionParam(cplx c) { newPos = c; }
    // virtual cplx GetPositionParam()
    //{
    //    return newPos;
    //}

  private:
    cplx newPos;
    int index;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& newPos;
        ar& index;
        ar& subject;
    }
};

class CommandContourAddPoint : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourAddPoint() = default;
    CommandContourAddPoint(Contour* s, cplx n) : mPos(n), subject(s) {}

    void exec();
    void SetPositionParam(cplx c) { mPos = c; }

  private:
    cplx mPos;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& mPos;
        ar& subject;
    }
};

class CommandContourRemovePoint : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourRemovePoint() = default;
    CommandContourRemovePoint(Contour* s, int i) : index(i), subject(s) {}

    void exec();

  private:
    int index;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& index;
        ar& subject;
    }
};

class CommandContourSubdivide : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourSubdivide() = default;
    CommandContourSubdivide(Contour* s, int i) : res(i), subject(s) {}

    void exec();

  private:
    int res;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& res;
        ar& subject;
    }
};

class CommandInputPlaneCreateContour : public Command
{
    friend class boost::serialization::access;
  public:
      CommandInputPlaneCreateContour() = default;
    CommandInputPlaneCreateContour(InputPlane* s, wxPoint p)
        : mPos(p), subject(s)
    {
    }

    void exec();

  private:
    wxPoint mPos;
    InputPlane* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& mPos;
        ar& subject;
    }
};

class CommandAxesSet : public Command
{
    friend class boost::serialization::access;
  public:
      CommandAxesSet() = default;
    CommandAxesSet(Axes* a, double rMin, double rMax, double iMin, double iMax)
        : subject(a), realMin(rMin), realMax(rMax), imagMin(iMin), imagMax(iMax)
    {
    }

    void exec();

  private:
    Axes* subject;
    double realMin, realMax, imagMin, imagMax;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& realMin;
        ar& realMax;
        ar& imagMin;
        ar& imagMax;
    }
};

class CommandAxesReset : public Command
{
    friend class boost::serialization::access;
  public:
      CommandAxesReset() = default;
    CommandAxesReset(Axes* a) : subject(a) {}

    void exec();

  private:
    Axes* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
    }
};

class CommandOutputPlaneFuncEntry : public Command
{
    friend class boost::serialization::access;
  public:
      CommandOutputPlaneFuncEntry() = default;
    CommandOutputPlaneFuncEntry(OutputPlane* a, std::string s)
        : subject(a), func(s)
    {
    }

    void exec();

  private:
    OutputPlane* subject;
    std::string func;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& func;
    }
};

namespace boost {
    namespace serialization {
        template<class Archive>
        void serialize(Archive& ar, wxPoint& p, const unsigned int version)
        {
            ar& p.x;
            ar& p.y;
        }
    }
}

BOOST_CLASS_EXPORT_KEY(CommandOutputPlaneFuncEntry)
BOOST_CLASS_EXPORT_KEY(CommandAxesReset)
BOOST_CLASS_EXPORT_KEY(CommandAxesSet)
BOOST_CLASS_EXPORT_KEY(CommandInputPlaneCreateContour)
BOOST_CLASS_EXPORT_KEY(CommandContourSubdivide)
BOOST_CLASS_EXPORT_KEY(CommandContourRemovePoint)
BOOST_CLASS_EXPORT_KEY(CommandContourAddPoint)
BOOST_CLASS_EXPORT_KEY(CommandContourMovePoint)
BOOST_CLASS_EXPORT_KEY(CommandContourPlaceAt)
BOOST_CLASS_EXPORT_KEY(CommandContourTranslate)