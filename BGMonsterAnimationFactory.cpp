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
	if (Core::Get()->Game() == GAME_BALDURSGATE2)
		o = IE::orientation_ext_to_base(o);
	// Armor
	// TODO: For real
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
		default:
			break;
	}
	if (o >= IE::ORIENTATION_NE
				&& uint32(o) <= IE::ORIENTATION_SE) {
			// Orientation 5 uses bitmap from orientation 3 mirrored,
			// 6 uses 2, and 7 uses 1
			description.mirror = true;
			description.sequence_number -= (o - 4) * 2;
		}
}

