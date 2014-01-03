#pragma once

#include "BlockheadInternals.h"

_DeclareMemHdlr(RaceSexMenuPoser, "unrestricted camera movement in the racesex menu");
_DeclareMemHdlr(RaceSexMenuRender, "prevents the camera from being reset every frame");
_DeclareMemHdlr(RaceSexMenuBodyFix, "force-refresh the player's skeleton");
_DeclareMemHdlr(PlayerInventory3DAnimSequenceQueue, "swaps idle anim file paths");

void PatchSundries(void);