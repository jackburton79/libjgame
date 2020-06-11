#include "Action.h"
#include "Actor.h"
#include "AreaResource.h"
#include "Core.h"
#include "CreResource.h"
#include "Door.h"
#include "Game.h"
#include "GUI.h"
#include "IDSResource.h"
#include "Parsing.h"
#include "Party.h"
#include "Region.h"
#include "ResManager.h"
#include "RoomBase.h"
#include "Script.h"
#include "Timer.h"

#include <algorithm>
#include <assert.h>
#include <sstream>


bool Script::sDebug = false;

static int sIndent = 0;
static void IndentMore()
{
	sIndent++;
}


static void IndentLess()
{
	if (--sIndent < 0)
		sIndent = 0;
}


static void PrintIndentation()
{
	for (int i = 0; i < sIndent; i++)
		std::cout << " ";
}


static void
VariableGetScopeName(const char* variable, std::string& varScope, std::string& varName)
{
	std::string variableScope;
	varScope.append(variable, 6);
	std::string variableName;
	varName.append(&variable[6]);
}


Script::Script(node* rootNode)
	:
	fRootNode(rootNode),
	fProcessed(false),
	fOrTriggers(0),
	fSender(NULL),
	fLastTrigger(NULL),
	fCurrentNode(NULL)
{
	if (rootNode == NULL)
		throw "Script::Script(): root node is NULL";

	fRootNode->next = NULL;
}


Script::~Script()
{
	_DeleteNode(fRootNode);
}


/* static */
void
Script::SetDebug(bool debug)
{
	sDebug = debug;
}


void
Script::Print() const
{
	node *nextNode = fRootNode;
	while (nextNode != NULL) {
		_PrintNode(nextNode);
		nextNode = nextNode->next;
	}
}


/* static */
trigger_node*
Script::FindTriggerNode(Object* object, node* start)
{
	return static_cast<trigger_node*>(FindNode(object, BLOCK_TRIGGER, start));
}


/* static */
action_node*
Script::FindActionNode(Object* object, node* start)
{
	return static_cast<action_node*>(FindNode(object, BLOCK_ACTION, start));
}


/* static */
object_node*
Script::FindObjectNode(const Object* object, node* start)
{
	return static_cast<object_node*>(FindNode(object, BLOCK_OBJECT, start));
}


/* static */
node*
Script::FindNode(const Object* object, block_type nodeType, node* start)
{
	if (start->type == nodeType)
		return const_cast<node*>(start);

	node_list::const_iterator i;
	for (i = start->children.begin(); i != start->children.end(); i++) {
		node *n = FindNode(object, nodeType, *i);
		if (n != NULL)
			return n;
	}

	if (start->next != NULL)
		return FindNode(object, nodeType, start->next);

	return NULL;
}


/* static */
Object*
Script::FindTriggerObject(Object* object, trigger_node* start)
{
	object_node* objectNode = FindObjectNode(object, start);
	if (objectNode == NULL)
		return NULL;
	
	return GetObject(object, objectNode);
}


/* static */
Object*
Script::FindSenderObject(Object* object, action_node* start)
{
	object_node* objectNode = FindObjectNode(object, start);
	if (objectNode == NULL || objectNode->Empty()) {
		std::cout << "FindSenderObject returned " << (object ? object->Name() : "NULL") << std::endl;
		return object;
	}
	
	Object* result = GetObject(object, objectNode);
	if (result == NULL) {
		std::cout << "FindSenderObject returned " <<  (object ? object->Name() : "NULL") << std::endl;
		return object;
	}

	std::cout << "FindSenderObject returned " <<  result->Name() << std::endl;
	return result;
}


Object*
Script::FindTargetObject(Object* object, action_node* start)
{
	std::cout << "FindTargetObject:" << std::endl;
	object_node* objectNode = FindObjectNode(object, start);
	if (objectNode != NULL)
		objectNode = static_cast<object_node*>(objectNode->Next());

	if (objectNode == NULL || objectNode->Empty()) {
		std::cout << "FindTargetObject returned NULL" << std::endl;
		return NULL;
	}

	objectNode->Print();
	std::cout << std::endl;
	
	Object* result = GetObject(object, objectNode);
	std::cout << "FindTargetObject returned " << (result ? result->Name() : "NONE") << std::endl;
	return result;
}


bool
Script::Execute(bool& continuing)
{
	/*assert(fSender != NULL);
	if (fSender == NULL) {
		std::cerr << "Script::Execute(): fSender is NULL!" << std::endl;
	}
	*/	
	fLastTrigger = NULL;
	// for each CR block
	// for each CO block
	// check triggers
	// if any trigger
	// find response_set block
	// choose response
	// execute actions
	if (sDebug) {
		std::cout << "*** SCRIPT START: " << (fSender ? fSender->Name() : "");
		std::cout << " ***" << std::endl;
#if 0
		if (!strcmp(fSender->Name(), "Amnish Soldier")) {
			Print();		
		}
#endif
	}

	bool foundContinue = continuing;
	// TODO: Find correct place
	::node* condRes = FindNode(fSender, BLOCK_CONDITION_RESPONSE, fRootNode);
	while (condRes != NULL) {
		::node* condition = FindNode(fSender, BLOCK_CONDITION, condRes);
		while (condition != NULL) {
			if (sDebug)
				std::cout << "CONDITION" << std::endl;
			if (!_EvaluateConditionNode(condition))
				break;
				
			if (foundContinue) {
				std::cout << "CONTINUE(), EXIT SCRIPT" << std::endl;
				return true;
			}
				
			// Check action list
			if (fSender != NULL && !fSender->IsActionListEmpty()) {
				if (!fSender->IsInterruptable())
					break;
			}

			::node* responseSet = condition->Next();
			if (responseSet == NULL)
				break;
			if (sDebug)
				std::cout << "RESPONSE" << std::endl;
			if (!_HandleResponseSet(responseSet)) {
				// When the above method returns false, it means it encountered a CONTINUE
				// so the script should stop running.
				SetProcessed();
				foundContinue = true;
				return false;
			} else {
				if (sDebug) {
					std::cout << "*** SCRIPT RETURNED " << (fSender ? fSender->Name() : "");
					std::cout << " ***" << std::endl;
				}
				SetProcessed();
				return true;
			}
			condition = responseSet->Next();
		}
		condRes = condRes->Next();
	};
	if (sDebug) {
		std::cout << "*** SCRIPT END " << (fSender ? fSender->Name() : "");
		std::cout << " ***" << std::endl;
	}

	SetProcessed();

	return true;
}


Object*
Script::Sender()
{
	return fSender;
}


void
Script::SetSender(Object* object)
{
	fSender = object;
}


/* static */
Object*
Script::ResolveIdentifier(Object* object, const int id)
{
	std::string identifier = IDTable::ObjectAt(id);
	if (identifier == "MYSELF")
		return object;
	if (identifier.find("PLAYER") != std::string::npos) {
		// TODO: dangerous code
		char* n = &identifier[6];
		uint32 numPlayer = strtoul(n, NULL, 10) - 1;
		return Game::Get()->Party()->ActorAt(numPlayer);
	}

	// TODO: Implement more identifiers
	if (identifier == "NEARESTENEMYOF") {
		Actor* actor = dynamic_cast<Actor*>(object);
		if (actor == NULL)
			return NULL;
		return Core::Get()->GetNearestEnemyOf(actor);
	}
	// TODO: Move that one here ?

	if (identifier == "LASTSEENBY")
		return object->FindTrigger("LastSeen");
#if 0
	if (identifier == "LASTTRIGGER")
		return LastTrigger();
#endif
#if 1
	if (identifier == "LASTATTACKEROF")
		return object->FindTrigger("AttackedBy");
#endif

	std::cout << "ResolveIdentifier: UNIMPLEMENTED(" << id << ") = ";
	std::cout << identifier << std::endl;
	return NULL;
}


/* static */
Object*
Script::GetObject(Object* source, object_node* node)
{
	std::cout << "Script::GetObject() ";
	Object* result = NULL;
	if (node->name[0] != '\0') {
		std::cout << "Specified name: " << node->name << std::endl; 
		result = Core::Get()->GetObject(node->name);
	} else if (node->identifiers[0] != 0) {
		std::cout << "Specified identifiers: " ;
		// If there are any identifiers, use those to get the object
		Actor* target = NULL;
		for (int32 id = 0; id < 5; id++) {
			const int identifier = node->identifiers[id];
			if (identifier == 0) {
				break;
			}
			std::cout << IDTable::ObjectAt(identifier) << ", ";
			target = dynamic_cast<Actor*>(ResolveIdentifier(source, identifier));
			/*if (source != NULL)
				source->Print();*/
		}
		std::cout << std::endl;
		// TODO: Filter using wildcards in node
		/*std::cout << "returned ";
		if (target != NULL)
			std::cout << target->Name() << std::endl;
		else
			std::cout << "NONE" << std::endl;*/
		result = target;
	} else {
		std::cout << "Specified wildcards (BUGGY):" << std::endl;
		// Otherwise use the other parameters
		Object* wildCard = Core::Get()->GetObjectFromNode(node);
		if (wildCard != NULL)
			result = dynamic_cast<Actor*>(wildCard);
	}

	std::cout << "Script::GetObject() returned. ";
	if (result != NULL) {
		std::cout << "Found: ";
		result->Print();
	} else
		std::cout << "Found NONE" << std::endl;
	return result;
}


Object*
Script::LastTrigger() const
{
	return fLastTrigger;
}


void
Script::SetProcessed()
{
	fProcessed = true;
}


bool
Script::Processed() const
{
	return fProcessed;
}


/* static */
bool
Script::IsActionInstant(uint16 id) const
{
	std::string actionName = IDTable::ActionAt(id);
	IDSResource* instants = gResManager->GetIDS("INSTANT");
	if (instants == NULL)
		return false;

	bool returnValue = instants->StringForID(id) != "";
	std::cout << actionName << " is" << (returnValue ? "" : " not") << " instant." << std::endl;		
	gResManager->ReleaseResource(instants);

	return returnValue;
}


bool
Script::_EvaluateConditionNode(node* conditionNode)
{
	fOrTriggers = 0;

	trigger_node* trig = FindTriggerNode(fSender, conditionNode);
	bool blockEvaluation = true;
	while (trig != NULL) {
		if (fOrTriggers > 0) {
			blockEvaluation = _EvaluateTrigger(trig);
			if (blockEvaluation)
				break;
			fOrTriggers--;
		} else {
			blockEvaluation = _EvaluateTrigger(trig) && blockEvaluation;
			if (!blockEvaluation)
				break;
		}
		trig = static_cast<trigger_node*>(trig->Next());
	}
	if (sDebug) {
		std::cout << "SCRIPT: TRIGGER BLOCK returned ";
		std::cout << (blockEvaluation ? "TRUE" : "FALSE") << std::endl;
	}

	return blockEvaluation;
}


bool
Script::_EvaluateTrigger(trigger_node* trig)
{
	Actor* actor = dynamic_cast<Actor*>(fSender);
	//if (actor != NULL && actor->SkipConditions())
	//	return false;

	if (sDebug) {
		std::cout << "SCRIPT: TRIGGER ",
		std::cout << (trig->flags != 0 ? "!" : "");
		std::cout << IDTable::TriggerName(trig->id) << " (";
		std::cout << std::dec << trig->id << " ";
		std::cout << std::hex << trig->id << ")";
		std::cout << std::endl;
		trig->Print();
	}

	Core* core = Core::Get();
	bool returnValue = false;
	try {
		switch (trig->id) {
			case 0x0002:
			{
				/* 0x0002 AttackedBy(O:Object*,I:Style*AStyles)
				 * Returns true only if the active CRE was attacked in the style
				 * specified (not necessarily hit) or had an offensive spell cast
				 * on it by the specified object in the last script round.
				 * The style parameter is non functional - this trigger is triggered
				 * by any attack style.
				 */
				// TODO: fix ?
				returnValue = fSender->HasTrigger("AttackedBy", trig);
				break;
			}
			case 0x0020:
			{
				// HitBy
				// Returns true on first Script launch, IOW initial area load
				if (!Processed()) {
					returnValue = true;
					break;
				}
				//object_node* object = FindObjectNode(trig);
				//returnValue = Object::CheckIfNodeInList(object,
				//		fSender->LastScriptRoundResults()->Hitters());
				break;
			}
			case 0x0022:
			{
				/* TimerExpired(I:ID*) */
				std::ostringstream stringStream;
				stringStream << fSender->Name() << " " << trig->parameter1;
				std::cout << "TimerExpired " << stringStream.str() << std::endl;
				GameTimer* timer = GameTimer::Get(stringStream.str().c_str());
				if (timer != NULL && timer->Expired()) {
					returnValue = true;
				}
				break;
			}
			case 0x002F:
			{
				/* 0x002F Heard(O:Object*,I:ID*SHOUTIDS)
				Returns true only if the active CRE was within 30 feet
				of the specified object and the specified object shouted
				the specified number (which does not have to be in
				SHOUTIDS.ids) in the last script round.
				NB. If the object is specified as a death variable,
				the trigger will only return true if the corresponding
				object shouting also has an Enemy-Ally flag of NEUTRAL. */
#if 0
				Object* object = FindObject(fSender, trig);
				if (object != NULL && core->Distance(fSender, object) <= 30
						&& object->LastScriptRoundResults()->Shouted()
						== trig->parameter1) {
					returnValue = true;
				}
#endif
				break;
			}
			case 0x0036:
			{
				/*0x0036 OnCreation()
				Returns true if the script is processed for the first time this session,
				e.g. when a creature is created (for CRE scripts) or when the player
				enters an area (for ARE scripts).*/
				returnValue = fSender->HasTrigger("ONCREATION()");
				break;
			}
			case 0x004c:
			{
				// Entered(O:Object)
				//object_node* node = FindObjectNode(trig);
				//Region* region = dynamic_cast<Region*>(fSender);
				//std::vector<std::string>::const_iterator i;

				/*Object* object = Object::GetMatchingObjectFromList(
										region->
										LastScriptRoundResults()->
										EnteredActors(), node);*/
				//if (object != NULL) {
				//	fLastTrigger = object;
				//	returnValue = true;
				//}
				break;
			}
			case 0x0052:
			{
				/* OPENED(O:OBJECT*) (82 0x52) */
				object_node* objectNode = FindObjectNode(fSender, trig);
				if (objectNode == NULL)
					break;
				break;
				// TODO: We assume this is a door, but also
				// containers can be opened/closed
				/*Door* door = dynamic_cast<Door*>(fSender);
				if (door == NULL)
					break;
				if (!door->Opened())
					break;*/
#if 0
				Object* object = core->GetObjectFromNode(
						door->CurrentScriptRoundResults()->fOpenedBy.c_str());
				if (object != NULL)
					returnValue = object->MatchNode(objectNode);
#endif
				break;
			}
			case 0x0070:
			{
				/* 0x0070 Clicked(O:Object*)
				 *	Only for trigger regions.
				 *	Returns true if the specified object
				 *	clicked on the trigger region running this script.
				 */
				object_node* objectNode = FindObjectNode(fSender, trig);
				// TODO: Maybe LastClicker should return Actor*
				Actor* clicker = dynamic_cast<Actor*>(fSender->LastClicker());
				if (clicker != NULL) {
					objectNode->Print();
					returnValue = clicker->MatchNode(objectNode);
				}
				// TODO: When to set this, other than now ?
				//if (returnValue)
					//fLastTrigger = fSender;

				break;
			}
			case 0x400A:
			{
				//ALIGNMENT(O:OBJECT*,I:ALIGNMENT*Align) (16395 0x400A)
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = object->IsAlignment(trig->parameter1);
				break;
			}
			case 0x400B:
			{
				//ALLEGIANCE(O:OBJECT*,I:ALLEGIENCE*EA) (16395 0x400b)
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = object->IsEnemyAlly(trig->parameter1);
				break;
			}
			case 0x400C:
			{
				/*0x400C Class(O:Object*,I:Class*Class)*/
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = object->IsClass(trig->parameter1);
				break;
			}
			case 0x400E:
			{
				/* GENERAL(O:OBJECT*,I:GENERAL*GENERAL) (16398 0x400e)*/
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL) {
					returnValue = object->IsGeneral(trig->parameter1);
					if (sDebug) {
						std::cout << "object: " << IDTable::GeneralAt(object->CRE()->General());
						std::cout << " vs " << IDTable::GeneralAt(trig->parameter1) << std::endl;				
					}				
				}
				break;
			}
			case 0x400F:
			{
				/*0x400F Global(S:Name*,S:Area*,I:Value*)
				Returns true only if the variable with name 1st parameter
				of type 2nd parameter has value 3rd parameter.*/
				std::string variableScope;
				std::string variableName;
				VariableGetScopeName(trig->string1, variableScope, variableName);
				int32 variableValue = 0;
				if (variableScope.compare("LOCALS") == 0) {
					variableValue = fSender->Vars().Get(variableName.c_str());
				} else {
					// TODO: Check for AREA variables, currently we
					// treat AREA variables as global variables
					variableValue = Core::Get()->Vars().Get(trig->string1);
				}
				returnValue = variableValue == trig->parameter1;
				break;
			}
			case 0x4017:
			{
				// Race()
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = object->IsRace(trig->parameter1);
				break;
			}
			case 0x4018:
			{
				/* 0x4018 Range(O:Object*,I:Range*)
				Returns true only if the specified object
				is within distance given (in feet) of the active CRE. */
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = core->Distance(object, fSender) <= trig->parameter1;
				break;
			}
			case 0x401C:
			{
				/* See(O:Object*)
				 * Returns true only if the active CRE can see
				 * the specified object which must not be hidden or invisible.
				 */
				returnValue = actor->CanSee(FindTriggerObject(fSender, trig));
				std::cout << (returnValue ? "TRUE" : "FALSE") << std::endl;
				break;
			}
			case 0x401E:
			{
				/* Time(I:Time*Time)
				 * Returns true only if the period of day matches the period
				 * in the 2nd parameter (taken from Time.ids).
				 * Hours are offset by 30 minutes, e.g. Time(1) is true
				 * between 00:30 and 01:29.
				 */
				if (GameTimer::HourOfDay() == trig->parameter1)
					returnValue = true;
				break;
			}
			case 0x4023:
				/* 0x4023 True() */
				returnValue = true;
				break;
			case 0x4027:
			{
				//DELAY(I:DELAY*) (16423 0x4027)
				// TODO: Implement

				returnValue = true;
				break;
			}
			case 0x402b:
			{
				/* ACTIONLISTEMPTY() (16427 0x402b)*/
				returnValue = actor->IsActionListEmpty();
				break;
			}
			case 0x4034:
			{
				/*0x4034 GlobalGT(S:Name*,S:Area*,I:Value*)
				See Global(S:Name*,S:Area*,I:Value*) except the variable
				must be greater than the value specified to be true.
				*/
				returnValue = core->Vars().Get(trig->string1) > trig->parameter1;
				break;
			}
			case 0x4035:
			{	/*
				0x4035 GlobalLT(S:Name*,S:Area*,I:Value*)
				As above except for less than. */
				returnValue = core->Vars().Get(trig->string1) < trig->parameter1;
				break;
			}
			case 0x4037:
			{
				/* StateCheck(O:Object*,I:State*State)
				 * Returns true only if the specified object
				 * is in the state specified.
				 */
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = object->IsState(trig->parameter1);
				break;
			}
			case 0x4039:
			{
				/* NUMBEROFTIMESTALKEDTO(I:NUM*) (16441 0x4039) */
				if (actor->NumTimesTalkedTo() == (uint32)trig->parameter1)
					returnValue = true;
				break;
			}
			case 0x4040:
			{
				/* GlobalTimerExpired(S:Name*,S:Area*) (16448 0x4040) */
				// TODO: Merge Name and Area before passing them to the
				// Timer::Get() method (also below)
				std::string timerName;
				timerName.append(trig->string2).append(trig->string1);
				GameTimer* timer = GameTimer::Get(timerName.c_str());
				returnValue = timer != NULL && timer->Expired();
				break;
			}
			case 0x4041:
			{
				/* GlobalTimerNotExpired(S:Name*,S:Area*) */
				std::string timerName;
				timerName.append(trig->string2).append(trig->string1);
				GameTimer* timer = GameTimer::Get(timerName.c_str());
				returnValue = timer == NULL || !timer->Expired();
				break;
			}
			case 0x4043:
			{
				// InParty
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = Game::Get()->Party()->HasActor(object);
				break;
			}
			case 0x4051:
			{
				/*
				 * 0x4051 Dead(S:Name*)
				 * Returns only true if the creature with the specified script name
				 * has its death variable set to 1. Not every form of death sets this,
				 * but most do. So it's an almost complete test for death.
				 * The creature must have existed for this to be true.
				 * Note that SPRITE_IS_DEAD variables are not set if the creature is
				 * killed by a neutral creature.
				 */
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (actor != NULL) {
					const char* deathVariable = object->CRE()->DeathVariable();
					returnValue = object->Vars().Get(deathVariable) == 1;
				}
				break;
			}
			case 0x4063:
			{
				/*INWEAPONRANGE(O:OBJECT*) (16483 0x4063) */
				int range = 40;
				// TODO: Check weapon range
				Actor* object = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (object != NULL)
					returnValue = core->Distance(actor, fSender) <= range;
				break;
			}
			case 0x4068:
			{
				/* TimeGT(I:Time*Time)
				 * Returns true only if the current time is greater than
				 * that specified. Hours are offset by 30 minutes,
				 * e.g. TimeGT(1) is true between 01:30 and 02:29.
				 */
				// TODO: Offset hour
				if (GameTimer::HourOfDay() > trig->parameter1)
					returnValue = true;
				break;
			}
			case 0x4069:
			{
				//TIMELT(I:TIME*TIME) (16489 0x4069)
				// TODO: Offset hour
				if (GameTimer::HourOfDay() < trig->parameter1)
					returnValue = true;
				break;
			}
			case 0x4074:
			{
				/*
				 * 0x4074 Detect(O:Object*)
				 * Returns true if the the specified object
				 * is detected by the active CRE in any way
				 * (hearing or sight). Neither Move Silently
				 * and Hide in Shadows prevent creatures
				 * being detected via Detect().
				 * Detect ignores Protection from Creature
				 * type effects for static objects.
				 */
				// TODO: This is valid for Regions scripts. What is the "Active CRE" ?
				if (actor == NULL)
					break;
				returnValue = actor->CanSee(FindTriggerObject(fSender, trig));
					// || core->Hear(fSender, object);
				break;
			}
			case 0x4076:
			{
				/*
				 * 0x4076 OpenState(O:Object*,I:Open*BOOLEAN)
				 * NT Returns true only if the open state of the specified door
				 * matches the state specified in the 2nd parameter.
				 */
				object_node* doorObj = FindObjectNode(fSender, trig);
				Door *door = dynamic_cast<Door*>(
								core->GetObject(doorObj->name));
				if (door != NULL) {
					bool openState = trig->parameter1 == 1;
					returnValue = door->Opened() == openState;
				}
				break;
			}
			case 0x407D:
			{
				/* ISOVERME(O:Object*)
				 * Only for trigger regions. Returns true only if the specified
				 * object is over the trigger running the script
				 */
				Region* region = dynamic_cast<Region*>(fSender);
				if (region == NULL)
					break;
				object_node* objectNode = FindObjectNode(fSender, trig);
				if (objectNode != NULL) {
					// TODO: won't work if the object is generic and not specified					
					Actor* actor = core->GetObjectFromNode(objectNode);
					if (actor != NULL)
						returnValue = region->Contains(actor->Position());
				}

				break;
			}
			case 0x407E:
			{
				/* 0x407E AreaCheck(S:ResRef*)
				 * Returns true only if the active CRE
				 * is in the area specified.
				 */
				// TODO: We only check the active area
				returnValue = !strcmp(core->CurrentRoom()->Name(), trig->string1);
				break;
			}
			case 0x4086:
			{
				// AREATYPE(I:NUMBER*AREATYPE) (16518 0x4086)

				// TODO: We only check the active area
				//const uint16 areaType = RoomContainer::Get()->AREA()->Type();
				//returnValue = areaType & trig->parameter1;
				break;
			}
			case 0x4089:
			{
				/*0x4089 OR(I:OrCount*)*/
				fOrTriggers = trig->parameter1;
				returnValue = true;
				break;
			}
			case 0x401b:
			{
				/* REPUTATIONLT(O:OBJECT*,I:REPUTATION*) (16411 0x401b) */
				Actor* actor = dynamic_cast<Actor*>(FindTriggerObject(fSender, trig));
				if (actor != NULL) {
					returnValue = actor->CRE()->Reputation() < trig->parameter1;
				}
				break;
			}
			default:
			{
				if (sDebug) {
					std::cout << "SCRIPT: UNIMPLEMENTED TRIGGER!!!" << std::endl;
				}
				break;
			}
		}
	} catch (...) {
		std::cerr << "SCRIPT: EvaluateTrigger() caught exception" << std::endl;
	}
	if (trig->flags != 0)
		returnValue = !returnValue;
	return returnValue;
}


bool
Script::_HandleResponseSet(node* responseSet)
{
	response_node* responses[5];
	response_node* response = static_cast<response_node*>(
			FindNode(fSender, BLOCK_RESPONSE, responseSet));
	int i = 0;
	int totalChance = 0;
	while (response != NULL) {
		responses[i] = response;
		totalChance += responses[i]->probability;
		response = static_cast<response_node*>(response->Next());
		i++;
	}
	
	for (int p = 0; p < i; p++) {
		std::cout << "response " << p << ": probability ";
		std::cout << std::dec << responses[p]->probability << std::endl;
	}
	// TODO: Fix this and take the probability into account
	int randomResponse = Core::RandomNumber(0, i);
	action_node* action = FindActionNode(fSender, responses[randomResponse]);
	// More than one action
	while (action != NULL) {
		// When _HandleAction() returns false,
		// it means it's a CONTINUE() action.
		if (!_HandleAction(action)) {
			std::cout << "Continuing. Script returns" << std::endl;
			return false;
		}
		action = static_cast<action_node*>(action->Next());
	}
	return true;
}


bool
Script::_HandleAction(action_node* act)
{
	Object* sender = Script::FindSenderObject(fSender, act);
	if (sDebug) {
		std::cout << "SCRIPT: ACTION ";
		std::cout << "Sender: " << (sender ? sender->Name() : "") << std::endl;
		act->Print();
		std::cout << std::endl;
	}

	Core* core = Core::Get();
	
	Actor* thisActor = dynamic_cast<Actor*>(sender);
	
	bool runNow = Script::IsActionInstant(act->id);
	switch (act->id) {
		case 1:
		{
			std::cout << "ActionOverride()" << std::endl;
			break;
		}
		case 3:
		{
			/* Attack(O:Target*) */
			Attack* attackAction = new Attack(sender, act);
			sender->AddAction(attackAction);
			break;
		}
		case 7:
		{
			/* CreateCreature(S:NewObject*,P:Location*,I:Face*) */
			// TODO: If point is (-1, -1) we should put the actor near
			// the active creature. Which one is the active creature?
			Action* action = new CreateCreatureAction(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 8:
		{
			/*	DIALOGUE(O:OBJECT*) (8 0x8) */
			/*Actor* actor = dynamic_cast<Actor*>(FindObject(act));
			if (actor != NULL) {
				Dialogue* dialogueAction = new Dialogue(thisActor, actor);
				thisActor->AddAction(dialogueAction);
			}*/
			break;
		}
		case 10:
		{
			/* ENEMY() (10 0xa) */
			uint32 id = IDTable::EnemyAllyValue("ENEM");
			thisActor->SetEnemyAlly(id);
			break;
		}
		case 22:
		{
			// MoveToObject
			Action* walkTo = new WalkToObject(thisActor, act);
			thisActor->AddAction(walkTo);
			break;
		}
		case 23:
		{
			// MoveToPoint
			if (thisActor != NULL) {
				WalkTo* walkTo = new WalkTo(thisActor, act);
				thisActor->AddAction(walkTo);
				thisActor->StopCheckingConditions();
			}
			break;
		}
		case 29:
		{
			/* RunAwayFrom(O:Creature*,I:Time*) */
			if (thisActor != NULL) {
				RunAwayFrom* run = new RunAwayFrom(thisActor, act);
				thisActor->AddAction(run);
			}
			break;
		}
		case 30:
		{
			// SetGlobal
			sender->AddAction(new SetGlobalAction(sender, act), runNow);
			break;
		}
		case 36:
		{
			/*
			 * 36 Continue()
			 * This action instructs the script parser to continue looking
			 * for actions in the active creatures action list.
			 * This is mainly included in scripts for efficiency.
			 * Continue should also be appended to any script blocks added
			 * to the top of existing scripts, to ensure correct functioning
			 * of any blocks which include the OnCreation trigger.
			 * Continue may prevent actions being completed until the script
			 * parser has finished its execution cycle. Continue() must be
			 * the last command in an action list to function correctly.
			 * Use of continue in a script block will cause the parser
			 * to treater subsequent empty response blocks as though they
			 * contained a Continue() command - this parsing can be stopped
			 * by including a NoAction() in the empty response block.
			 */
			// by returning false, we instruct the caller to stop
			// executing the script
			std::cout << "CONTINUE!!!!" << std::endl;
			return false;
		}
		case 40:
		{
			// 40 PlayDead(I:Time*)
			// This action instructs the active creature to "play dead",
			// i.e. to lay on the ground, for the specified interval
			// (measured in AI updates per second (AI updates default to 15 per second).
			// If used on a PC, the player can override the action by
			// issuing a standard move command.
			if (thisActor != NULL) {
				Action* action = new PlayDeadAction(thisActor, act);
				thisActor->AddAction(action);
			}
			break;
		}
		case 49:
		{
			/* MOVEVIEWPOINT(P:TARGET*,I:SCROLLSPEED*SCROLL)(49 0x31) */
			Action* action = new MoveViewPoint(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 61:
		{
			/* 61 StartTimer(I:ID*,I:Time*)
			This action starts a timer local to the active creature.
			The timer is measured in seconds, and the timer value is
			not saved in save games. The timer is checked with the
			TimerExpired trigger.*/

			// TODO: We should add the timer local to the active creature,
			// whatever that means
			if (sender == NULL)
				printf("NULL TARGET\n");
			std::ostringstream stringStream;
			stringStream << sender->Name() << " " << act->integer1;

			GameTimer::Add(stringStream.str().c_str(), act->integer2);

			break;
		}
		case 63:
		{
			/* WAIT(I:TIME*)(63 0x3f) */
			Wait* wait = new Wait(sender, act);
			sender->AddAction(wait);
			break;
		}
		case 84:
		{
			/* 84 (0x54) FACE(I:DIRECTION) */
			if (thisActor != NULL) {
				ChangeOrientationExtAction* action = new ChangeOrientationExtAction(thisActor, act);
				thisActor->AddAction(action);
			}			
			break;
		}
		case 85:
		{
			/* 85 RandomWalk */
			sender->AddAction(new RandomWalk(sender, act));
			break;
		}
		case 86:
		{
			/* 86 SetInterrupt(I:State*Boolean) */
			sender->AddAction(new SetInterruptableAction(sender, act), runNow);
			break;
		}
		case 0x53:
		{
			/* 83 SmallWait(I:Time*) */
			Action* wait = new SmallWait(sender, act);
			sender->AddAction(wait);
			break;
		}

		case 106:
		{
			/* Shout */
			// Check if target is silenced
			/*if (thisActor != NULL)
				thisActor->Shout(act->integer1);*/
			break;
		}
		case 0x64:
		{
			/* 100 RandomFly */
			/*if (thisActor != NULL && thisActor->IsInterruptable())
				core->RandomFly(thisActor);*/
			break;
		}
		case 0x65:
		{
			/* 101 FlyToPoint(Point, Time) */
			/*if (thisActor != NULL && thisActor->IsInterruptable()) {
				FlyTo* flyTo = new FlyTo(thisActor, act->where, act->integer1);
				thisActor->AddAction(flyTo);
			}*/
			break;
		}
		case 109:
		{	
			// INCREMENTGLOBAL(S:NAME*,S:AREA*,I:VALUE*) (109 0x6d)
			std::string variableScope;
			std::string variableName;
			VariableGetScopeName(act->string1, variableScope, variableName);			
			int32 value = core->Vars().Get(act->string1);
			core->Vars().Set(act->string1, value + act->integer1);
			break;		
		}		
		case 111:
		{
			/* DESTROYSELF() (111 0x6f) */
			// TODO: Add as action			
			std::cout << "DESTROY SELF" << std::endl;
			Action* action = new DestroySelfAction(thisActor, act);
			thisActor->AddAction(action, runNow);
			break;
		}
		case 113:
		{	
			// FORCESPELL(O:TARGET,I:SPELL*SPELL)(113, 0x71)
			sender->AddAction(new ForceSpell(sender, act));
			break;
		}
		case 115:
		{
			/* SETGLOBALTIMER(S:NAME*,S:AREA*,I:TIME*GTIMES) (115 0x73)*/
			std::string timerName;
			// TODO: We append the timer name to the area name,
			// check if it's okay
			timerName.append(act->string2).append(act->string1);
			GameTimer::Add(timerName.c_str(), act->integer1);
			break;
		}
		case 120:
		{
			/* STARTCUTSCENE(S:CUTSCENE*)(120 0x78) */
			sender->AddAction(new StartCutsceneAction(sender, act), runNow);
			break;
		}
		case 121:
		{
			/* STARTCUTSCENEMODE()(121 0x79) */
			sender->AddAction(new StartCutsceneModeAction(sender, act), runNow);
			break;
		}
		case 127:
		{
			/* CUTSCENEID(O:OBJECT*)(127 0x7f) */
			// TODO: Should be correct			
			//Actor* actor = dynamic_cast<Actor*>(FindObject(sender, act));
			Object* target = Script::FindTargetObject(fSender, act);
			if (target != NULL) {
				std::cout << "CUTSCENEID: " << target->Name() << std::endl;
				Core::Get()->SetCutsceneActor(target);
				SetSender(target);
				target->SetInterruptable(false);
			}
			break;
		}
		case 0x97:
		{
			/* 151 DisplayString(O:Object*,I:StrRef*)
			 * This action displays the strref specified by the StrRef parameter
			 * in the message window, attributing the text to
			 * the specified object.
			 */
			//core->DisplayMessage(act->integer1);
			break;
		}
		case 0xA7:
		{
			core->PlayMovie(act->string1);
			break;
		}
		case 134:
		{
			/* AttackReevaluate(O:Target*,I:ReevaluationPeriod*)
			 *  (134 0x86)
			 */
			Attack* attackAction = new Attack(thisActor, act);
			thisActor->AddAction(attackAction);

			break;
		}
		case 143:
		{
			/* OPENDOOR(O:OBJECT*)(143 0x8f) */
			sender->AddAction(new OpenDoor(sender, act));
			break;
		}
		case 177:
		{
			/* TRIGGERACTIVATION(O:OBJECT*,I:STATE*BOOLEAN)(177 0xb1) */
			Action* action = new TriggerActivationAction(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 198:
		{
			/* STARTDIALOGNOSET(O:OBJECT*) (198 0xc6) */
			// TODO: Implement more correctly.
			/*Actor* actor = dynamic_cast<Actor*>(FindObject(fSender, act));
			if (actor != NULL) {
				Dialogue* dialogueAction = new Dialogue(thisActor, actor);
				thisActor->AddAction(dialogueAction);
			}*/
			break;
		}
		case 202:
		{	
			/* FADETOCOLOR(P:POINT*,I:BLUE*) (202 0xca) */
			FadeToColorAction* action = new FadeToColorAction(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 203:
		{
			/* FADEFROMCOLOR(P:POINT*,I:BLUE*)(203 0xcb) */
			FadeFromColorAction* action = new FadeFromColorAction(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 207:
		{
			/* 207 MoveToPointNoInterrupt(P:Point*)
			 * This action causes the active creature to move to the specified coordinates.
			 * The action will update the position of creatures as stored in ARE files
			 * (first by setting the coordinates of the destination point, then by setting
			 * the coordinates of the current point once the destination is reached).
			 * Conditions are not checked until the destination point is reached.*/
			if (thisActor != NULL) {
				WalkTo* walkTo = new WalkTo(thisActor, act);
				thisActor->AddAction(walkTo);
				thisActor->StopCheckingConditions();
			}
			break;
		}
		case 225:
		{
			/* MOVEBETWEENAREASEFFECT(S:AREA*,S:EFFECT*,P:LOCATION*,I:FACE*)(225 0xe1) */
			// Active creature. Which is it ? For now, we use actor 0 in party		
			Action* action = new MoveBetweenAreasEffect(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 228: // 0xe4
		{
			/* CreateCreatureImpassable(S:NewObject*,P:Location*,I:Face*) (228 0xe4) */
			/* This action creates the specified creature
			 * on a normally impassable surface (e.g. on a wall,
			 * on water, on a roof). */
			Action* action = new CreateCreatureImpassableAction(sender, act);
			sender->AddAction(action, runNow);
			break;
		}
		case 229:
		{
			// FACEOBJECT(O:OBJECT*)
			Action* action = new FaceObject(sender, act);
			sender->AddAction(action);
			break;
		}
		case 254:
		{
			/* SCREENSHAKE(P:POINT*,I:DURATION*)(254 0xfe) */
			Action* action = new ScreenShake(sender, act);
			sender->AddAction(action);
			break;
		}
		case 269:
		{
			// DISPLAYSTRINGHEAD(O:OBJECT*,I:STRREF*)(269 0x10d)
			Action* action = new DisplayStringHead(sender, act);
			sender->AddAction(action);
			break;
		}
		case 272:
		{
			// 272 CreateVisualEffect(S:Object*,P:Location*) 0x110
			sender->AddAction(new CreateVisualEffect(sender, act), runNow);
			break;
		}
		case 273:
		{
			// CREATEVISUALEFFECTOBJECT(S:DIALOGFILE*,O:TARGET*) 
			sender->AddAction(new CreateVisualEffectObject(sender, act), runNow);			
			break;
		}
		case 286: // 0x11e
		{
			/* HIDEGUI */
			sender->AddAction(new HideGUIAction(sender, act), runNow);
			break;
		}
		case 287:
		{
			/* UNHIDEGUI()(287 0x11f) */
			sender->AddAction(new UnhideGUIAction(sender, act), runNow);
			break;
		}
		case 311:
		{
			/* DISPLAYSTRINGWAIT(O:OBJECT*,I:STRREF*)(311 0x137) */
			// This action displays the specified string over the head
			// on the specified object (on the game-screen).
			// The text stays onscreen until the associated sound has completed playing.
			// TODO: use an action which plays the sound
			Action* action = new DisplayStringHead(sender, act);
			sender->AddAction(action);
			break;
		}
		default:
			if (sDebug) {
				std::cout << "SCRIPT: UNIMPLEMENTED ACTION!!!" << std::endl;
			}
			break;
	}

	return true;
}


void
Script::_PrintNode(node* n) const
{
	PrintIndentation();
	std::cout << "<" << n->header << ">";
	n->Print();
	if (n->type == BLOCK_SCRIPT
		|| n->type == BLOCK_CONDITION_RESPONSE
		|| n->type == BLOCK_CONDITION
		|| n->type == BLOCK_RESPONSESET) {
		std::cout << std::endl;
		PrintIndentation();
	}
	node_list::iterator c;
	IndentMore();
	bool printCount = false;
	int32 count = 0;
	if (n->type == BLOCK_ACTION && n->children.size() == 3) {
		printCount = true;
	}
	for (c = n->children.begin(); c != n->children.end(); c++) {
		if (printCount)
			std::cout << "(" << count << ") ";
		_PrintNode(*c);
		if (printCount)
			count++;
	}
	IndentLess();
	if (n->type != BLOCK_OBJECT)
		PrintIndentation();
	std::cout << "</" << n->header << ">" << std::endl;
}


void
Script::_DeleteNode(::node* node)
{
	if (node != NULL) {
		if (node->next != NULL)
			_DeleteNode(node->next);
		delete node;
	}
}
