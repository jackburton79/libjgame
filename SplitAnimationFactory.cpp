/*
 * SplitAnimationFactory.cpp
 *
 *  Created on: 19/nov/2014
 *      Author: stefano
 */

#include "SplitAnimationFactory.h"

#include "Animation.h"

SplitAnimationFactory::SplitAnimationFactory(const char* baseName, const uint16 id)
	:
	AnimationFactory(baseName, id)
{
}


SplitAnimationFactory::~SplitAnimationFactory()
{
}


Animation*
SplitAnimationFactory::AnimationFor(int action, int o)
{
	// Check if animation was already loaded
	// TODO: code duplication, put this into AnimationFactory
	std::pair<int, int> key = std::make_pair(action, o);
	std::map<std::pair<int, int>, Animation*>::const_iterator i;
	i = fAnimations.find(key);
	if (i != fAnimations.end())
		return i->second;

	std::cout << "SplitAnimationFactory::AnimationFor" << std::endl;

	animation_description description;
	description.bam_name = fBaseName;
	description.sequence_number = o;
	description.mirror = false;
	// G1/G11-G15, G2/G21/26
	switch (o) {
		case IE::ORIENTATION_NE:
		case IE::ORIENTATION_NW:
		case IE::ORIENTATION_N:
			description.bam_name.append("H");
			break;
		default:
			description.bam_name.append("L");
			break;
	}

	switch (action) {
		case ACT_WALKING:
			description.bam_name.append("G1");
			description.sequence_number = uint32(o);
			break;
		case ACT_ATTACKING:
			description.bam_name.append("G1");
			description.sequence_number = uint32(o);
			break;
		case ACT_STANDING:
			description.bam_name.append("G1");
			description.sequence_number = uint32(o);
			break;
		default:
			break;
	}
	if (o >= IE::ORIENTATION_NE && uint32(o) <= IE::ORIENTATION_SE) {
		// Orientation 5 uses bitmap from orientation 3 mirrored,
		// 6 uses 2, and 7 uses 1
		description.mirror = true;
		description.sequence_number -= (o - 4) * 2;
	}

	return InstantiateAnimation(description, key);
}
