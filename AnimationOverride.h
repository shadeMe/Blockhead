#pragma once

#include "BlockheadInternals.h"
#include "AssetOverride.h"

// overrides are implemented as special animations
class ActorAnimationOverrider
{
	typedef std::vector<std::string>			AnimationFileListT;		// filenames relative to the SpecialAnims folder
	typedef std::list<NPCHandleT>				NPCListT;

	static const std::string					kOverrideFileTag;		// used to identify BlockHead specific special anims

	NPCListT									Blacklist;				// overrides will not be applied on blacklisted NPCs

	UInt32										GetSpecialAnims(TESNPC* NPC, const char* Prefix, AnimationFileListT& OutFiles) const;
	void										ClearOverrides(TESNPC* NPC) const;
	void										ApplyOverrides(TESNPC* NPC, AnimationFileListT& Files) const;
	bool										GetBlacklisted(TESNPC* NPC) const;
public:
	ActorAnimationOverrider();
	~ActorAnimationOverrider();

	void										AddToBlacklist(TESNPC* NPC);
	void										RemoveFromBlacklist(TESNPC* NPC);
	void										ClearBlacklist(void);

	void										Override(TESNPC* NPC) const;

	static ActorAnimationOverrider				Instance;
};

_DeclareMemHdlr(TESObjectREFRRefreshAnimData, "applies overrides when the actor's animation data is constructed");

void PatchAnimationOverride(void);

namespace AnimOverride
{
	void HandleLoadGame(void);
}
