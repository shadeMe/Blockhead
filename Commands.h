#pragma once

#include "CommandTable.h"
#include "BlockheadInternals.h"

extern CommandInfo kCommandInfo_SetBodyAssetOverride;
extern CommandInfo kCommandInfo_GetBodyAssetOverride;
extern CommandInfo kCommandInfo_ResetBodyAssetOverride;

extern CommandInfo kCommandInfo_SetHeadAssetOverride;
extern CommandInfo kCommandInfo_GetHeadAssetOverride;
extern CommandInfo kCommandInfo_ResetHeadAssetOverride;

extern CommandInfo kCommandInfo_GetFaceGenAge;
extern CommandInfo kCommandInfo_SetFaceGenAge;

extern CommandInfo kCommandInfo_RefreshAnimData;

extern CommandInfo kCommandInfo_SetAgeTextureOverride;
extern CommandInfo kCommandInfo_ResetAgeTextureOverride;

extern CommandInfo kCommandInfo_ToggleAnimOverride;

extern CommandInfo kCommandInfo_RegisterEquipmentOverrideHandler;
extern CommandInfo kCommandInfo_UnregisterEquipmentOverrideHandler;



void RegisterCommands(const OBSEInterface* obse);
void RegisterCommandsWithCSE(void);
