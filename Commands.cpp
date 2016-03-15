#include "Commands.h"
#include "BodyOverride.h"
#include "HeadOverride.h"
#include "AnimationOverride.h"
#include "EquipmentOverride.h"

static bool Cmd_SetBodyAssetOverride_Execute(COMMAND_ARGS)
{
	char AssetPath[kMaxMessageLength];
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractFormatStringArgs(0, AssetPath, paramInfo, arg1, opcodeOffsetPtr,
														scriptObj, eventList, kCommandInfo_SetBodyAssetOverride.numParams,
														&BodyPart, &AssetType, &NPC))
	{
		return true;
	}

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		switch (AssetType)
		{
		case IActorAssetData::kAssetType_Texture:
			*result = ScriptBodyOverrideAgent::TextureOverrides.Add(NPC, BodyPart, AssetPath);
			break;
		case IActorAssetData::kAssetType_Model:
			*result = ScriptBodyOverrideAgent::MeshOverrides.Add(NPC, BodyPart, AssetPath);
			break;
		case IActorAssetData::kAssetType_BodyEGT:
			*result = ScriptBodyOverrideAgent::FaceGenTextureOverrides.Add(NPC, BodyPart, AssetPath);
			break;
		}
	}

	return true;
}

static bool Cmd_GetBodyAssetOverride_Execute(COMMAND_ARGS)
{
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &BodyPart, &AssetType, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		const char* OverridePath = NULL;
		switch (AssetType)
		{
		case IActorAssetData::kAssetType_Texture:
			OverridePath = ScriptBodyOverrideAgent::TextureOverrides.GetOverridePath(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_Model:
			OverridePath = ScriptBodyOverrideAgent::MeshOverrides.GetOverridePath(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_BodyEGT:
			OverridePath = ScriptBodyOverrideAgent::FaceGenTextureOverrides.GetOverridePath(NPC, BodyPart);
			break;
		}

		if (OverridePath)
			Interfaces::kOBSEStringVar->Assign(PASS_COMMAND_ARGS, OverridePath);
		else
			Interfaces::kOBSEStringVar->Assign(PASS_COMMAND_ARGS, "");

		if (OverridePath && IsConsoleOpen())
			Console_Print("Override: %s", OverridePath);
	}

	return true;
}

static bool Cmd_ResetBodyAssetOverride_Execute(COMMAND_ARGS)
{
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &BodyPart, &AssetType, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		switch (AssetType)
		{
		case IActorAssetData::kAssetType_Texture:
			ScriptBodyOverrideAgent::TextureOverrides.Remove(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_Model:
			ScriptBodyOverrideAgent::MeshOverrides.Remove(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_BodyEGT:
			ScriptBodyOverrideAgent::FaceGenTextureOverrides.Remove(NPC, BodyPart);
			break;
		}
	}

	return true;
}

static bool Cmd_GetFaceGenAge_Execute(COMMAND_ARGS)
{
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		*result = InstanceAbstraction::GetNPCFaceGenAge(NPC);

		if (IsConsoleOpen())
			Console_Print("Age: %f", *result);
	}

	return true;
}

static bool Cmd_SetFaceGenAge_Execute(COMMAND_ARGS)
{
	UInt32 Age = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &Age, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		if (Age < 15)
			Age = 15;
		else if (Age > 65)
			Age = 65;

		InstanceAbstraction::SetNPCFaceGenAge(NPC, Age);
	}

	return true;
}

static bool Cmd_SetHeadAssetOverride_Execute(COMMAND_ARGS)
{
	char AssetPath[kMaxMessageLength];
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractFormatStringArgs(0, AssetPath, paramInfo, arg1, opcodeOffsetPtr,
		scriptObj, eventList, kCommandInfo_SetBodyAssetOverride.numParams,
		&BodyPart, &AssetType, &NPC))
	{
		return true;
	}

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		switch (AssetType)
		{
		case IActorAssetData::kAssetType_Texture:
			*result = ScriptHeadOverrideAgent::TextureOverrides.Add(NPC, BodyPart, AssetPath);
			break;
		case IActorAssetData::kAssetType_Model:
			*result = ScriptHeadOverrideAgent::MeshOverrides.Add(NPC, BodyPart, AssetPath);
			break;
		}
	}

	return true;
}

static bool Cmd_GetHeadAssetOverride_Execute(COMMAND_ARGS)
{
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &BodyPart, &AssetType, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		const char* OverridePath = NULL;
		switch (AssetType)
		{
		case IActorAssetData::kAssetType_Texture:
			OverridePath = ScriptHeadOverrideAgent::TextureOverrides.GetOverridePath(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_Model:
			OverridePath = ScriptHeadOverrideAgent::MeshOverrides.GetOverridePath(NPC, BodyPart);
			break;
		}

		if (OverridePath)
			Interfaces::kOBSEStringVar->Assign(PASS_COMMAND_ARGS, OverridePath);
		else
			Interfaces::kOBSEStringVar->Assign(PASS_COMMAND_ARGS, "");

		if (OverridePath && IsConsoleOpen())
			Console_Print("Override: %s", OverridePath);
	}

	return true;
}

static bool Cmd_ResetHeadAssetOverride_Execute(COMMAND_ARGS)
{
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &BodyPart, &AssetType, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		switch (AssetType)
		{
		case IActorAssetData::kAssetType_Texture:
			ScriptHeadOverrideAgent::TextureOverrides.Remove(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_Model:
			ScriptHeadOverrideAgent::MeshOverrides.Remove(NPC, BodyPart);
			break;
		}
	}

	return true;
}

static bool Cmd_RefreshAnimData_Execute(COMMAND_ARGS)
{
	thisCall<void>(0x004E3490, thisObj);

	return true;
}

static bool Cmd_SetAgeTextureOverride_Execute(COMMAND_ARGS)
{
	char TexturePath[kMaxMessageLength];
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractFormatStringArgs(0, TexturePath, paramInfo, arg1, opcodeOffsetPtr,
		scriptObj, eventList, kCommandInfo_SetAgeTextureOverride.numParams,
		&NPC))
	{
		return true;
	}

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
		FaceGenAgeTextureOverrider::Instance.RegisterAgeTextureScriptOverride(NPC, TexturePath);

	return true;
}

static bool Cmd_ResetAgeTextureOverride_Execute(COMMAND_ARGS)
{
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
		FaceGenAgeTextureOverrider::Instance.UnregisterAgeTextureScriptOverride(NPC);

	return true;
}

static bool Cmd_ToggleAnimOverride_Execute(COMMAND_ARGS)
{
	TESNPC* NPC = NULL;
	UInt32 State = 0;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &State, &NPC))
		return true;

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		if (State)
			ActorAnimationOverrider::Instance.RemoveFromBlacklist(NPC);
		else
			ActorAnimationOverrider::Instance.AddToBlacklist(NPC);
	}

	return true;
}

static bool Cmd_RegisterEquipmentOverrideHandler_Execute(COMMAND_ARGS)
{
	TESForm* HandlerScript = NULL;
	TESObjectREFR* Ref = NULL;
	TESNPC* NPC = NULL;
	TESRace* Race = NULL;
	TESForm* EquippedItem = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &HandlerScript, &Ref, &NPC, &Race, &EquippedItem))
		return true;

	*result = ActorEquipmentOverrider::InvalidID;

	if (HandlerScript && HandlerScript->typeID == kFormType_Script)
	{
		Script* Handler = OBLIVION_CAST(HandlerScript, TESForm, Script);
		SME_ASSERT(Handler);

		ActorEquipmentOverrider::OverrideHandlerIdentifierT HandlerID;
		if (ActorEquipmentOverrider::Instance.RegisterHandler(Handler, Ref, NPC, Race, EquippedItem, HandlerID))
			*result = HandlerID;
	}

	return true;
}

static bool Cmd_UnregisterEquipmentOverrideHandler_Execute(COMMAND_ARGS)
{
	ActorEquipmentOverrider::OverrideHandlerIdentifierT HandlerID = ActorEquipmentOverrider::InvalidID;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &HandlerID))
		return true;

	*result = 0;

	if (ActorEquipmentOverrider::Instance.UnregisterHandler(HandlerID))
		*result = 1;

	return true;
}

static ParamInfo kParams_SetBodyAssetOverride[SIZEOF_FMT_STRING_PARAMS + 3] =
{
	FORMAT_STRING_PARAMS,
	{	"body part",	kParamType_Integer,	0	},
	{	"asset type",	kParamType_Integer,	0	},
	{	"npc",			kParamType_NPC,	1	},
};

static ParamInfo kParams_GetBodyAssetOverride[3] =
{
	{	"body part",	kParamType_Integer,	0	},
	{	"asset type",	kParamType_Integer,	0	},
	{	"npc",			kParamType_NPC,	1	},
};

static ParamInfo kParams_GetFaceGenAge[1] =
{
	{	"npc",	kParamType_NPC,	1	},
};

static ParamInfo kParams_SetFaceGenAge[2] =
{
	{	"age",	kParamType_Integer,	0	},
	{	"npc",	kParamType_NPC,	1	},
};

static ParamInfo kParams_SetAgeTextureOverride[SIZEOF_FMT_STRING_PARAMS + 1] =
{
	FORMAT_STRING_PARAMS,
	{	"npc",			kParamType_NPC,	1	},
};

static ParamInfo kParams_ToggleAnimOverride[2] =
{
	{	"state",	kParamType_Integer,	0	},
	{	"npc",		kParamType_NPC,	1	},
};

static ParamInfo kParams_RegisterEquipmentOverrideHandler[5] =
{
	{ "handler script",			kParamType_InventoryObject,	0 },
	{ "filter ref",				kParamType_Actor,	1 },
	{ "filter npc",				kParamType_NPC,	1 },
	{ "filter race",			kParamType_Race,	1 },
	{ "filter equipped item",	kParamType_InventoryObject,	1 },
};

static ParamInfo kParams_UnregisterEquipmentOverrideHandler[1] =
{
	{ "handler id",		kParamType_Integer,	0 },
};

CommandInfo kCommandInfo_SetBodyAssetOverride =
{
	"SetBodyAssetOverride",
	"",
	0,
	"Overrides the NPC's body asset.",
	0,
	SIZEOF_FMT_STRING_PARAMS + 3,
	kParams_SetBodyAssetOverride,

	Cmd_SetBodyAssetOverride_Execute
};

CommandInfo kCommandInfo_GetBodyAssetOverride =
{
	"GetBodyAssetOverride",
	"",
	0,
	"Returns the NPC's body asset override.",
	0,
	3,
	kParams_GetBodyAssetOverride,

	Cmd_GetBodyAssetOverride_Execute
};

CommandInfo kCommandInfo_ResetBodyAssetOverride =
{
	"ResetBodyAssetOverride",
	"",
	0,
	"Removes the NPC's body asset override.",
	0,
	3,
	kParams_GetBodyAssetOverride,

	Cmd_ResetBodyAssetOverride_Execute
};

CommandInfo kCommandInfo_GetFaceGenAge =
{
	"GetFaceGenAge",
	"",
	0,
	"Returns the NPC's FaceGen age.",
	0,
	1,
	kParams_GetFaceGenAge,

	Cmd_GetFaceGenAge_Execute
};

CommandInfo kCommandInfo_SetFaceGenAge =
{
	"SetFaceGenAge",
	"",
	0,
	"Sets the NPC's FaceGen age.",
	0,
	2,
	kParams_SetFaceGenAge,

	Cmd_SetFaceGenAge_Execute
};

CommandInfo kCommandInfo_SetHeadAssetOverride =
{
	"SetHeadAssetOverride",
	"",
	0,
	"Overrides the NPC's head asset.",
	0,
	SIZEOF_FMT_STRING_PARAMS + 3,
	kParams_SetBodyAssetOverride,

	Cmd_SetHeadAssetOverride_Execute
};

CommandInfo kCommandInfo_GetHeadAssetOverride =
{
	"GetHeadAssetOverride",
	"",
	0,
	"Returns the NPC's head asset override.",
	0,
	3,
	kParams_GetBodyAssetOverride,

	Cmd_GetHeadAssetOverride_Execute
};

CommandInfo kCommandInfo_ResetHeadAssetOverride =
{
	"ResetHeadAssetOverride",
	"",
	0,
	"Removes the NPC's head asset override.",
	0,
	3,
	kParams_GetBodyAssetOverride,

	Cmd_ResetHeadAssetOverride_Execute
};

CommandInfo kCommandInfo_RefreshAnimData =
{
	"RefreshAnimData",
	"",
	0,
	"Refreshes the calling reference's animation state.",
	1,
	1,
	kParams_OneOptionalInt,

	Cmd_RefreshAnimData_Execute
};

CommandInfo kCommandInfo_SetAgeTextureOverride =
{
	"SetAgeTextureOverride",
	"",
	0,
	"Overrides the NPC's age texture base path.",
	0,
	SIZEOF_FMT_STRING_PARAMS + 1,
	kParams_SetAgeTextureOverride,

	Cmd_SetAgeTextureOverride_Execute
};

CommandInfo kCommandInfo_ResetAgeTextureOverride =
{
	"ResetAgeTextureOverride",
	"",
	0,
	"Removes the NPC's age texture base path override.",
	0,
	1,
	kParams_GetFaceGenAge,

	Cmd_ResetAgeTextureOverride_Execute
};

CommandInfo kCommandInfo_ToggleAnimOverride =
{
	"ToggleAnimOverride",
	"",
	0,
	"Toggles Blockhead's animation overrides for the NPC.",
	0,
	2,
	kParams_ToggleAnimOverride,

	Cmd_ToggleAnimOverride_Execute
};

CommandInfo kCommandInfo_RegisterEquipmentOverrideHandler =
{
	"RegisterEquipmentOverrideHandler",
	"",
	0,
	"Registers a user-function to handle equipment model overriding.",
	0,
	5,
	kParams_RegisterEquipmentOverrideHandler,

	Cmd_RegisterEquipmentOverrideHandler_Execute
};

CommandInfo kCommandInfo_UnregisterEquipmentOverrideHandler =
{
	"UnregisterEquipmentOverrideHandler",
	"",
	0,
	"Unregisters an exisiting equipment override handler.",
	0,
	1,
	kParams_UnregisterEquipmentOverrideHandler,

	Cmd_UnregisterEquipmentOverrideHandler_Execute
};

void RegisterCommands( const OBSEInterface* obse )
{
	obse->SetOpcodeBase(0x27F0);													// 27F0 - 27FF
	obse->RegisterCommand(&kCommandInfo_SetBodyAssetOverride);
	obse->RegisterTypedCommand(&kCommandInfo_GetBodyAssetOverride, kRetnType_String);
	obse->RegisterCommand(&kCommandInfo_ResetBodyAssetOverride);
	obse->RegisterCommand(&kCommandInfo_GetFaceGenAge);
	obse->RegisterCommand(&kCommandInfo_SetFaceGenAge);
	obse->RegisterCommand(&kCommandInfo_SetHeadAssetOverride);
	obse->RegisterTypedCommand(&kCommandInfo_GetHeadAssetOverride, kRetnType_String);
	obse->RegisterCommand(&kCommandInfo_ResetHeadAssetOverride);
	obse->RegisterCommand(&kCommandInfo_RefreshAnimData);
	obse->RegisterCommand(&kCommandInfo_SetAgeTextureOverride);
	obse->RegisterCommand(&kCommandInfo_ResetAgeTextureOverride);
	obse->RegisterCommand(&kCommandInfo_ToggleAnimOverride);
	obse->RegisterCommand(&kCommandInfo_RegisterEquipmentOverrideHandler);
	obse->RegisterCommand(&kCommandInfo_UnregisterEquipmentOverrideHandler);
}

void RegisterCommandsWithCSE( void )
{
	SME_ASSERT(Interfaces::kCSEConsole && Interfaces::kCSEIntelliSense);

	Interfaces::kCSEConsole->PrintToConsole("Blockhead", "Registering command URLs ...");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("SetBodyAssetOverride", "http://cs.elderscrolls.com/index.php?title=SetBodyAssetOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("GetBodyAssetOverride", "http://cs.elderscrolls.com/index.php?title=GetBodyAssetOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("ResetBodyAssetOverride", "http://cs.elderscrolls.com/index.php?title=ResetBodyAssetOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("GetFaceGenAge", "http://cs.elderscrolls.com/index.php?title=GetFaceGenAge");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("SetFaceGenAge", "http://cs.elderscrolls.com/index.php?title=SetFaceGenAge");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("SetHeadAssetOverride", "http://cs.elderscrolls.com/index.php?title=SetHeadAssetOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("GetHeadAssetOverride", "http://cs.elderscrolls.com/index.php?title=GetHeadAssetOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("ResetHeadAssetOverride", "http://cs.elderscrolls.com/index.php?title=ResetHeadAssetOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("RefreshAnimData", "http://cs.elderscrolls.com/index.php?title=RefreshAnimData");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("SetAgeTextureOverride", "http://cs.elderscrolls.com/index.php?title=SetAgeTextureOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("ResetAgeTextureOverride", "http://cs.elderscrolls.com/index.php?title=ResetAgeTextureOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("ToggleAnimOverride", "http://cs.elderscrolls.com/index.php?title=ToggleAnimOverride");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("RegisterEquipmentOverrideHandler", "http://cs.elderscrolls.com/index.php?title=RegisterEquipmentOverrideHandler");
	Interfaces::kCSEIntelliSense->RegisterCommandURL("UnregisterEquipmentOverrideHandler", "http://cs.elderscrolls.com/index.php?title=UnregisterEquipmentOverrideHandler");
}