/*
 * Referenceable.h
 *
 *  Created on: 12/lug/2012
 *      Author: stefano
 */

#include "SupportDefs.h"

#ifndef __REFERENCEABLE_H
#define __REFERENCEABLE_H

class Referenceable {
public:
	Referenceable(int32 initialRefCount = 1);
	virtual ~Referenceable();

	void Acquire();
	bool Release();

	int32 RefCount() const;

protected:
	virtual void FirstReferenceAcquired();
	virtual void LastReferenceReleased();
	
private:
	int32 fRefCount;
};

#endif /* REFERENCEABLE_H_ */
