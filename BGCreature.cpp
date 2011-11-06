#include "BGCreature.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream.h>

#include <ByteOrder.h>

#include "FileStream.h"

BGCreature::BGCreature(uint8 *data, uint32 size, uint32 key)
	:
	TResource(data, size, key)
{
	char signature[9];
	ReadAt(0, signature, 8);
	signature[8] = '\0';
	assert(strcmp(signature, CRE_SIGNATURE) == 0);	
}


BGCreature::~BGCreature()
{
}


const uint32
BGCreature::Kit()
{
	uint32 kit;
	ReadAt(0x244, kit);
	return kit;
}


const char *
BGCreature::KitStr()
{
	return KitToStr(Kit());
}


uint32
BGCreature::Experience()
{
	uint32 exp;
	ReadAt(0x18, exp);
	return exp;
}


uint32
BGCreature::ExperienceValue()
{
	uint32 exp;
	ReadAt(0x14, exp);
	return exp;
}


uint32
BGCreature::Gold()
{
	uint32 gold;
	ReadAt(0x1C, gold);
	return gold;
}


void
BGCreature::GetAttributes(BaseAttributes &attributes)
{
	ReadAt(0x238, attributes);
}


const char *
BGCreature::DialogFile()
{
	static char temp[10];
	ReadAt(0x2cc, temp, 8);
	return temp;
}


/*
void
BGCreature::GetCreatureInfo(TStream &file)
{
	char array[9];
	file.Read(array, 8);
	array[8] = '\0';
	cout << array << endl;
	if (!strcmp(array, "CRE V1.0")) {	
		file >> fLongNameOffset;
		printf("Long Name Offset: %ld\n", fLongNameOffset);
		file >> fShortNameOffset;
		printf("Short Name Offset: %ld\n", fShortNameOffset);
		
		file >> fFlags;
		printf("Exportable: %s\n", fFlags & CRE_IS_EXPORTABLE ? "YES" : "NO");	
		
		file.Seek(2, SEEK_CUR);
		
		file >> fExperienceValue;
		printf("Experience value: %ld\n", fExperienceValue);
		file >> fExperience;
		printf("Experience: %ld\n", fExperience);
		file >> fGold;
		printf("Gold: %ld\n", fGold);
		file >> fPermanentFlags;
		printf("Permanent flags: %lX\n", fPermanentFlags);
		file >> fCurrentHP;
		printf("Current HP: %d\n", fCurrentHP);
		file >> fMaxHP;
		printf("Max HP: %d\n", fMaxHP);
		file >> fAnimationID;
		printf("Animation ID: %d\n", fAnimationID);
		
		file.Seek(2, SEEK_CUR); // unknown field
		
		cout << "Color indexes: " << endl;
		file >> fColors.metal;
		cout << "\t" << "metal: " << (int)fColors.metal << endl;
		file >> fColors.minor;
		cout << "\t" << "minor: " << (int)fColors.minor << endl;
		file >> fColors.major;
		cout << "\t" << "major: " << (int)fColors.major << endl;
		file >> fColors.skin;
		cout << "\t" << "skin: " << (int)fColors.skin << endl;
		file >> fColors.leather;
		cout << "\t" << "leather: " << (int)fColors.leather << endl;
		file >> fColors.armor;
		cout << "\t" << "armor: " << (int)fColors.armor << endl;
		file >> fColors.hair;
		cout << "\t" << "hair: " << (int)fColors.hair << endl;
		
		file.Seek(1, SEEK_CUR); //?
		
		file.Read(fRSCSmallPortrait, 8);
		fRSCSmallPortrait[8] = '\0';
		printf("Small portrait resource name: %s\n", fRSCSmallPortrait);
		
		file.Read(fRSCLargePortrait, 8);
		fRSCLargePortrait[8] = '\0';
		printf("Large portrait resource name: %s\n", fRSCLargePortrait);
		
		file >> fReputation;
		printf("Reputation: %d\n", fReputation);
		
		file >> fHideInShadows;
		printf("Hide in shadows: %d\n", fHideInShadows);
		
		printf("Armor class: \n");
		file >> fArmorClass.natural;
		printf("\tNatural: %d\n", fArmorClass.natural);
		file >> fArmorClass.effective;
		printf("\tEffective: %d\n", fArmorClass.effective);
		file >> fArmorClass.crushing;
		printf("\tCrushing: %d\n", fArmorClass.crushing);
		file >> fArmorClass.missile;
		printf("\tMissile: %d\n", fArmorClass.missile);
		file >> fArmorClass.piercing;
		printf("\tPiercing: %d\n", fArmorClass.piercing);
		file >> fArmorClass.slashing;
		printf("\tSlashing: %d\n", fArmorClass.slashing);
		
		file >> fTHAC0;
		printf("THAC0: %d\n", fTHAC0);
		
		file >> fNumberOfAttacks;
		printf("Number of Attacks: %d\n", fNumberOfAttacks);
		
		printf("Save Versus: \n");
		file >> fSaveVersus.death;
		file >> fSaveVersus.wands;
		file >> fSaveVersus.poly;
		file >> fSaveVersus.breath;
		file >> fSaveVersus.spell;
		 
		printf("\tdeath: %d\n", fSaveVersus.death);
		printf("\twands: %d\n", fSaveVersus.wands);
		printf("\tpoly: %d\n", fSaveVersus.poly);
		printf("\tbreath: %d\n", fSaveVersus.breath);
		printf("\tspell: %d\n", fSaveVersus.spell);
		
		printf("Resistances: \n");
		file >> fResistances.fire;
		file >> fResistances.cold;
		file >> fResistances.electricity;
		file >> fResistances.acid;
		file >> fResistances.magic;
		file >> fResistances.magic_fire;
		file >> fResistances.magic_cold;
		file >> fResistances.slashing;
		file >> fResistances.crushing;
		file >> fResistances.piercing;
		file >> fResistances.missile;
		printf("\tfire: %d\n", fResistances.fire);
		printf("\tcold: %d\n", fResistances.cold);
		printf("\telectricity: %d\n", fResistances.electricity);
		printf("\tacid: %d\n", fResistances.acid);
		printf("\tmagic: %d\n", fResistances.magic);
		printf("\tmagic_fire: %d\n", fResistances.magic_fire);
		printf("\tmagic_cold: %d\n", fResistances.magic_cold);
		printf("\tslashing: %d\n", fResistances.slashing);
		printf("\tcrushing: %d\n", fResistances.crushing);
		printf("\tpiercing: %d\n", fResistances.piercing);
		printf("\tmissile: %d\n", fResistances.missile);
		
		file >> fDetectIllusion;
		printf("Detect illusion: %d\n", fDetectIllusion);
		
		file >> fSetTraps;
		cout << "Set traps: " << (int)fSetTraps << endl;
		
		file >> fLore;
		cout << "Lore: " << (int)fLore << endl;
		
		file >> fLockPicking;
		cout << "Lock picking: " << (int)fLockPicking << endl;
		
		file >> fStealth;
		cout << "Stealth: " << (int)fStealth << endl;
		
		file >> fDisarmTraps;
		cout << "Find/Disarm traps: " << (int)fDisarmTraps << endl;
		
		file >> fPickPockets;
		cout << "Pick-pockets: " << (int)fPickPockets << endl;
		
		file >> fFatigue;
		cout << "Fatigue: " << (int)fFatigue << endl;
		
		file >> fIntoxification;
		cout << "Intoxification: " << (int)fIntoxification << endl;
		
		file >> fLuck;
		cout << "Luck: " << (int)fLuck << endl;
		
		file.Seek(0x238 - 0x6e, SEEK_CUR); // TODO: Later
		
		printf("Attributes:\n");
		file >> fAttributes.strength;
		file >> fAttributes.strength_bonus;
		file >> fAttributes.intelligence;
		file >> fAttributes.wisdom;
		file >> fAttributes.dexterity;
		file >> fAttributes.constitution;
		file >> fAttributes.charisma;
		printf("\tstrength: %d", fAttributes.strength);
		if (fAttributes.strength_bonus != 0)
			printf("/%d", fAttributes.strength_bonus);
		printf("\n");
		printf("\tintellicence: %d\n", fAttributes.intelligence);
		printf("\twisdom: %d\n", fAttributes.wisdom);
		printf("\tdexterity: %d\n", fAttributes.dexterity);
		printf("\tconstitution: %d\n", fAttributes.constitution);
		printf("\tcharisma: %d\n", fAttributes.charisma);
		
		file >> fMorale;
		cout << "Morale: " << (int)fMorale << endl;
		
		file >> fMoraleBreak;
		cout << "Morale break: " << (int)fMoraleBreak << endl;
		
		file >> fRacialEnemy;
		cout << "Racial enemy: " << (int)fRacialEnemy << endl;
		
		file >> fMoraleRecoveryTime;
		cout << "Morale recovery time: " << (int)fMoraleRecoveryTime << endl;
 		
 		file.Seek(1, SEEK_CUR);
 		
 		file >> fKit;
 		
 		cout << "Kit: " << KitToStr(fKit) << "(0x" << hex << fKit << ")" << endl;
 		
 		file.Seek(0x2cc - 0x248, SEEK_CUR);
		
		file.Read(fDialogFile, 8);
		fDialogFile[8] = '\0';
		
		printf("Dialog file: %s\n", fDialogFile);
		
	}
}
*/

const char *
KitToStr(uint32 kit)
{
	switch (kit) {
		case KIT_BARBARIAN:
			return "Barbarian";
		case KIT_TRUECLASS:
			return "TrueClass";
		case KIT_BERSERKER:
			return "Berserker";
		case KIT_WIZARDSLAYER:
			return "WizardSlayer";
		case KIT_KENSAI:
			return "Kensai";
		case KIT_CAVALIER:
			return "Cavalier";
		case KIT_INQUISITOR:
			return "Inquisitor";
		case KIT_UNDEADHUNTER:
			return "Undead Hunter";
		case KIT_ARCHER:
			return "Archer";
		case KIT_STALKER:
			return "Stalker";
		case KIT_BEASTMASTER:
			return "Beast Master";
		case KIT_ASSASSIN:
			return "Assassin";
		case KIT_BOUNTYHUNTER:
			return "Bounty Hunter";
		case KIT_SWASHBUCKLER:
			return "SwashBuckler";
		case KIT_BLADE:
			return "Blade";
		case KIT_JESTER:
			return "Jester";
		case KIT_SKALD:
			return "Skald";
		case KIT_TOTEMIC:
			return "Totemic";
		case KIT_SHAPESHIFTER:
			return "Shape Shifter";
		case KIT_AVENGER:
			return "Avenger";
		case KIT_GODTALOS:
			return "Druid of Talos";
		case KIT_GODHELM:
			return "Druid of Helm";
		case KIT_GODLATHANDER:
			return "Druid of Lathander";
		case ABJURER:
			return "Abjurer";
		case CONJURER:
			return "Conjurer";
		case DIVINER:
			return "Diviner";
		case ENCHANTER:
			return "Enchanter";
		case ILLUSIONIST:
			return "Illusionist";
		case INVOKER:
			return "Invoker";
		case NECROMANCER:
			return "Necromancer";
		case TRANSMUTER:
			return "Transmuter";
		case NONE:
		default:
			return "None";
	}
}

