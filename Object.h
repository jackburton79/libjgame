/*
 * Object.h
 *
 *  Created on: 12/lug/2012
 *      Author: stefano
 */

#ifndef __OBJECT_H
#define __OBJECT_H

#include "IETypes.h"
#include "Reference.h"
#include "Referenceable.h"
#include "SupportDefs.h"
#include "Variables.h"

#include <list>
#include <map>
#include <string>
#include <vector>

// TODO: Actor and Object aren't well separated.
// Either merge in one class or improve the separation
struct object_node;
class Action;
class Actor;
class Object;
class Region;
class Script;
class TileCell;
class Action;
class Object : public Referenceable {
public:
	Object(const char* name, const char* scriptName = NULL);

	void Print() const;

	const char* Name() const;
	void SetName(const char* name);

	virtual IE::point Position() const;
	virtual IE::rect Frame() const = 0;

	Variables& Vars();

	virtual void Clicked(Object* clicker);
	virtual void ClickedOn(Object* target);

	void EnteredRegion(Region* region);
	void ExitedRegion(Region* region);

	bool IsVisible() const;
	bool IsInsideVisibleArea() const;

	virtual void Update(bool scripts);

	// TODO: Merge and clean this mess
	void AddAction(Action* action);
	bool IsActionListEmpty() const;
	void ClearActionList();

	void AddScript(::Script* script);
	void ClearScripts();
	
	void SetWaitTime(int32 time);

	virtual IE::point NearestPoint(const IE::point& point) const;
	
	void DestroySelf();
	bool ToBeDestroyed() const;

protected:
	virtual ~Object();
	void LastReferenceReleased();

private:
	void _UpdateTileCell();
	void _ExecuteScripts(int32 maxLevel);

	std::string fName;
	bool fVisible;
	int32 fTicks;
	
	typedef std::vector< ::Script*> ScriptsList;
	ScriptsList fScripts;
	int32 fWaitTime;

	std::list<Action*> fActions;
	::Variables fVariables;

	Region* fRegion;
	
	bool fToDestroy;
};

#endif // __OBJECT_H
