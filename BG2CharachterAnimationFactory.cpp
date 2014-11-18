/*
 * BGAnimationFactory.cpp
 *
 *  Created on: 16/nov/2014
 *      Author: stefano
 */

#include "BG2CharachterAnimationFactory.h"

#include "Animation.h"

BG2CharachterAnimationFactory::BG2CharachterAnimationFactory(const char* baseName)
	:
	AnimationFactory(baseName)
{
}


BG2CharachterAnimationFactory::~BG2CharachterAnimationFactory()
{
}


Animation*
BG2CharachterAnimationFactory::AnimationFor(int action, IE::orientation o)
{
	// Check if animation was already loaded
	std::pair<int, IE::orientation> key = std::make_pair(action, o);
	std::map<std::pair<int, IE::orientation>, Animation*>::const_iterator i;
	i = fAnimations.find(key);
	if (i != fAnimations.end())
		return i->second;

	std::cout << "BG2AnimationFactory::AnimationFor" << std::endl;

	std::cout << "BG2AnimationFor" << std::endl;
	animation_description description;
	description.bam_name = fBaseName;
	description.sequence_number = o;
	description.mirror = false;
	// Armor
	// TODO: For real
	description.bam_name.append("1");
	switch (action) {
		case ACT_WALKING:
		{
			if (_HasAnimation(description.bam_name + "W2")) {
				description.bam_name.append("W2");
				description.sequence_number = uint32(o);
			} else {
				description.bam_name.append("G11");
				description.sequence_number = uint32(o) + 8;
			}

			break;
		}
		case ACT_STANDING:
			description.bam_name.append("G1");
			description.sequence_number = uint32(o) + 8;
			break;
		case ACT_ATTACKING:
			description.bam_name.append("A1");
			description.sequence_number = uint32(o);
			break;
		default:
			break;
	}
	if (uint32(o) >= IE::ORIENTATION_NE && uint32(o) <= IE::ORIENTATION_SE) {
		if (_HasEastBams()) {
			description.bam_name.append("E");
			// TODO: Doesn't work for some animations (IE: ACOW)
			//sequenceNumber -= 1;
		} else {
			// Orientation 5 uses bitmap from orientation 3 mirrored,
			// 6 uses 2, and 7 uses 1
			description.mirror = true;
			description.sequence_number -= (uint32(o) - 4) * 2;
		}
	}

	return InstantiateAnimation(description, key);
}
