#include "Commands.h"
#include "Contour.h"
#include "ContourCircle.h"
#include "ContourParametric.h"
#include "InputPlane.h"
#include "OutputPlane.h"

BOOST_CLASS_EXPORT_IMPLEMENT(CommandParametricFuncEntry)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAxesReset)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAxesSet)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAddContour)
// BOOST_CLASS_EXPORT_IMPLEMENT(CommandInputPlaneCreateContour)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourSubdivide)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourEditRadius)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourRemovePoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourAddPoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourMovePoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourPlaceAt)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourTranslate)

void CommandContourTranslate::exec() { subject->Translate(newPos, oldPos); }
void CommandContourTranslate::undo() { subject->Translate(oldPos, newPos); }

CommandContourPlaceAt::CommandContourPlaceAt(Contour* s, cplx n, int pt)
    : newPos(n), subject(s), point(pt)
{
    oldPos = s->GetCtrlPoint(pt);
}

void CommandContourPlaceAt::exec()
{
    subject->Translate(newPos, subject->GetCtrlPoint(point));
}

void CommandContourPlaceAt::undo() { subject->Translate(oldPos, newPos); }

CommandContourMovePoint::CommandContourMovePoint(Contour* s, cplx n, int i)
    : newPos(n), index(i), subject(s)
{
    oldPos = s->GetCtrlPoint(i);
}

CommandContourMovePoint::CommandContourMovePoint(Contour* s, cplx n, int i,
                                                 cplx old)
    : newPos(n), index(i), subject(s)
{
    oldPos = old;
}

void CommandContourMovePoint::exec() { subject->SetCtrlPoint(index, newPos); }

void CommandContourMovePoint::undo() { subject->SetCtrlPoint(index, oldPos); }

void CommandContourAddPoint::exec()
{
    index = subject->GetPointCount();
    subject->AddPoint(mPos);
}

void CommandContourAddPoint::undo() { subject->RemovePoint(index); }

CommandContourRemovePoint::CommandContourRemovePoint(Contour* s, int i)
    : index(i), subject(s)
{
    oldPos = s->GetCtrlPoint(i);
}

void CommandContourRemovePoint::exec() { subject->RemovePoint(index); }

void CommandContourRemovePoint::undo() { subject->AddPoint(oldPos); }

void CommandContourSubdivide::exec() { subject->Subdivide(res); }

// CommandInputPlaneCreateContour::CommandInputPlaneCreateContour(InputPlane* s,
// wxPoint p)
//    : mPos(p), subject(s)
//{
//    index = s->GetContourCount();
//}
//
// void CommandInputPlaneCreateContour::exec()
//{
//    subject->AddContour(std::move(subject->CreateContour(mPos)));
//}
//
// void CommandInputPlaneCreateContour::undo()
//{
//    subject->RemoveContour(index);
//}

CommandAxesSet::CommandAxesSet(ComplexPlane* par, double rMin, double rMax,
                               double iMin, double iMax)
    : subject(&par->axes), parent(par)
{
    for (int i = 0; i < 4; i++)
        oldBounds[i] = subject->c[i];
    newBounds[0] = rMin;
    newBounds[1] = rMax;
    newBounds[2] = iMin;
    newBounds[3] = iMax;
}

CommandAxesSet::CommandAxesSet(ComplexPlane* par)
    : subject(&parent->axes), parent(par)
{
    for (int i = 0; i < 4; i++)
    {
        oldBounds[i] = subject->c[i];
        newBounds[i] = subject->c[i];
    }
}

void CommandAxesSet::exec()
{
    subject->realMin = newBounds[0];
    subject->realMax = newBounds[1];
    subject->imagMin = newBounds[2];
    subject->imagMax = newBounds[3];
    parent->UpdateGrid();
}

void CommandAxesSet::undo()
{
    subject->realMin = oldBounds[0];
    subject->realMax = oldBounds[1];
    subject->imagMin = oldBounds[2];
    subject->imagMax = oldBounds[3];
    parent->UpdateGrid();
}

void CommandAxesSet::SetPositionParam(cplx c)
{
    for (int i = 0; i < 4; i++)
        newBounds[i] = subject->c[i];
}

CommandAxesReset::CommandAxesReset(Axes* a) : subject(a)
{
    for (int i = 0; i < 4; i++)
        oldBounds[i] = a->c[i];
}

void CommandAxesReset::exec()
{
    subject->realMax = 10;
    subject->realMin = -10;
    subject->imagMax = 10;
    subject->imagMin = -10;
}

void CommandAxesReset::undo()
{
    subject->realMin = oldBounds[0];
    subject->realMax = oldBounds[1];
    subject->imagMin = oldBounds[2];
    subject->imagMax = oldBounds[3];
}

CommandParametricFuncEntry::CommandParametricFuncEntry(ContourParametric* C,
                                                       ParsedFunc<cplx> g,
                                                       InputPlane* par)
    : subject(C), parent(par)
{
    oldfunc = *C->GetFunctionPtr();
    newfunc = g;
}

void CommandParametricFuncEntry::exec()
{
    *subject->GetFunctionPtr() = newfunc;
    subject->Subdivide(parent->GetRes());
}

void CommandParametricFuncEntry::undo()
{
    *subject->GetFunctionPtr() = oldfunc;
    subject->Subdivide(parent->GetRes());
}

void CommandHistory::undo()
{
    history[--index]->undo();
    if (menu)
    {
        if (!index) menu->FindItem(wxID_UNDO)->Enable(false);
        menu->FindItem(wxID_REDO)->Enable(true);
    }
}

void CommandHistory::redo()
{
    history[index++]->exec();
    if (menu)
    {
        if (index == history.size()) menu->FindItem(wxID_REDO)->Enable(false);
        menu->FindItem(wxID_UNDO)->Enable(true);
    }
}

void CommandHistory::RecordCommand(std::unique_ptr<Command> C)
{
    // Wipe out stored commands past the current one
    history.resize(index++);
    history.push_back(std::move(C));
    if (menu)
    {
        menu->FindItem(wxID_UNDO)->Enable(true);
        menu->FindItem(wxID_REDO)->Enable(false);
    }
}

void CommandHistory::UpdateLastCommand(cplx c)
{
    history.back()->SetPositionParam(c);
}

void CommandHistory::PopCommand()
{
    history.pop_back();
    index--;
    if (menu)
    {
        if (!index) menu->FindItem(wxID_UNDO)->Enable(false);
        menu->FindItem(wxID_REDO)->Enable(true);
    }
}

void CommandAddContour::exec()
{
    subject->AddContour(C);
    C->markedForRedraw = true;
}

void CommandAddContour::undo()
{
    int i = subject->GetContourCount() - 1;
    subject->RemoveContour(i);
}

CommandContourEditRadius::CommandContourEditRadius(ContourCircle* s, double rad)
    : subject(s), radius(rad)
{
    oldRad = s->GetRadius();
}

void CommandContourEditRadius::exec() { subject->SetRadius(radius); }

void CommandContourEditRadius::undo() { subject->SetRadius(oldRad); }

void CommandContourEditRadius::SetPositionParam(cplx c)
{
    radius = abs(c - subject->GetCenter());
}

CommandRemoveContour::CommandRemoveContour(InputPlane* s, size_t i)
    : subject(s), index(i), C(s->GetContour(i))
{
}

void CommandRemoveContour::exec()
{
    subject->RemoveContour(index);
    subject->ShowAxisControls();
}

void CommandRemoveContour::undo()
{
    subject->InsertContour(C, index);
    C->markedForRedraw = true;
}

CommandContourColorSet::CommandContourColorSet(Contour* s, wxColor col)
    : subject(s), color(col)
{
    oldColor = s->color;
}

void CommandContourColorSet::exec()
{
    subject->color           = color;
    subject->markedForRedraw = true;
}

void CommandContourColorSet::undo()
{
    subject->color           = oldColor;
    subject->markedForRedraw = true;
}

CommandOutputFuncEntry::CommandOutputFuncEntry(ParsedFunc<cplx> g, OutputPlane* par) : newfunc(g), parent(par)
{
    oldfunc = parent->GetFunc();
}

void CommandOutputFuncEntry::exec()
{
    parent->CopyFunction(newfunc);
    parent->GetFuncInput()->SetValue(newfunc.GetInputText());
}

void CommandOutputFuncEntry::undo()
{
    parent->CopyFunction(oldfunc);
    parent->GetFuncInput()->SetValue(oldfunc.GetInputText());
}
