#ifndef __ANIMATION_H
#define __ANIMATION_H

#include "Frame.h"
#include "IETypes.h"

#include "Referenceable.h"

enum animation_action {
	ACT_WALKING,
	ACT_STANDING
};


enum animation_type {
	ANIMATION_SIMPLE,
	ANIMATION_MONSTER,
	ANIMATION_CHARACTER
};

class Actor;
class BAMResource;
class Animation : public Referenceable {
public:
	Animation(IE::animation *animDesc);
	Animation(CREResource *cre, int action, int sequence, IE::point position);
	~Animation();

	const char* Name() const;

	::Frame Frame();
	void Next();
	::Frame NextFrame();

	IE::point Position() const;

private:
	BAMResource *fBAM;
	IE::point fCenter;
	int16 fCycleNumber;
	int16 fCurrentFrame;
	uint16 fMaxFrame;
	uint16 fType;
	bool fHold;
	bool fBlackAsTransparent;
	bool fMirrored;
	char fName[16];

	res_ref _ClassifyAnimation(uint16 id, uint16& type);
	res_ref _SelectAnimation(CREResource *cre, int action, int face);
};

#endif // __ANIMATION_H
