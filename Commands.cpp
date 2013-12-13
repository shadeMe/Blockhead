#include "Commands.h"
#include "BodyOverride.h"
#include "HeadOverride.h"

static bool Cmd_SetBodyAssetOverride_Execute(COMMAND_ARGS)
{
	char TexturePath[kMaxMessageLength];
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractFormatStringArgs(0, TexturePath, paramInfo, arg1, opcodeOffsetPtr,
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
			*result = ScriptBodyOverrideAgent::TextureOverrides.Add(NPC, BodyPart, TexturePath);
			break;
		case IActorAssetData::kAssetType_Model:
			*result = ScriptBodyOverrideAgent::MeshOverrides.Add(NPC, BodyPart, TexturePath);
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
	{
		return true;
	}

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
			ScriptBodyOverrideAgent::TextureOverrides.Remove(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_Model:
			ScriptBodyOverrideAgent::MeshOverrides.Remove(NPC, BodyPart);
			break;
		}
	}

	return true;
}


static bool Cmd_GetFaceGenAge_Execute(COMMAND_ARGS)
{
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &NPC))
	{
		return true;
	}

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
	{
		return true;
	}

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
	char TexturePath[kMaxMessageLength];
	UInt32 BodyPart = 0;
	UInt32 AssetType = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractFormatStringArgs(0, TexturePath, paramInfo, arg1, opcodeOffsetPtr,
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
			*result = ScriptHeadOverrideAgent::TextureOverrides.Add(NPC, BodyPart, TexturePath);
			break;
		case IActorAssetData::kAssetType_Model:
			*result = ScriptHeadOverrideAgent::MeshOverrides.Add(NPC, BodyPart, TexturePath);
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
	{
		return true;
	}

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
			ScriptHeadOverrideAgent::TextureOverrides.Remove(NPC, BodyPart);
			break;
		case IActorAssetData::kAssetType_Model:
			ScriptHeadOverrideAgent::MeshOverrides.Remove(NPC, BodyPart);
			break;
		}
	}

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
}
