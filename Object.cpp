/*
 * Object.cpp
 *
 *  Created on: 12/lug/2012
 *      Author: stefano
 */

#include "Actor.h"
#include "Core.h"
#include "CreResource.h"
#include "IDSResource.h"
#include "Object.h"
#include "ResManager.h"
#include "Script.h"

#include <algorithm>

Object::Object(const char* name)
	:
	fName(name),
	fVisible(true),
	fScript(NULL),
	fTicks(0),
	fCurrentScriptRoundResults(NULL),
	fLastScriptRoundResults(NULL)
{
	fCurrentScriptRoundResults = new ScriptResults;
}


Object::~Object()
{
	delete fLastScriptRoundResults;
	delete fCurrentScriptRoundResults;
}


void
Object::Print() const
{
	const Actor* thisActor = dynamic_cast<const Actor*>(this);
	if (thisActor == NULL)
	    return;
	CREResource* cre = thisActor->CRE();
	std::cout << "*** " << thisActor->Name() << " ***" << std::endl;
	std::cout << "Gender: " << IDTable::GenderAt(cre->Gender());
	std::cout << " (" << (int)cre->Gender() << ")" << std::endl;
	std::cout << "Class: " << IDTable::ClassAt(cre->Class());
	std::cout << " (" << (int)cre->Class() << ")" << std::endl;
	std::cout << "Race: " << IDTable::RaceAt(cre->Race());
	std::cout << " (" << (int)cre->Race() << ")" << std::endl;
	std::cout << "EA: " << IDTable::EnemyAllyAt(cre->EnemyAlly());
	std::cout << " (" << (int)cre->EnemyAlly() << ")" << std::endl;
	std::cout << "General: " << IDTable::GeneralAt(cre->General());
	std::cout << " (" << (int)cre->General() << ")" << std::endl;
	std::cout << "Specific: " << IDTable::SpecificAt(cre->Specific());
	std::cout << " (" << (int)cre->Specific() << ")" << std::endl;
	std::cout << "*********" << std::endl;
}


const char*
Object::Name() const
{
	return fName;
}


void
Object::SetVariable(const char* name, int32 value)
{
	fVariables[name] = value;
}


int32
Object::GetVariable(const char* name)
{
	return fVariables[name];
}


bool
Object::IsVisible() const
{
	return fVisible;
}


void
Object::Update(bool scripts)
{
	if (scripts) {
		if (++fTicks == 15) {
			fTicks = 0;
			if (fScript != NULL)
				fScript->Execute();
		}
	}
	Actor* actor = dynamic_cast<Actor*>(this);
	if (actor != NULL) {
		actor->UpdateMove(actor->IsFlying());
	}
}


void
Object::SetScript(Script* script)
{
	fScript = script;
	fScript->SetTarget(this);
}


// Checks if this object matches with the specified object_node.
// Also keeps wildcards in consideration. Used for triggers.
bool
Object::MatchNode(object_node* node) const
{
	if (IsName(node->name)
		&& IsClass(node->classs)
		&& IsRace(node->race)
		&& IsAlignment(node->alignment)
		&& IsGender(node->gender)
		&& IsGeneral(node->general)
		&& IsSpecific(node->specific)
		&& IsEnemyAlly(node->ea))
		return true;

	return false;
}


bool
Object::MatchWithOneInList(const std::vector<Object*>& vec) const
{
	std::vector<Object*>::const_iterator iter;
	for (iter = vec.begin(); iter != vec.end(); iter++) {
		(*iter)->Print();
		if ((*iter)->IsEqual(this))
			return true;
	}

	return false;
}


/* static */
bool
Object::CheckIfNodeInList(object_node* node,
		const std::vector<Object*>& vector)
{
	node->Print();

	std::vector<Object*>::const_iterator iter;
	for (iter = vector.begin(); iter != vector.end(); iter++) {
		if ((*iter)->MatchNode(node))
			return true;
	}
	return false;
}


/* static */
bool
Object::CheckIfNodesInList(const std::vector<object_node*>& nodeList,
		const std::vector<Object*>& vector)
{
	//node->Print();

	std::vector<Object*>::const_iterator iter;
	std::vector<object_node*>::const_iterator nodeIter;
	for (iter = vector.begin(); iter != vector.end(); iter++) {
		for (nodeIter = nodeList.begin(); nodeIter != nodeList.end(); nodeIter++) {
			if ((*iter)->MatchNode(*nodeIter))
				return true;
		}
	}
	return false;
}


bool
Object::IsEqual(const Object* objectB) const
{
	const Actor* actorA = dynamic_cast<const Actor*>(this);
	const Actor* actorB = dynamic_cast<const Actor*>(objectB);
	if (actorA == NULL || actorB == NULL)
		return false;

	CREResource* creA = actorA->CRE();
	CREResource* creB = actorB->CRE();

	if (::strcasecmp(actorA->Name(), actorB->Name()) == 0
		&& (creA->Class() == creB->Class())
		&& (creA->Race() == creB->Race())
		&& (creA->Alignment() == creB->Alignment())
		&& (creA->Gender() == creB->Gender())
		&& (creA->General() == creB->General())
		&& (creA->Specific() == creB->Specific())
		&& (creA->EnemyAlly(), creB->EnemyAlly()))
		return true;
	return false;
}


/* static */
bool
Object::IsDummy(const object_node* node)
{
	if (node->alignment == 0
			&& node->classs == 0
			&& node->ea == 0
			//&& node->faction == 0
			//&& node->team == 0
			&& node->general == 0
			&& node->race == 0
			&& node->gender == 0
			&& node->specific == 0
			&& node->identifiers == 0
			&& node->name[0] == '\0')
		return true;

	return false;
}


Object*
Object::ResolveIdentifier(const int id) const
{
	std::cout << "ResolveIdentifier(" << id << ") = ";
	std::string identifier = IDTable::ObjectAt(id);
	std::cout << identifier << std::endl;
	if (identifier == "MYSELF")
		return const_cast<Object*>(this);
	// TODO: Implement more identifiers
	if (identifier == "NEARESTENEMYOF") {
		return Core::Get()->GetNearestEnemyOf(this);
	}
	return NULL;
}


bool
Object::IsEnemyOf(const Object* object) const
{
	// TODO: Implement correctly
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL) {
		std::cout << "Not an actor" << std::endl;
		return false;
	}

	return actor->IsEnemyOfEveryone();
}


bool
Object::IsName(const char* name) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;
	if (name[0] == '\0' || !strcasecmp(name, actor->Name()))
		return true;
	return false;
}


bool
Object::IsClass(int c) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	CREResource* cre = actor->CRE();
	if (c == 0 || c == cre->Class())
		return true;

	return false;
}


bool
Object::IsRace(int race) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	CREResource* cre = actor->CRE();
	if (race == 0 || race == cre->Race())
		return true;

	return false;
}


bool
Object::IsGender(int gender) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	CREResource* cre = actor->CRE();
	if (gender == 0 || gender == cre->Gender())
		return true;

	return false;
}


bool
Object::IsGeneral(int general) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	CREResource* cre = actor->CRE();
	if (general == 0 || general == cre->General())
		return true;

	return false;
}


bool
Object::IsSpecific(int specific) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	CREResource* cre = actor->CRE();
	if (specific == 0 || specific == cre->Specific())
		return true;

	return false;
}


bool
Object::IsAlignment(int alignment) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	CREResource* cre = actor->CRE();
	if (alignment == 0 || alignment == cre->Alignment())
		return true;

	return false;
}


bool
Object::IsEnemyAlly(int ea) const
{
	const Actor* actor = dynamic_cast<const Actor*>(this);
	if (actor == NULL)
		return false;

	if (ea == 0)
		return true;

	CREResource* cre = actor->CRE();
	if (ea == cre->EnemyAlly())
		return true;

	std::string eaString = IDTable::EnemyAllyAt(ea);

	if (eaString == "PC") {
		// TODO: Should check if Actor is in party
		if (false)
			return true;
	} else if (eaString == "GOODCUTOFF") {
		if (cre->EnemyAlly() <= ea)
			return true;
	} else if (eaString == "EVILCUTOFF") {
		if (cre->EnemyAlly() >= ea)
			return true;
	}

	return false;
}


void
Object::NewScriptRound()
{
	delete fLastScriptRoundResults;
	fLastScriptRoundResults = fCurrentScriptRoundResults;
	fCurrentScriptRoundResults = new ScriptResults;
}


ScriptResults*
Object::CurrentScriptRoundResults() const
{
	return fCurrentScriptRoundResults;
}


ScriptResults*
Object::LastScriptRoundResults() const
{
	return fLastScriptRoundResults;
}


void
Object::Attack(Object* target)
{
	target->fCurrentScriptRoundResults->fAttackers.push_back(this);
}


// ScriptResults
ScriptResults::ScriptResults()
	:
	fShouted(-1)
{
}


const std::vector<Object*>&
ScriptResults::Attackers() const
{
	return fAttackers;
}


const std::vector<Object*>&
ScriptResults::Hitters() const
{
	return fHitters;
}


const
std::vector<Object*>&
ScriptResults::SeenList() const
{
	return fSeen;
}


Object*
ScriptResults::LastAttacker() const
{
	const int32 numAttackers = fAttackers.size();
	if (numAttackers == 0)
		return NULL;
	return fAttackers[fAttackers.size() - 1];
}


int32
ScriptResults::CountAttackers() const
{
	return fAttackers.size();
}


Object*
ScriptResults::AttackerAt(int32 i) const
{
	return fAttackers[i];
}


int
ScriptResults::Shouted() const
{
	return fShouted;
}
