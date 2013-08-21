#include "Commands.h"

static bool Cmd_SetBodyTextureOverride_Execute(COMMAND_ARGS)
{
	char TexturePath[kMaxMessageLength];
	UInt32 BodyPart = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractFormatStringArgs(0, TexturePath, paramInfo, arg1, opcodeOffsetPtr,
														scriptObj, eventList, kCommandInfo_SetBodyTextureOverride.numParams,
														&BodyPart, &NPC))
	{
		return true;
	}
	
	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		*result = ScriptedBodyTextureOverrideManager::Instance.Add(NPC, BodyPart, TexturePath);
	}

	return true;
}

static bool Cmd_GetBodyTextureOverride_Execute(COMMAND_ARGS)
{
	UInt32 BodyPart = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &BodyPart, &NPC))
	{
		return true;
	}

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		const char* OverridePath = ScriptedBodyTextureOverrideManager::Instance.GetOverridePath(NPC, BodyPart);
		if (OverridePath)
			Interfaces::kOBSEStringVar->Assign(PASS_COMMAND_ARGS, OverridePath);
		else
			Interfaces::kOBSEStringVar->Assign(PASS_COMMAND_ARGS, "");

		if (OverridePath && IsConsoleOpen())
			Console_Print("Override: %s", OverridePath);
	}

	return true;
}

static bool Cmd_ResetBodyTextureOverride_Execute(COMMAND_ARGS)
{
	UInt32 BodyPart = 0;
	TESNPC* NPC = NULL;

	if (!Interfaces::kOBSEScript->ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &BodyPart, &NPC))
	{
		return true;
	}

	*result = 0;

	if (thisObj && NPC == NULL)
		NPC = OBLIVION_CAST(thisObj->baseForm, TESForm, TESNPC);

	if (NPC)
	{
		ScriptedBodyTextureOverrideManager::Instance.Remove(NPC, BodyPart);
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
	}

	return true;
}

static bool Cmd_SetFaceGenAge_Execute(COMMAND_ARGS)
{
	double Age = 0.0f;
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
		InstanceAbstraction::SetNPCFaceGenAge(NPC, Age);
	}

	return true;
}


static ParamInfo kParams_SetBodyTextureOverride[SIZEOF_FMT_STRING_PARAMS + 2] =
{
	FORMAT_STRING_PARAMS,
	{	"body part",	kParamType_Integer,	0	},
	{	"npc",			kParamType_NPC,	1	},
};

static ParamInfo kParams_GetBodyTextureOverride[2] =
{
	{	"body part",	kParamType_Integer,	0	},
	{	"npc",			kParamType_NPC,	1	},
};

static ParamInfo kParams_GetFaceGenAge[1] =
{
	{	"npc",	kParamType_NPC,	1	},
};

static ParamInfo kParams_SetFaceGenAge[2] =
{
	{	"age",	kParamType_Float,	0	},
	{	"npc",	kParamType_NPC,	1	},
};

CommandInfo kCommandInfo_SetBodyTextureOverride =
{
	"SetBodyTextureOverride",
	"",
	0,
	"Overrides the NPC's body texture.",
	0,
	SIZEOF_FMT_STRING_PARAMS + 2,
	kParams_SetBodyTextureOverride,
	
	Cmd_SetBodyTextureOverride_Execute
};

CommandInfo kCommandInfo_GetBodyTextureOverride =
{
	"GetBodyTextureOverride",
	"",
	0,
	"Returns the NPC's body texture override.",
	0,
	2,
	kParams_GetBodyTextureOverride,

	Cmd_GetBodyTextureOverride_Execute
};

CommandInfo kCommandInfo_ResetBodyTextureOverride =
{
	"ResetBodyTextureOverride",
	"",
	0,
	"Removes the NPC's body texture override.",
	0,
	2,
	kParams_GetBodyTextureOverride,

	Cmd_ResetBodyTextureOverride_Execute
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