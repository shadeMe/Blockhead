#pragma once

#include "BlockheadInternals.h"

// overrides are implemented as special animations
class ActorAnimationOverrider
{
	typedef std::vector<std::string>			AnimationFileListT;		// filenames relative to the SpecialAnims folder

	static const std::string					kOverrideFilePrefix;	// used to identify BlockHead specific special anims

	static UInt32								GetSpecialAnims(TESNPC* NPC, const char* Prefix, AnimationFileListT& OutFiles);
	static void									ClearOverrides(TESNPC* NPC);
	static void									ApplyOverrides(TESNPC* NPC, AnimationFileListT& Files);
public:
	static void									Override(TESNPC* NPC);
};

_DeclareMemHdlr(TESObjectREFRRefreshAnimData, "running right off the track");

void PatchAnimationOverride(void);
