#include "Commands.h"
#include "ContourCircle.h"
#include "ContourParametric.h"
#include "InputPlane.h"
#include "OutputPlane.h"

// BOOST_CLASS_EXPORT_IMPLEMENT(CommandSetFlag)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandEditAnim)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandEditVar)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandParametricFuncEntry)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAxesReset)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAxesSet)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAddContour)
//BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourSubdivide)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourEditRadius)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourRemovePoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourScale)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourRotate)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourRotateAndScale)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourAddPoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourSetPoint)
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
    subject->markedForRedraw = true;
}

void CommandContourPlaceAt::undo() { subject->Translate(oldPos, newPos); }

CommandContourSetPoint::CommandContourSetPoint(Contour* s, cplx n, int i)
    : newPos(n), index(i), subject(s)
{
    oldPos = s->GetCtrlPoint(i);
}

CommandContourSetPoint::CommandContourSetPoint(Contour* s, cplx n, int i,
                                               cplx old)
    : newPos(n), index(i), subject(s)
{
    oldPos = old;
}

void CommandContourSetPoint::exec()
{
    subject->SetCtrlPoint(index, newPos);
    subject->markedForRedraw = true;
}

void CommandContourSetPoint::undo() { subject->SetCtrlPoint(index, oldPos); }

void CommandContourAddPoint::exec()
{
    index = subject->GetPointCount();
    subject->AddPoint(mPos);
    subject->markedForRedraw = true;
}

void CommandContourAddPoint::undo() { subject->RemovePoint(index); }

CommandContourRemovePoint::CommandContourRemovePoint(Contour* s, int i)
    : index(i), subject(s)
{
    oldPos = s->GetCtrlPoint(i);
}

void CommandContourRemovePoint::exec()
{
    subject->RemovePoint(index);
    subject->markedForRedraw = true;
}

void CommandContourRemovePoint::undo() { subject->AddPoint(oldPos); }

//void CommandContourSubdivide::exec() { subject->Subdivide(res); }

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

CommandAxesSet::CommandAxesSet(ComplexPlane* par, Grid* g)
    : subject(&parent->axes), parent(par), grid(g)
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
    subject->RecalcSteps();
    if (grid)
    {
        grid->hStep = subject->reStep;
        grid->vStep = subject->imStep;
    }
}

void CommandAxesSet::undo()
{
    subject->realMin = oldBounds[0];
    subject->realMax = oldBounds[1];
    subject->imagMin = oldBounds[2];
    subject->imagMax = oldBounds[3];
    subject->RecalcSteps();
    if (grid)
    {
        grid->hStep = subject->reStep;
        grid->vStep = subject->imStep;
    }
}

void CommandAxesSet::SetPositionParam(cplx c)
{
    for (int i = 0; i < 4; i++)
        newBounds[i] = subject->c[i];
}

CommandAxesReset::CommandAxesReset(ComplexPlane* par)
    : subject(&par->axes), parent(par), grid(par->GetGrid())
{
    for (int i = 0; i < 4; i++)
        oldBounds[i] = subject->c[i];
}

void CommandAxesReset::exec()
{
    subject->realMax = 10;
    subject->realMin = -10;
    subject->imagMax = 10;
    subject->imagMin = -10;
    subject->RecalcSteps();
    if (grid)
    {
        grid->hStep = subject->reStep;
        grid->vStep = subject->imStep;
    }
}

void CommandAxesReset::undo()
{
    subject->realMin = oldBounds[0];
    subject->realMax = oldBounds[1];
    subject->imagMin = oldBounds[2];
    subject->imagMax = oldBounds[3];
    subject->RecalcSteps();
    if (grid)
    {
        grid->hStep = subject->reStep;
        grid->vStep = subject->imStep;
    }
}

CommandParametricFuncEntry::CommandParametricFuncEntry(ContourParametric* C,
                                                       ParsedFunc<cplx> g)
    : subject(C)
{
    oldfunc = *C->GetFunctionPtr();
    newfunc = g;
}

void CommandParametricFuncEntry::exec()
{
    *subject->GetFunctionPtr() = newfunc;
    subject->markedForRedraw = true;
}

void CommandParametricFuncEntry::undo()
{
    *subject->GetFunctionPtr() = oldfunc;
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

void CommandHistory::Clear()
{
    history.clear();
    index = 0;
    menu->FindItem(wxID_REDO)->Enable(false);
    menu->FindItem(wxID_REDO)->Enable(false);
}

void CommandAddContour::exec()
{
    subject->AddContour(C);
    subject->GetToolPanel()->PopulateContourTextCtrls(C.get());
    C->markedForRedraw = true;
}

void CommandAddContour::undo()
{
    int i = subject->GetContourCount() - 1;
    subject->RemoveContour(i);
    subject->ShowAxisControls();
}

CommandContourEditRadius::CommandContourEditRadius(ContourCircle* s, double rad)
    : subject(s), radius(rad)
{
    oldRad = s->GetRadius();
}

void CommandContourEditRadius::exec()
{ 
    subject->SetRadius(radius);
    subject->markedForRedraw = true;
}

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
    subject->GetToolPanel()->PopulateContourTextCtrls(C.get());
}

CommandContourColorSet::CommandContourColorSet(Contour* s, wxColor col)
    : subject(s), color(col)
{
    oldColor = s->color;
}

void CommandContourColorSet::exec()
{
    subject->color = color;
    subject->markedForRedraw = true;
}

void CommandContourColorSet::undo() { subject->color = oldColor; }

CommandOutputFuncEntry::CommandOutputFuncEntry(ParsedFunc<cplx> g,
                                               OutputPlane* par)
    : newfunc(g), parent(par)
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

CommandEditVar::CommandEditVar(std::string tok, cplx c, ParsedFunc<cplx>* f)
    : token(tok), newVal(c), func(f)
{
    oldVal = f->GetVar(tok)->GetVal();
}

void CommandEditVar::exec() { func->SetVariable(token, newVal); }

void CommandEditVar::undo() { func->SetVariable(token, oldVal); }

CommandAddAnim::CommandAddAnim(std::shared_ptr<Animation> s, InputPlane* in)
    : subject(s), parent(in)
{
    index = parent->AnimCount() - 1;
}

void CommandAddAnim::exec() { parent->AddAnimation(subject); }

void CommandAddAnim::undo() { parent->RemoveAnimation(index); }

CommandRemoveAnim::CommandRemoveAnim(int i, InputPlane* in)
    : index(i), parent(in)
{
    subject = parent->GetAnimation(i);
}

void CommandRemoveAnim::exec() { parent->RemoveAnimation(index); }

void CommandRemoveAnim::undo() { parent->InsertAnimation(index, subject); }

CommandEditAnim::CommandEditAnim(std::shared_ptr<Animation> A, int dur_ms,
                                 int reverse, double offset, bool bounce,
                                 int sel1, int sel2, int sel3, int handle,
                                 std::shared_ptr<Contour> C)
    : newDur_ms(dur_ms), newReverse(reverse), newOffset(offset),
      newBounce(bounce), newsel1(sel1), newsel2(sel2), newsel3(sel3),
      newhandle(handle), newPath(C), subject(A)
{
    oldDur_ms  = subject->duration_ms;
    oldReverse = subject->reverse;
    oldOffset  = subject->offset;
    oldBounce  = subject->bounce;
    oldsel1    = subject->subjSel;
    oldsel2    = subject->comSel;
    oldsel3    = subject->pathSel;
    oldhandle  = subject->handle;
    oldPath    = subject->GetPath();
}

void CommandEditAnim::exec()
{
    subject->duration_ms = newDur_ms;
    subject->reverse     = newReverse;
    subject->offset      = newOffset;
    subject->bounce      = newBounce;
    subject->subjSel     = newsel1;
    subject->comSel      = newsel2;
    subject->pathSel     = newsel3;
    subject->handle      = newhandle;
    subject->SetPathContour(newPath);
}

void CommandEditAnim::undo()
{
    subject->duration_ms = oldDur_ms;
    subject->reverse     = oldReverse;
    subject->offset      = oldOffset;
    subject->bounce      = oldBounce;
    subject->subjSel     = oldsel1;
    subject->comSel      = oldsel2;
    subject->pathSel     = oldsel3;
    subject->handle      = oldhandle;
    subject->SetPathContour(oldPath);
}

CommandContourRotateAndScale::CommandContourRotateAndScale(Contour* s, cplx c,
                                                           cplx piv)
    : subject(s), V(c)
{
    if (pivot == cplx(INFINITY, INFINITY))
        pivot = s->GetCenter();
    else
        pivot = piv;
}

void CommandContourRotateAndScale::exec()
{
    subject->RotateAndScale(V / Vlast, pivot);
    subject->markedForRedraw = true;
}

void CommandContourRotateAndScale::undo()
{
    subject->RotateAndScale(Vlast / V, pivot);
}
