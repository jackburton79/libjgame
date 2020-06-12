/*
 * BGAnimationFactory.cpp
 *
 *  Created on: 16/nov/2014
 *      Author: stefano
 */

#include "BGMonsterAnimationFactory.h"

#include "Animation.h"
#include "Core.h"

BGMonsterAnimationFactory::BGMonsterAnimationFactory(const char* baseName, const uint16 id)
	:
	AnimationFactory(baseName, id)
{
}


BGMonsterAnimationFactory::~BGMonsterAnimationFactory()
{
}


/* virtual */
void
BGMonsterAnimationFactory::GetAnimationDescription(int action, int o, animation_description& description)
{
	//std::cout << "BGAnimationFactory" << std::endl;
	description.bam_name = fBaseName;
	description.mirror = false;
	//if (Core::Get()->Game() == GAME_BALDURSGATE2)
	//	o = IE::orientation_ext_to_base(o);
	// Armor
	// TODO: For real
	std::cerr << "orientation: " << std::dec << o << std::endl;
	if (o >= IE::ORIENTATION_EXT_NNE
				&& uint32(o) <= IE::ORIENTATION_EXT_SSE) {
		// Orientation 5 uses bitmap from orientation 3 mirrored,
		// 6 uses 2, and 7 uses 1
		description.mirror = true;
		o = 16 - o;
	}
	switch (action) {
		case ACT_WALKING:
			description.bam_name.append("G11");
			description.sequence_number = o;
			break;
		case ACT_STANDING:
			description.bam_name.append("G1");
			description.sequence_number = o + 10;
			break;
		case ACT_ATTACKING:
			description.bam_name.append("G2");
			description.sequence_number = o;
			break;
		case ACT_CAST_SPELL:
			description.bam_name.append("G25"); // spell build-up
			//description.bam_name.append("G26"); // spell release
			description.sequence_number = o + 44;
			break;
		default:
			std::cerr << "BGMonsterAnimationFactory::GetAnimationDescription(): UNIMPLEMENTED ";
			std::cerr << fBaseName << ", action " << action << ", orientation " << o << std::endl;
			break;
	}
}

