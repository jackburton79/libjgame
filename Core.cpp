#include "Core.h"

#include "Action.h"
#include "Actor.h"
#include "CreResource.h"
#include "Door.h"
#include "GUI.h"
#include "IDSResource.h"
#include "MveResource.h"
#include "Party.h"
#include "RectUtils.h"
#include "Region.h"
#include "ResManager.h"
#include "Room.h"
#include "Script.h"
#include "TextArea.h"
#include "Timer.h"
#include "TLKResource.h"
#include "Window.h"

#include <algorithm>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <vector>


static Core* sCore = NULL;

//const static uint32 kRoundDuration = 6000; // 6 second. Actually this is the

Core::Core()
	:
	fGame(GAME_BALDURSGATE2),
	fCurrentRoom(NULL),
	fActiveActor(NULL),
	fRoomScript(NULL),
	fLastScriptRoundTime(0),
	fPaused(false)
{
	srand(time(NULL));
}


Core::~Core()
{
	delete fCurrentRoom;
}


/* static */
Core*
Core::Get()
{
	return sCore;
}


bool
Core::Initialize(const char* path)
{
	if (sCore != NULL)
		return true;

	try {
		sCore = new Core();
	} catch (...) {
		return false;
	}

	if (!gResManager->Initialize(path))
		return false;

	// Detect game
	// TODO: Find a better/safer way
	std::cout << "Detecting game... ";
	std::vector<std::string> stringList;
	if (gResManager->GetResourceList(stringList, "CSJON", RES_CRE) == 1) {
		sCore->fGame = GAME_BALDURSGATE2;
		std::cout << "GAME_BALDURSGATE2" << std::endl;
	} else {
		sCore->fGame = GAME_BALDURSGATE;
		std::cout << "GAME_BALDURSGATE" << std::endl;
	}

	std::cout << std::endl;


	// TODO: Move this elsewhere.
	// This should be filled by the player selection

	try {
		IE::point point = { 20, 20 };
		Party* party = Party::Get();
		if (sCore->fGame == GAME_BALDURSGATE)
			party->AddActor(new Actor("AJANTI", point, 0));
		else
			party->AddActor(new Actor("AESOLD", point, 0));
	} catch (...) {

	}
	return true;
}


void
Core::Destroy()
{
	delete sCore;
}


void
Core::TogglePause()
{
	fPaused = !fPaused;
}


uint32
Core::Game() const
{
	return fGame;
}


void
Core::EnteredArea(Room* area, Script* script)
{
	// TODO: Move this elsewhere
	fCurrentRoom = area;

	area->SetScript(script);
	SetRoomScript(script);

	// The area script
	if (fRoomScript != NULL) {
		fRoomScript->Execute();
	}

	area->SetScript(NULL);

	_PrintObjects();
}


void
Core::SetVariable(const char* name, int32 value)
{
	std::cout << "SetVariable(" << name << ", " << value;
	std::cout << " (old value: " << fVariables[name] << ")";
	std::cout << std::endl;
	fVariables[name] = value;
}


int32
Core::GetVariable(const char* name)
{
	std::cout << "GetVariable(" << name << "): " << fVariables[name];
	std::cout << std::endl;
	return fVariables[name];
}


void
Core::RegisterObject(Object* object)
{
	fObjects.push_back(object);
}


void
Core::UnregisterObject(Object* object)
{
	fObjects.remove(object);
}


Object*
Core::GetObject(Object* source, object_node* node) const
{
	// TODO: Move into object_node::Print()
	/*std::cout << "Core::GetObject(";
	std::cout << "source: " << source->Name() << ", ";
	std::cout << "node: ( ";
	if (node->name[0])
		std::cout << node->name;
	if (node->general)
		std::cout << ", " << IDTable::GeneralAt(node->general);
	if (node->classs)
		std::cout << ", " << IDTable::ClassAt(node->classs);
	if (node->specific)
		std::cout << ", " << IDTable::SpecificAt(node->specific);
	if (node->ea)
		std::cout << ", " << IDTable::EnemyAllyAt(node->ea);
	if (node->gender)
		std::cout << ", " << IDTable::GenderAt(node->gender);
	if (node->race)
		std::cout << ", " << IDTable::RaceAt(node->race);
	if (node->alignment)
		std::cout << ", " << IDTable::AlignmentAt(node->alignment);
	std::cout << ") ) -> " ;
*/
	if (node->name[0] != '\0')
		return GetObject(node->name);

	// If there are any identifiers, use those to get the object
	if (node->identifiers[0] != 0) {
		Object* target = NULL;
		for (int32 id = 0; id < 5; id++) {
			const int identifier = node->identifiers[id];
			if (identifier == 0)
				break;
			//std::cout << IDTable::ObjectAt(identifier) << ", ";
			target = source->ResolveIdentifier(identifier);
			source = target;
		}
		// TODO: Filter using wildcards in node
	/*	std::cout << "returned ";
		if (target != NULL)
			std::cout << target->Name() << std::endl;
		else
			std::cout << "NONE" << std::endl;*/
		return target;
	}


	// Otherwise use the other parameters
	// TODO: Simplify, merge code.
	std::list<Object*>::const_iterator i;
	for (i = fObjects.begin(); i != fObjects.end(); i++) {
		if ((*i)->MatchNode(node)) {
			//std::cout << "returned " << (*i)->Name() << std::endl;
			//(*i)->Print();
			return *i;
		}
	}

	//std::cout << "returned NONE" << std::endl;
	return NULL;
}


Object*
Core::GetObject(const char* name) const
{
	std::list<Object*>::const_iterator i;
	for (i = fObjects.begin(); i != fObjects.end(); i++) {
		if (!strcmp(name, (*i)->Name())) {
		//	std::cout << "returned " << name << std::endl;
			return *i;
		}
	}

	//std::cout << "returned NONE" << std::endl;
	return NULL;
}


Object*
Core::GetObject(const Region* region) const
{
	// TODO: Only returns the first object!
	std::list<Object*>::const_iterator i;
	for (i = fObjects.begin(); i != fObjects.end(); i++) {
		Actor* actor = dynamic_cast<Actor*>(*i);
		if (actor == NULL)
			continue;
		if (region->Contains(actor->Position()))
			return actor;
	}

	return NULL;
}


Object*
Core::GetNearestEnemyOf(const Object* object) const
{
	std::list<Object*>::const_iterator i;
	int minDistance = INT_MAX;
	Actor* nearest = NULL;
	for (i = fObjects.begin(); i != fObjects.end(); i++) {
		Actor* actor = dynamic_cast<Actor*>(*i);
		if (actor == NULL)
			continue;
		if ((*i) != object && (*i)->IsEnemyOf(object)) {
			int distance = Distance(object, *i);
			if (distance < minDistance) {
				minDistance = distance;
				nearest = actor;
			}
		}
	}
	if (nearest != NULL) {
		std::cout << "Nearest Enemy of " << object->Name();
		std::cout << " is " << nearest->Name() << std::endl;
	}
	return nearest;
}


void
Core::PlayMovie(const char* name)
{
	MVEResource* resource = gResManager->GetMVE(name);
	if (resource != NULL) {
		resource->Play();
		gResManager->ReleaseResource(resource);
	}
}


void
Core::DisplayMessage(uint32 strRef)
{
	// TODO: Move away from Core ? this adds too many
	// dependencies
	TLKEntry* entry = Dialogs()->EntryAt(strRef);
	std::cout << entry->string << std::endl;
	if (Window* window = GUI::Get()->GetWindow(4)) {
		TextArea *textArea = dynamic_cast<TextArea*>(
									window->GetControlByID(3));
		if (textArea != NULL)
			textArea->SetText(entry->string);
	}
	delete entry;
}



void
Core::SetRoomScript(Script* script)
{
	fRoomScript = script;
}


void
Core::NewScriptRound()
{

}


void
Core::CheckScripts()
{
}


void
Core::UpdateLogic(bool executeScripts)
{
	if (fPaused)
		return;

	Timer::UpdateGameTime();

	// TODO: Not nice, should stop the scripts in some other way
	if (strcmp(Room::Get()->AreaName().CString(), "WORLDMAP") == 0)
		return;

	// TODO: Fix/Improve
	std::list<Object*>::iterator i;
	for (i = fObjects.begin(); i != fObjects.end(); i++) {
		(*i)->Update(executeScripts);
	}

	fActiveActor = NULL;

	_RemoveStaleObjects();
}


bool
Core::See(const Object* source, const Object* object) const
{
	// TODO: improve
	if (object == NULL)
		return false;

	const Actor* sourceActor = dynamic_cast<const Actor*>(source);
	if (sourceActor == NULL)
		return false;

	std::cout << source->Name() << " SEE ";
	std::cout << object->Name() << " ?" << std::endl;

	return sourceActor->HasSeen(object);
}


int
Core::Distance(const Object* a, const Object* b) const
{
	const IE::point positionA = a->Position();
	const IE::point positionB = b->Position();

	IE::point invalidPoint = { -1, -1 };
	if (positionA == invalidPoint && positionB == invalidPoint)
		return 100; // TODO: ???

	return positionA - positionB;
}


void
Core::Open(Object* actor, Door* door)
{
	if (!door->Opened())
		door->Open(actor);
}


void
Core::Close(Object* actor, Door* door)
{
	if (door->Opened())
		door->Close(actor);
}


void
Core::RandomFly(Actor* actor)
{
	int16 randomX = (rand() % 200) - 100;
	int16 randomY = (rand() % 200) - 100;

	FlyToPoint(actor, offset_point(actor->Position(), randomX,
							randomY), 1);
}


void
Core::FlyToPoint(Actor* actor, IE::point point, uint32 time)
{
	// TODO:
	actor->SetFlying(true);
	//if (rect_contains(fCurrentRoom->AreaRect(), point))
	actor->SetDestination(point);
}


void
Core::RandomWalk(Actor* actor)
{
	int16 randomX = (rand() % 100) - 50;
	int16 randomY = (rand() % 100) - 50;

	IE::point destination = offset_point(actor->Position(), randomX, randomY);
	WalkTo* walkTo = new WalkTo(actor, destination);
	actor->AddAction(walkTo);
	//actor->SetFlying(false);
}


/* static */
int32
Core::RandomNumber(int32 start, int32 end)
{
	return start + rand() % (end - start);
}


const std::list<Object*>&
Core::Objects() const
{
	return fObjects;
}


void
Core::_PrintObjects() const
{
	for (std::list<Object*>::const_iterator i = fObjects.begin();
											i != fObjects.end(); i++) {
		(*i)->Print();
	}
}


void
Core::_RemoveStaleObjects()
{
	std::list<Object*>::iterator i = fObjects.begin();
	while (i != fObjects.end()) {
		if ((*i)->IsStale()) {
			if (Actor* actor = dynamic_cast<Actor*>(*i)) {
				Room::Get()->ActorExitedArea(actor);
				delete *i;
			} else
				delete *i;
			i = fObjects.erase(i);
		} else
			i++;
	}
}
