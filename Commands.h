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

#include <map>
#include <string>

typedef std::complex<double> cplx;

// Commands intended for use in animation and a future undo/redo
// system. Basically just packages up a void function and its arguments
// in a polymorphic object. Derived classes can specify a destination and
// enclose a function which returns to that in exex() if necessary.
// SetPositionParam() is used to to pass a cplx to the command, intended
// For updating the animation parameter, but maybe useful for user editing.

class Contour;
class ContourCircle;
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
    friend class boost::serialization::access;
  public:
    virtual ~Command() {}
    virtual void exec() = 0;
    // If a command is not invertible, it must store the state of the relevant
    // parts of the application and restore them.
    virtual void undo() = 0;
    //Used for animations.
    virtual void SetPositionParam(cplx c){};
private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {}
};
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Command)

// Simple undo/redo system. 
class CommandHistory
{
public:
    void undo();
    void redo();
    void SetMenu(wxMenu* m) { menu = m; }
    void RecordCommand(std::unique_ptr<Command> C);
    void UpdateLastCommand(cplx c);
    void PopCommand();
private:
    std::vector<std::unique_ptr<Command>> history;
    size_t index;
    wxMenu* menu; // For enabling/disabling undo/redo buttons
};

class CommandNull : public Command
{
    friend class boost::serialization::access;
public:
    void exec() {}
    void undo() {}
};

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
    void undo();
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
      CommandContourPlaceAt(Contour* s, cplx n = 0, int pt = -1);

    void exec();
    void undo();
    void SetPositionParam(cplx c) { newPos = c; }

  private:
    int point;
    cplx oldPos;
    cplx newPos;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& newPos;
        ar& oldPos;
        ar& point;
        ar& subject;
    }
};

class CommandContourMovePoint : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourMovePoint() = default;
      CommandContourMovePoint(Contour* s, cplx n, int i);
      CommandContourMovePoint(Contour* s, cplx n, int i, cplx old);

    void exec();
    void undo();
    void SetPositionParam(cplx c) { newPos = c; }

  private:
    cplx newPos;
    cplx oldPos;
    int index;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& newPos;
        ar& oldPos;
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
    void undo();
    void SetPositionParam(cplx c) { mPos = c; }

  private:
    cplx mPos;
    int index;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& mPos;
        ar& index;
        ar& subject;
    }
};

class CommandContourRemovePoint : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourRemovePoint() = default;
      CommandContourRemovePoint(Contour* s, int i);

    void exec();
    void undo();

  private:
    int index;
    cplx oldPos;
    Contour* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& index;
        ar& oldPos;
        ar& subject;
    }
};

class CommandContourEditRadius : public Command
{
    friend class boost::serialization::access;
public:
    CommandContourEditRadius() = default;
    CommandContourEditRadius(ContourCircle* s, double rad);
    void exec();
    void undo();
    void SetPositionParam(cplx c);
private:
    ContourCircle* subject;
    double radius;
    double oldRad;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& radius;
        ar& oldRad;
        ar& subject;
    }
};

// May eliminate this command, as it's not something the user does.
// Currently used in the animation system, though.
class CommandContourSubdivide : public Command
{
    friend class boost::serialization::access;
  public:
      CommandContourSubdivide() = default;
    CommandContourSubdivide(Contour* s, int i) : res(i), subject(s) {}

    void exec();
    // I assume there is no real need to restore the previous state,
    // since it should be overwritten next time it's needed.
    void undo() {} 

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

//class CommandInputPlaneCreateContour : public Command
//{
//    friend class boost::serialization::access;
//  public:
//      CommandInputPlaneCreateContour() = default;
//      CommandInputPlaneCreateContour(InputPlane* s, wxPoint p);
//
//    void exec();
//    void undo();
//
//  private:
//    wxPoint mPos;
//    int index;
//    InputPlane* subject;
//    template <class Archive>
//    void serialize(Archive& ar, const unsigned int version)
//    {
//        ar& mPos;
//        ar& index;
//        ar& subject;
//    }
//};

class CommandAddContour : public Command
{
    friend class boost::serialization::access;
public:
    CommandAddContour() = default;
    CommandAddContour(InputPlane* s, std::shared_ptr<Contour> c)
        : subject(s), C(c) {}

    void exec();
    void undo();

private:
    InputPlane* subject;
    std::shared_ptr<Contour> C;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& C;
    }
};

class CommandRemoveContour : public Command
{
    friend class boost::serialization::access;
public:
    CommandRemoveContour() = default;
    CommandRemoveContour(InputPlane* s, size_t i);

    void exec();
    void undo();

private:
    InputPlane* subject;
    size_t index;
    std::shared_ptr<Contour> C;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& C;
        ar& index;
    }
};

class CommandContourColorSet : public Command
{
    friend class boost::serialization::access;
public:
    CommandContourColorSet() = default;
    CommandContourColorSet(Contour* s, wxColor col);

    void exec();
    void undo();

private:
    Contour* subject;
    wxColor color;
    wxColor oldColor;

    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar& color;
        ar& oldColor;
        ar& subject;
    }
};

class CommandAxesSet : public Command
{
    friend class boost::serialization::access;
  public:
      CommandAxesSet() = default;
      CommandAxesSet(Axes* a, double rMin, double rMax, double iMin, double iMax);

    void exec();
    void undo();

  private:
    Axes* subject;
    double newBounds[4];
    double oldBounds[4];
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& newBounds;
        ar& oldBounds;
    }
};

class CommandAxesReset : public Command
{
    friend class boost::serialization::access;
  public:
      CommandAxesReset() = default;
      CommandAxesReset(Axes* a);

    void exec();
    void undo();

  private:
      double oldBounds[4];
    Axes* subject;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& oldBounds;
    }
};

class CommandOutputPlaneFuncEntry : public Command
{
    friend class boost::serialization::access;
  public:
      CommandOutputPlaneFuncEntry() = default;
      CommandOutputPlaneFuncEntry(OutputPlane* a, std::string s);

    void exec();
    void undo();

  private:
    OutputPlane* subject;
    std::string func;
    std::string oldfunc;
    std::map<std::string, cplx> oldVars;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& subject;
        ar& func;
        ar& oldfunc;
        ar& oldVars;
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
//BOOST_CLASS_EXPORT_KEY(CommandInputPlaneCreateContour)
BOOST_CLASS_EXPORT_KEY(CommandAddContour)
BOOST_CLASS_EXPORT_KEY(CommandContourSubdivide)
BOOST_CLASS_EXPORT_KEY(CommandContourEditRadius)
BOOST_CLASS_EXPORT_KEY(CommandContourRemovePoint)
BOOST_CLASS_EXPORT_KEY(CommandContourAddPoint)
BOOST_CLASS_EXPORT_KEY(CommandContourMovePoint)
BOOST_CLASS_EXPORT_KEY(CommandContourPlaceAt)
BOOST_CLASS_EXPORT_KEY(CommandContourTranslate)