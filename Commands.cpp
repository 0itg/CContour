#include "Commands.h"
#include "Contour.h"
#include "InputPlane.h"
#include "OutputPlane.h"

BOOST_CLASS_EXPORT_IMPLEMENT(CommandOutputPlaneFuncEntry)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAxesReset)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandAxesSet)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandInputPlaneCreateContour)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourSubdivide)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourRemovePoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourAddPoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourMovePoint)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourPlaceAt)
BOOST_CLASS_EXPORT_IMPLEMENT(CommandContourTranslate)


void CommandContourTranslate::exec() { subject->Translate(newPos, oldPos); }

void CommandContourPlaceAt::exec()
{
    if (point > -1)
        subject->Translate(newPos, subject->GetCtrlPoint(point));
    else
        subject->Translate(newPos, subject->GetCenter());
}

void CommandContourMovePoint::exec() { subject->SetCtrlPoint(index, newPos); }

void CommandContourAddPoint::exec() { subject->AddPoint(mPos); }

void CommandContourRemovePoint::exec() { subject->RemovePoint(index); }

void CommandContourSubdivide::exec() { subject->Subdivide(res); }

void CommandInputPlaneCreateContour::exec()
{
    subject->AddContour(std::move(subject->CreateContour(mPos)));
}

void CommandAxesSet::exec()
{
    subject->realMax = realMax;
    subject->realMin = realMin;
    subject->imagMax = imagMax;
    subject->imagMin = imagMin;
}

void CommandAxesReset::exec()
{
    subject->realMax = 10;
    subject->realMin = -10;
    subject->imagMax = 10;
    subject->imagMin = -10;
}

void CommandOutputPlaneFuncEntry::exec() { subject->EnterFunction(func); }
