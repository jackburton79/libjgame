#include "Action.h"
#include "Actor.h"
#include "Animation.h"
#include "Core.h"
#include "Door.h"
#include "Game.h"
#include "GraphicsEngine.h"
#include "GUI.h"
// TODO: Temporary for debug. Remove
#include "IDSResource.h"
#include "Parsing.h"
#include "RectUtils.h"
#include "Region.h"
#include "ResManager.h"
#include "RoomBase.h"
#include "Script.h"
#include "SPLResource.h"
#include "TextSupport.h"
#include "Timer.h"
// TODO: Remove this dependency
#include "TLKResource.h"

#include <algorithm>
#include <cxxabi.h>
#include <typeinfo>

static bool
PointSufficientlyClose(const IE::point& pointA, const IE::point& pointB)
{
	return (std::abs(pointA.x - pointB.x) <= 5 * 2)
		&& (std::abs(pointA.y - pointB.y) <= 5 * 2);
}


static void
VariableGetScopeName(const char* variable, std::string& varScope, std::string& varName)
{
	std::string variableScope;
	varScope.append(variable, 6);
	std::string variableName;
	varName.append(&variable[6]);
}


// TODO: we should not pass Object pointers,
// but pass action parameters instead, which should be evaluated
// when the action is being executed
Action::Action(Object* object, action_node* node)
    :
	fObject(object),
	fActionParams(node),
	fInitiated(false),
	fCompleted(false)
{
	fObject->Acquire();
}


Action::~Action()
{
	fObject->Release();
}


bool
Action::Initiated() const
{
	return fInitiated;
}


void
Action::SetInitiated()
{
	fInitiated = true;
}


bool
Action::Completed() const
{
    return fCompleted;
}


void
Action::SetCompleted()
{
	std::cout << Name() << ":SetCompleted()!" << std::endl;
	fCompleted = true;
}


std::string
Action::Name() const
{
	int status;
	char* demangled = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
	std::string name = demangled;
	free(demangled);
	return name;
}


// SetGlobalAction
SetGlobalAction::SetGlobalAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
SetGlobalAction::operator()()
{
	std::string variableScope;
	std::string variableName;
	VariableGetScopeName(fActionParams->string1, variableScope, variableName);
	if (variableScope.compare("LOCALS") == 0) {
		if (fObject != NULL)
			fObject->Vars().Set(variableName.c_str(),
					fActionParams->integer1);
	} else {
		// TODO: Check for AREA variables
		Core::Get()->Vars().Set(fActionParams->string1, fActionParams->integer1);
	}
	SetCompleted();
}


// CreateCreatureAction
CreateCreatureAction::CreateCreatureAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
CreateCreatureAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	IE::point point = fActionParams->where;
	if (point.x == -1 && point.y == -1) {
		Actor* thisActor = dynamic_cast<Actor*>(fObject);
		if (thisActor != NULL) {
			std::cout << "active creature: " << fObject->Name() << std::endl;
			point = thisActor->Position();
			point.x += Core::RandomNumber(-20, 20);
			point.y += Core::RandomNumber(-20, 20);
		}
	} else {
		std::cout << "create actor at " << point.x << ", " << point.y << std::endl;
	}
	Actor* actor = new Actor(fActionParams->string1, point, fActionParams->integer1);
	Core::Get()->AddActorToCurrentArea(actor);
	//core->SetActiveActor(actor);
	SetCompleted();
}


// CreateCreatureImpassableAction
CreateCreatureImpassableAction::CreateCreatureImpassableAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
CreateCreatureImpassableAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	Actor* actor = new Actor(fActionParams->string1,
						fActionParams->where, fActionParams->integer1);
	std::cout << "Created actor (IMPASSABLE) " << fActionParams->string1 << " on ";
	std::cout << fActionParams->where.x << ", " << fActionParams->where.y << std::endl;
	//actor->SetDestination(fActionParams->where);
	Core::Get()->AddActorToCurrentArea(actor);
	SetCompleted();
	//core->SetActiveActor(actor);
}


// TriggerActivationAction
TriggerActivationAction::TriggerActivationAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
TriggerActivationAction::operator()()
{
	Region* region = dynamic_cast<Region*>(Script::FindTargetObject(fObject, fActionParams));
	if (region != NULL)
		region->ActivateTrigger();
	SetCompleted();
}


// DestroySelfAction
DestroySelfAction::DestroySelfAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
DestroySelfAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	Object* object = Script::FindSenderObject(fObject, fActionParams);
	object->DestroySelf();
	SetCompleted();
}


// ForceSpell
ForceSpell::ForceSpell(Object* object, action_node* node)
	:
	Action(object, node),
	fDuration(50)
{
}


/* virtual */
void
ForceSpell::operator()()
{
	Actor* sender = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	sender->Print();
	Object* target = Script::FindTargetObject(sender, fActionParams);
	target->Print();

	IDSResource* spellIDS = gResManager->GetIDS("SPELL");
	std::string spellName = spellIDS->StringForID(fActionParams->integer1).c_str();
	gResManager->ReleaseResource(spellIDS);

	std::cout << "spell: " << spellName << std::endl;
	/*SPLResource* spellResource = gResManager->GetSPL(spellName);
	std::cout << spellResource->SpellDescriptionIdentified() << std::endl;
	std::cout << spellResource->SpellDescriptionUnidentified() << std::endl;
	gResManager->ReleaseResource(spellResource);*/
	sender->SetAnimationAction(ACT_CAST_SPELL);
	// TODO: only for testing
	if (fDuration-- == 0) {
		// TODO: There should be a way to set the previous animation action,
		// because we don't know here if ACT_STANDING is the correct one
		sender->SetAnimationAction(ACT_STANDING);
		SetCompleted();
	}
}


// MoveBetweenAreasEffect
MoveBetweenAreasEffect::MoveBetweenAreasEffect(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
MoveBetweenAreasEffect::operator()()
{
	if (!Initiated()) {
		SetInitiated();
		Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
		if (actor != NULL) {
			std::cout << "area:" << fActionParams->string1 << std::endl;
			actor->SetPosition(fActionParams->where);
			actor->SetOrientation(fActionParams->integer1);
		}
		SetCompleted();
	}
}


// PlayDeadAction
PlayDeadAction::PlayDeadAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
PlayDeadAction::operator()()
{
	if (!Initiated()) {
		SetInitiated();
		Actor* actor = dynamic_cast<Actor*>(fObject);
		if (actor == NULL)
			SetCompleted();
		actor->SetAnimationAction(ACT_DEAD);
		fDuration = fActionParams->integer1 * AI_UPDATE_FREQ;
	}
	
	if (fDuration-- <= 0)
		SetCompleted();
}


// SetInterruptableAction
SetInterruptableAction::SetInterruptableAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
SetInterruptableAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	fObject->SetInterruptable(fActionParams->integer1 == 1);
	SetCompleted();
}


// WalkTo
WalkTo::WalkTo(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
WalkTo::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (!Initiated()) {	
		actor->SetDestination(fActionParams->where);
		SetInitiated();
	}

	if (actor->Position() == actor->Destination()) {
		SetCompleted();
		return;
	}
}


// WalkToObject
WalkToObject::WalkToObject(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
WalkToObject::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor == NULL)
		return;

	Object* target = Script::FindTargetObject(actor, fActionParams);
	if (target == NULL) {
		SetCompleted();
		return;
	}
	
	//target->Acquire();
	
	IE::point destination = target->NearestPoint(actor->Position());
	if (!PointSufficientlyClose(actor->Position(), destination))
		actor->SetDestination(destination);

	if (actor->Position() == actor->Destination()) {
		SetCompleted();
		//target->Release();
		return;
	}
}

// RandomFly
RandomFly::RandomFly(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
RandomFly::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor == NULL)
		return;

	// TODO: Fly
	int16 randomX = Core::RandomNumber(-50, 50);
	int16 randomY = Core::RandomNumber(-50, 50);

	IE::point destination = offset_point(actor->Position(), randomX, randomY);
	if (!PointSufficientlyClose(actor->Position(), destination))
		actor->SetDestination(destination, true);

	if (actor->Position() == actor->Destination())
		SetCompleted();
}


// FlyTo
FlyTo::FlyTo(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
FlyTo::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor == NULL)
		return;

	if (!Initiated()) {
		actor->SetDestination(fActionParams->where, true);
		SetInitiated();
	}

	if (actor->Position() == actor->Destination()) {
		SetCompleted();
		actor->SetAnimationAction(ACT_STANDING);
		return;
	}

	actor->SetAnimationAction(ACT_WALKING);
	actor->MoveToNextPointInPath(true);
}


// RandomWalk
RandomWalk::RandomWalk(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
RandomWalk::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor == NULL)
		return;

	int16 randomX = Core::RandomNumber(-50, 50);
	int16 randomY = Core::RandomNumber(-50, 50);

	IE::point destination = offset_point(actor->Position(), randomX, randomY);
	if (!PointSufficientlyClose(actor->Position(), destination))
		actor->SetDestination(destination);

	if (actor->Position() == actor->Destination())
		SetCompleted();
}


// Wait
Wait::Wait(Object* object, action_node* node)
	:
	Action(object, node),
	fWaitTime(0)
{
}


/* virtual */
void
Wait::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	if (!Initiated()) {
		SetInitiated();
		fWaitTime = fActionParams->integer1 * AI_UPDATE_FREQ; // TODO use a constant
		return;
	}
	
	if (--fWaitTime <= 0)
		SetCompleted();
}


// SmallWait
SmallWait::SmallWait(Object* object, action_node* node)
	:
	Action(object, node),
	fWaitTime(0)
{
}


/* virtual */
void
SmallWait::operator()()
{
	// TODO: Sometimes there is a different sender object.
	// Find a way to execute this action on the correct sender
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	//Object* object = Script::FindObject(fObject, fActionParams);
	//if (object != NULL)
	//	object->SetWaitTime(fActionParams->integer1);
	if (!Initiated()) {
		SetInitiated();
		fWaitTime = fActionParams->integer1;
		return;
	}
	
	if (--fWaitTime <= 0)
		SetCompleted();
}


// OpenDoor
OpenDoor::OpenDoor(Object* sender, action_node* node)
	:
	Action(sender, node)
{
}


/* virtual */
void
OpenDoor::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor == NULL) {
		std::cerr << "NULL ACTOR!!!" << std::endl;
		return;
	}

	Object* target = Script::FindTargetObject(actor, fActionParams);
	Door* door = dynamic_cast<Door*>(target);
	if (door == NULL) {
		std::cerr << "NULL DOOR!!! MEANS THE OBJECT IS NOT A DOOR" << std::endl;
		SetCompleted();
		return;
	}

	if (!door->Opened()) {
		door->Toggle();
		SetCompleted();
	}
}


// DisplayMessage
DisplayMessage::DisplayMessage(Object* sender, action_node* node)
	:
	Action(sender, node)
{
}


/* virtual */
void
DisplayMessage::operator()()
{
	std::cout << "DisplayMessage:: ";
	std::cout << IDTable::GetDialog(fActionParams->integer1) << std::endl;
	SetCompleted();
}


// Attack
Attack::Attack(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
Attack::operator()()
{
	Actor* sender = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (sender == NULL)
		return;
	
	Actor* target = dynamic_cast<Actor*>(Script::FindTargetObject(sender, fActionParams));
	if (target == NULL){
		SetCompleted();
		return;
	}

	IE::point point = target->NearestPoint(sender->Position());
	if (!PointSufficientlyClose(sender->Position(), point))
		sender->SetDestination(point);

	if (sender->Position() != sender->Destination()) {
		sender->SetAnimationAction(ACT_WALKING);
		sender->MoveToNextPointInPath(sender->IsFlying());
	} else {
		sender->SetAnimationAction(ACT_ATTACKING);
		sender->AttackTarget(target);
		SetCompleted();
	}
}


// RunAwayFrom
RunAwayFrom::RunAwayFrom(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
RunAwayFrom::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor == NULL)
		return;

	// TODO: Improve.
	// TODO: We are recalculating this every time. Is it correct ?
	Actor* target = dynamic_cast<Actor*>(Script::FindTargetObject(actor, fActionParams));
	if (target == NULL){
		SetCompleted();
		return;
	}
	
	// TODO: Improve implementation
	if (Core::Get()->Distance(actor, target) < 200) {
		IE::point point = PointAway(actor, target);
		if (actor->Destination() != point) {
			actor->SetDestination(point);
		}
	}

	if (actor->Position() == actor->Destination()) {
		SetCompleted();
		actor->SetAnimationAction(ACT_STANDING);
	} else {
		actor->SetAnimationAction(ACT_WALKING);
		actor->MoveToNextPointInPath(actor->IsFlying());
	}
}


IE::point
RunAwayFrom::PointAway(Actor* actor, Actor* target)
{
	IE::point targetPos = target->NearestPoint(actor->Position());
	IE::point actorPos = actor->Position();
	if (targetPos.x > actorPos.x)
		actorPos.x -= 150;
	else if (targetPos.x < actorPos.x)
		actorPos.x += 150;

	if (targetPos.y > actorPos.y)
		actorPos.y -= 150;
	else if (targetPos.y < actorPos.y)
		actorPos.y += 150;

	return actorPos;
}


// DialogAction
DialogAction::DialogAction(Object* source, action_node* node)
	:
	Action(source, node)
{
	std::cout << "Dialogue::Dialogue()" << std::endl;
}


/* virtual */
void
DialogAction::operator()()
{
	Object* object = Script::FindSenderObject(fObject, fActionParams);
	if (object == NULL)
		return;
	
	Actor* target = dynamic_cast<Actor*>(Script::FindTargetObject(object, fActionParams));
	if (target == NULL) {
		SetCompleted();
		return;
	}

	std::cout << "object: " << object->Name();
	std::cout << "target: " << target->Name();
	std::cout << std::endl;
	// TODO: Some dialogue action require the actor to be near the target,
	// others do not. Must be able to differentiate
/*
	const IE::point point = fTarget->Target()->NearestPoint(fActor.Target()->Position());
	if (!PointSufficientlyClose(fActor.Target()->Destination(), point))
		fActor.Target()->SetDestination(point);
*/
	SetInitiated();
/*
	if (!PointSufficientlyClose(fActor.Target()->Position(), fTarget->Target()->Position())) {
		fActor.Target()->SetAnimationAction(ACT_WALKING);
		fActor.Target()->MoveToNextPointInPath(fActor.Target()->IsFlying());
	} else {
*/
		//SetAnimationAction(ACT_STANDING);
		target->InitiateDialogWith(NULL);
		SetCompleted();
	//}
}


// FadeToColorAction
FadeToColorAction::FadeToColorAction(Object* object, action_node* node)
	:
	Action(object, node),
	fCurrentValue(0),
	fTargetValue(0),
	fStepValue(1)
{
}


/* virtual */
void
FadeToColorAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	if (!Initiated()) {
		SetInitiated();
		fCurrentValue = 255;
		fTargetValue = 0;
		fStepValue = (fCurrentValue - fTargetValue) / fActionParams->where.x;
	}
	
	GraphicsEngine::Get()->SetFade(fCurrentValue);
	if (fCurrentValue > fTargetValue)
		fCurrentValue -= fStepValue;
	else
		SetCompleted();
}

// FadeFromColorAction
FadeFromColorAction::FadeFromColorAction(Object* object, action_node* node)
	:
	Action(object, node),
	fCurrentValue(0),
	fTargetValue(0),
	fStepValue(1)
{
}


/* virtual */
void
FadeFromColorAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	if (!Initiated()) {
		SetInitiated();
		fCurrentValue = 0;
		fTargetValue = 255;
		fStepValue = fTargetValue / fActionParams->where.x;
	}
	
	GraphicsEngine::Get()->SetFade(fCurrentValue);
	if (fCurrentValue < fTargetValue)
		fCurrentValue += fStepValue;
	else
		SetCompleted();
}


// MoveViewPoint
MoveViewPoint::MoveViewPoint(Object* object, action_node* node)
	:
	Action(object, node)
{	
}


/* virtual */
void
MoveViewPoint::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	//SetCompleted();
	//return;
	if (!Initiated()) {
		SetInitiated();
		fDestination = fActionParams->where;
		Core::Get()->CurrentRoom()->SanitizeOffsetCenter(fDestination);
		switch (fActionParams->integer1) {
			case 1:
				fScrollSpeed = 10;
				break;
			case 2:
				fScrollSpeed = 20;
				break;
			case 3:
				fScrollSpeed = 40;
				break;
			case 4:
				fScrollSpeed = 80;
				break;
			case 0:		
			default:		
				fScrollSpeed = 10000;
				break;
		}
	}
	
	RoomBase* room = Core::Get()->CurrentRoom();
	IE::point offset = room->AreaCenterPoint();
	const int16 step = fScrollSpeed;
	if (offset != fDestination) {
		if (offset.x > fDestination.x)
			offset.x = std::max((int16)(offset.x - step), fDestination.x);
		else if (offset.x < fDestination.x)
			offset.x = std::min((int16)(offset.x + step), fDestination.x);
		
		if (offset.y > fDestination.y)
			offset.y = std::max((int16)(offset.y - step), fDestination.y);
		else if (offset.y < fDestination.y)
			offset.y = std::min((int16)(offset.y + step), fDestination.y);
		room->SetAreaOffsetCenter(offset);
		std::cout << std::dec;
		std::cout << "offset: " << offset.x << ", " << offset.y << std::endl;
		std::cout << "fDestination: " << fDestination.x << ", " << fDestination.y << std::endl;
	} else
		SetCompleted();
}


ScreenShake::ScreenShake(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
ScreenShake::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	if (!Initiated()) {
		SetInitiated();
		if (fObject != NULL)
			fObject->SetWaitTime(fDuration);
		fOffset = fActionParams->where;
		fDuration = fActionParams->integer1;
	}
	
	GFX::point point = { 0, 0 };
	if (fDuration-- == 0) {	
		GraphicsEngine::Get()->SetRenderingOffset(point);	
		SetCompleted();
		return;
	}
	
	point.x = fOffset.x;	
	point.y = fOffset.y;
	
	GraphicsEngine::Get()->SetRenderingOffset(point);
	fOffset.x = -fOffset.x;
	fOffset.y = -fOffset.y;
}


// StartCutsceneModeAction
StartCutsceneModeAction::StartCutsceneModeAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
StartCutsceneModeAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	Core::Get()->StartCutsceneMode();
	SetCompleted();
}


// StartCutsceneAction
StartCutsceneAction::StartCutsceneAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
StartCutsceneAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	Core::Get()->StartCutscene(fActionParams->string1);
	SetCompleted();
}


// HideGUIAction
HideGUIAction::HideGUIAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
HideGUIAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	GUI::Get()->Hide();
	SetCompleted();
}


// UnhideGUIAction
UnhideGUIAction::UnhideGUIAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
UnhideGUIAction::operator()()
{
	if (fObject == NULL)
		std::cerr << "NULL OBJECT" << std::endl;
	GUI::Get()->Show();
	SetCompleted();
}


DisplayString::DisplayString(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
DisplayString::operator()()
{
	if (!Initiated()) {
		SetInitiated();
		fDuration = fActionParams->integer1;
		std::string string = IDTable::GetDialog(fActionParams->integer1); 
		GUI::Get()->DisplayString(string, fActionParams->where.x, fActionParams->where.y, fDuration * AI_UPDATE_FREQ);
	}
	
	if (fDuration-- <= 0) {	
		SetCompleted();
	}
}


DisplayStringHead::DisplayStringHead(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
DisplayStringHead::operator()()
{
	if (!Initiated()) {
		SetInitiated();
		fDuration = 100; //??
		Actor* sender = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
		Actor* actor = dynamic_cast<Actor*>(Script::FindTargetObject(sender, fActionParams));
		if (actor == NULL)
			SetCompleted();
		TLKEntry* tlkEntry = IDTable::GetTLK(fActionParams->integer1);
		actor->SetText(tlkEntry->text);
		delete tlkEntry;
	}
	if (fDuration-- <= 0) {
		Actor* sender = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
		Actor* actor = dynamic_cast<Actor*>(Script::FindTargetObject(sender, fActionParams));
		if (actor != NULL)
			actor->SetText("");
		SetCompleted();
	}
}


// ChangeOrientationExtAction
ChangeOrientationExtAction::ChangeOrientationExtAction(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
ChangeOrientationExtAction::operator()()
{
	Actor* actor = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	if (actor != NULL)
		actor->SetOrientation(fActionParams->integer1);
	SetCompleted();
}


// FaceObject
FaceObject::FaceObject(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
FaceObject::operator()()
{
	Actor* sender = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	Object* target = Script::FindTargetObject(sender, fActionParams);
	if (sender == NULL || target == NULL) {
		std::cerr << "FaceObject(): NULL object" << std::endl;
		SetCompleted();
		return;
	}

	const IE::rect objectFrame = target->Frame();
	IE::point point;
	point.x = (objectFrame.x_max - objectFrame.x_min) / 2;
	point.y = (objectFrame.y_max - objectFrame.y_min) / 2;
	sender->SetOrientation(point);
	SetCompleted();
}


// CreateVisualEffect
CreateVisualEffect::CreateVisualEffect(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
CreateVisualEffect::operator()()
{
	Core::Get()->PlayEffect(fActionParams->string1, fActionParams->where);
	SetCompleted();
}


// CreateVisualEffectObject
CreateVisualEffectObject::CreateVisualEffectObject(Object* object, action_node* node)
	:
	Action(object, node)
{
}


/* virtual */
void
CreateVisualEffectObject::operator()()
{
	Actor* sender = dynamic_cast<Actor*>(Script::FindSenderObject(fObject, fActionParams));
	Object* target = Script::FindTargetObject(sender, fActionParams);
	IE::point point;
	point.x = target->Frame().x_min;
	point.y = target->Frame().y_min;
	Core::Get()->PlayEffect(fActionParams->string1, point);
	SetCompleted();
}


