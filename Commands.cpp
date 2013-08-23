#include "Commands.h"
#include "BodyOverride.h"

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
		case IBodyOverrideAgent::kAssetType_Texture:
			*result = ScriptBodyOverrideAgent::ScriptBodyTexOverrides.Add(NPC, BodyPart, TexturePath);
			break;
		case IBodyOverrideAgent::kAssetType_Model:
			*result = ScriptBodyOverrideAgent::ScriptBodyMeshOverrides.Add(NPC, BodyPart, TexturePath);
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
		case IBodyOverrideAgent::kAssetType_Texture:
			OverridePath = ScriptBodyOverrideAgent::ScriptBodyTexOverrides.GetOverridePath(NPC, BodyPart);
			break;
		case IBodyOverrideAgent::kAssetType_Model:
			OverridePath = ScriptBodyOverrideAgent::ScriptBodyMeshOverrides.GetOverridePath(NPC, BodyPart);
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
		case IBodyOverrideAgent::kAssetType_Texture:
			ScriptBodyOverrideAgent::ScriptBodyTexOverrides.Remove(NPC, BodyPart);
			break;
		case IBodyOverrideAgent::kAssetType_Model:
			ScriptBodyOverrideAgent::ScriptBodyMeshOverrides.Remove(NPC, BodyPart);
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

void RegisterCommands( const OBSEInterface* obse )
{
	TODO("Add opcode assignment to the xSE depot")
		
	obse->SetOpcodeBase(0x27F0);													// 27F0 - 27FF
	obse->RegisterCommand(&kCommandInfo_SetBodyAssetOverride);
	obse->RegisterTypedCommand(&kCommandInfo_GetBodyAssetOverride, kRetnType_String);
	obse->RegisterCommand(&kCommandInfo_ResetBodyAssetOverride);
	obse->RegisterCommand(&kCommandInfo_GetFaceGenAge);
	obse->RegisterCommand(&kCommandInfo_SetFaceGenAge);
}
