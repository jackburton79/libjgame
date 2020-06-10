/*
 * AnimationFactory.h
 *
 *  Created on: 20/mag/2013
 *      Author: stefano
 */

#ifndef __ANIMATIONFACTORY_H_
#define __ANIMATIONFACTORY_H_

#include <map>
#include <string>
#include <vector>

#include "IETypes.h"
#include "Referenceable.h"

struct animation_description {
	std::string bam_name;
	int sequence_number;
	bool mirror;
};

struct CREColors;
class Animation;
class AnimationFactory : public Referenceable {
public:
	static AnimationFactory* GetFactory(const uint16 id);
	static void ReleaseFactory(AnimationFactory*);

	Animation* AnimationFor(int action, int orientation, CREColors* colors = NULL);
	
protected:
	AnimationFactory(const char* baseName, const uint16 id);
	virtual ~AnimationFactory();

	virtual void GetAnimationDescription(int action, int orientation, animation_description& description) = 0;
	
	const char* _GetBamName(const char* attributes) const;

	static std::map<uint16, AnimationFactory*> sAnimationFactory;

	std::string fBaseName;
	uint16 fID;
};

#endif /* ANIMATIONFACTORY_H_ */
