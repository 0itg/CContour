#pragma once
#pragma warning(disable : 4996)
#define WXUSINGDLL
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <complex>

#include <map>
#include <string>

#include "Parser.h"

typedef std::complex<double> cplx;

// Commands intended for use in animation and a future undo/redo
// system. Basically just packages up a void function and its arguments
// in a polymorphic object. Derived classes can specify a destination and
// enclose a function which returns to that in exex() if necessary.
// SetPositionParam() is used to to pass a cplx to the command, intended
// For updating the animation parameter, but maybe useful for user editing.

class Contour;
class ContourCircle;
class ContourParametric;
class ComplexPlane;
class InputPlane;
class OutputPlane;
class Animation;
class Grid;
struct Axes;
template <class T> class ParsedFunc;

enum enum_commands
{
    COMMAND_PLACE_AT = 0,
    COMMAND_EDIT_VAR,
    COMMAND_SET_PT,
    COMMAND_ROT_AND_SCALE,
    COMMAND_ROT,
    COMMAND_SCALE
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
    // Used for animations.
    virtual void SetPositionParam(cplx c){};

private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    }
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
    void UpdateLastCommand(cplx c = 0);
    // Command* GetCurrentCommand() { if (!history.empty() && index > 0) return
    // history[index-1].get(); else return nullptr; };
    void PopCommand();
    void Clear();

private:
    std::vector<std::unique_ptr<Command>> history;
    size_t index = 0;
    wxMenu* menu; // For enabling/disabling undo/redo buttons
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
        ar& boost::serialization::base_object<Command>(*this);
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

class CommandContourSetPoint : public Command
{
    friend class boost::serialization::access;

public:
    CommandContourSetPoint() = default;
    CommandContourSetPoint(Contour* s, cplx n, int i);
    CommandContourSetPoint(Contour* s, cplx n, int i, cplx old);

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

class CommandContourRotateAndScale : public Command
{
    friend class boost::serialization::access;

public:
    CommandContourRotateAndScale() = default;
    CommandContourRotateAndScale(Contour* s, cplx c = 1,
                                 cplx piv = cplx(INFINITY, INFINITY));

    void exec();
    void undo();
    virtual void SetPositionParam(cplx c)
    {
        Vlast = V;
        V     = c;
    }

protected:
    cplx V;
    cplx Vlast = 1;
    cplx pivot;
    Contour* subject;

private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& V;
        ar& Vlast;
        ar& pivot;
        ar& subject;
    }
};

class CommandContourRotate : public CommandContourRotateAndScale
{
    friend class boost::serialization::access;

public:
    CommandContourRotate() = default;
    CommandContourRotate(Contour* s, cplx c = 1,
                         cplx piv = cplx(INFINITY, INFINITY))
        : CommandContourRotateAndScale(s, c / abs(c), piv)
    {
    }

    virtual void SetPositionParam(cplx c)
    {
        Vlast = V;
        V     = c / abs(c);
    }

private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<CommandContourRotateAndScale>(
            *this);
    }
};

class CommandContourScale : public CommandContourRotateAndScale
{
    friend class boost::serialization::access;

public:
    CommandContourScale() = default;
    CommandContourScale(Contour* s, cplx c = 1,
                        cplx piv = cplx(INFINITY, INFINITY))
        : CommandContourRotateAndScale(s, abs(c), piv)
    {
    }

    virtual void SetPositionParam(cplx c)
    {
        Vlast = V;
        V     = abs(c);
    }

private:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<CommandContourRotateAndScale>(
            *this);
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
//class CommandContourSubdivide : public Command
//{
//    friend class boost::serialization::access;
//
//public:
//    CommandContourSubdivide() = default;
//    CommandContourSubdivide(Contour* s, int i) : res(i), subject(s) {}
//
//    void exec();
//    // I assume there is no real need to restore the previous state,
//    // since it should be overwritten next time it's needed.
//    void undo() {}
//
//private:
//    int res;
//    Contour* subject;
//
//    template <class Archive>
//    void serialize(Archive& ar, const unsigned int version)
//    {
//        ar& boost::serialization::base_object<Command>(*this);
//        ar& res;
//        ar& subject;
//    }
//};

class CommandAddContour : public Command
{
    friend class boost::serialization::access;

public:
    CommandAddContour() = default;
    CommandAddContour(InputPlane* s, std::shared_ptr<Contour> c)
        : subject(s), C(c)
    {
    }

    void exec();
    void undo();

private:
    InputPlane* subject;
    std::shared_ptr<Contour> C;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
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
        ar& boost::serialization::base_object<Command>(*this);
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
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
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
    CommandAxesSet(ComplexPlane* parent, double rMin, double rMax, double iMin,
                   double iMax);
    CommandAxesSet(ComplexPlane* parent, Grid* grid = nullptr);

    void exec();
    void undo();
    void SetPositionParam(cplx c);

private:
    ComplexPlane* parent;
    Grid* grid;
    Axes* subject;
    double newBounds[4];
    double oldBounds[4];

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& parent;
        ar& subject;
        ar& newBounds;
        ar& oldBounds;
        ar& grid;
    }
};

class CommandAxesReset : public Command
{
    friend class boost::serialization::access;

public:
    CommandAxesReset() = default;
    CommandAxesReset(ComplexPlane* par);

    void exec();
    void undo();

private:
    double oldBounds[4];
    ComplexPlane* parent;
    Grid* grid;
    Axes* subject;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& subject;
        ar& oldBounds;
        ar& grid;
    }
};

class CommandParametricFuncEntry : public Command
{
    friend class boost::serialization::access;

public:
    CommandParametricFuncEntry() = default;
    CommandParametricFuncEntry(ContourParametric* C, ParsedFunc<cplx> g);

    void exec();
    void undo();

private:
    ContourParametric* subject;
    ParsedFunc<cplx> oldfunc;
    ParsedFunc<cplx> newfunc;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& subject;
        ar& newfunc;
        ar& oldfunc;
    }
};

class CommandOutputFuncEntry : public Command
{
    friend class boost::serialization::access;

public:
    CommandOutputFuncEntry() = default;
    CommandOutputFuncEntry(ParsedFunc<cplx> g, OutputPlane* par);

    void exec();
    void undo();

private:
    OutputPlane* parent;
    ParsedFunc<cplx> oldfunc;
    ParsedFunc<cplx> newfunc;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& newfunc;
        ar& oldfunc;
        ar& parent;
    }
};

class CommandEditVar : public Command
{
    friend class boost::serialization::access;

public:
    CommandEditVar() = default;
    CommandEditVar(std::string tok, cplx c, ParsedFunc<cplx>* f);

    void exec();
    void undo();
    virtual void SetPositionParam(cplx c) { newVal = c; }

private:
    std::string token;
    cplx newVal;
    cplx oldVal;
    ParsedFunc<cplx>* func;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& token;
        ar& newVal;
        ar& oldVal;
        ar& func;
    }
};

class CommandAddAnim : public Command
{
    friend class boost::serialization::access;

public:
    CommandAddAnim() = default;
    CommandAddAnim(std::shared_ptr<Animation> s, InputPlane* in);

    void exec();
    void undo();

private:
    std::shared_ptr<Animation> subject;
    InputPlane* parent;
    size_t index;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& subject;
        ar& parent;
        ar& index;
    }
};

class CommandRemoveAnim : public Command
{
    friend class boost::serialization::access;

public:
    CommandRemoveAnim() = default;
    CommandRemoveAnim(int i, InputPlane* in);

    void exec();
    void undo();

private:
    std::shared_ptr<Animation> subject;
    InputPlane* parent;
    size_t index;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& subject;
        ar& parent;
        ar& index;
    }
};

class CommandEditAnim : public Command
{
    friend class boost::serialization::access;

public:
    CommandEditAnim() = default;
    CommandEditAnim(std::shared_ptr<Animation> A, int dur_ms, int reverse,
                    double offset, bool bounce, int sel1, int sel2, int sel3,
                    int handle, std::shared_ptr<Contour> C);

    void exec();
    void undo();

private:
    std::shared_ptr<Animation> subject;
    std::shared_ptr<Contour> newPath;
    std::shared_ptr<Contour> oldPath;
    int newDur_ms;
    int newReverse;
    double newOffset;
    bool newBounce;
    int newsel1;
    int newsel2;
    int newsel3;
    int newhandle;

    int oldDur_ms;
    int oldReverse;
    double oldOffset;
    bool oldBounce;
    int oldsel1;
    int oldsel2;
    int oldsel3;
    int oldhandle;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& newDur_ms;
        ar& newReverse;
        ar& newOffset;
        ar& newBounce;
        ar& newsel1;
        ar& newsel2;
        ar& newsel3;
        ar& newhandle;
        ar& newPath;
        ar& oldPath;
    }
};

class CommandSetFlag : public Command
{
    friend class boost::serialization::access;

public:
    CommandSetFlag() = default;
    CommandSetFlag(bool* s, bool val) : subject(s), value(val), old(*s) {}

    void exec() { *subject = value; }
    void undo() { *subject = old; }

private:
    bool* subject;
    bool old;
    bool value;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& boost::serialization::base_object<Command>(*this);
        ar& subject;
        ar& value;
        ar& old;
    }
};

// BOOST_CLASS_EXPORT_KEY(CommandSetFlag)
BOOST_CLASS_EXPORT_KEY(CommandEditAnim)
BOOST_CLASS_EXPORT_KEY(CommandEditVar)
BOOST_CLASS_EXPORT_KEY(CommandParametricFuncEntry)
BOOST_CLASS_EXPORT_KEY(CommandAxesReset)
BOOST_CLASS_EXPORT_KEY(CommandAxesSet)
BOOST_CLASS_EXPORT_KEY(CommandAddContour)
//BOOST_CLASS_EXPORT_KEY(CommandContourSubdivide)
BOOST_CLASS_EXPORT_KEY(CommandContourEditRadius)
BOOST_CLASS_EXPORT_KEY(CommandContourRemovePoint)
BOOST_CLASS_EXPORT_KEY(CommandContourScale)
BOOST_CLASS_EXPORT_KEY(CommandContourRotate)
BOOST_CLASS_EXPORT_KEY(CommandContourRotateAndScale)
BOOST_CLASS_EXPORT_KEY(CommandContourAddPoint)
BOOST_CLASS_EXPORT_KEY(CommandContourSetPoint)
BOOST_CLASS_EXPORT_KEY(CommandContourPlaceAt)
BOOST_CLASS_EXPORT_KEY(CommandContourTranslate)