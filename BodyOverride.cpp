#include "BodyOverride.h"

ScriptedActorAssetOverrider<ScriptedTextureOverrideData>		ScriptBodyOverrideAgent::TextureOverrides;
ScriptedActorAssetOverrider<ScriptedModelOverrideData>			ScriptBodyOverrideAgent::MeshOverrides;

ActorBodyAssetData::ActorBodyAssetData( UInt32 Type, AssetComponentT Component, TESNPC* Actor, const char* Path ) :
	IActorAssetData(Type, Component, Actor, Path)
{
	;//
}

bool ActorBodyAssetData::IsValid( void ) const
{
	switch (AssetComponent)
	{
	case kBodyPart_UpperBody:
	case kBodyPart_LowerBody:
	case kBodyPart_Hand:
	case kBodyPart_Foot:
	case kBodyPart_Tail:
		return true;
	default:
		return false;
	}
}

const char* ActorBodyAssetData::GetComponentName( void ) const
{
	switch (AssetComponent)
	{
	case kBodyPart_UpperBody:
		return "UpperBody";
	case kBodyPart_LowerBody:
		return "LowerBody";
	case kBodyPart_Hand:
		return "Hand";
	case kBodyPart_Foot:
		return "Foot";
	case kBodyPart_Tail:
		return "Tail";
	default:
		return "Unknown";
	}
}

void ActorBodyAssetData::GetOverrideAgents( OverrideAgentListT& List )
{
	OverrideAgentHandleT Script(new ScriptBodyOverrideAgent(this));
	OverrideAgentHandleT NPC(new PerNPCBodyOverrideAgent(this));
	OverrideAgentHandleT Race(new PerRaceBodyOverrideAgent(this));
	OverrideAgentHandleT Default(new DefaultAssetOverrideAgent(this));

	List.push_back(NPC);
	List.push_back(Script);
	List.push_back(Race);
	List.push_back(Default);
}


ScriptBodyOverrideAgent::ScriptBodyOverrideAgent( IActorAssetData* Data ) :
	IScriptAssetOverrideAgent(Data, NULL)
{
	switch (Data->AssetType)
	{
	case IActorAssetData::kAssetType_Texture:
		OverrideManager = &TextureOverrides;
		break;
	case IActorAssetData::kAssetType_Model:
		OverrideManager = &MeshOverrides;
		break;
	} 

	SME_ASSERT(OverrideManager);
}

bool PerNPCBodyOverrideAgent::GetEnabled( void ) const
{
	switch (Data->AssetType)
	{
	case IActorAssetData::kAssetType_Texture:
		return Settings::kBodyOverrideTexturePerNPC.GetData().i != 0;
	case IActorAssetData::kAssetType_Model:
		return Settings::kBodyOverrideModelPerNPC.GetData().i != 0;
	default:
		return false;
	} 
}

const char* PerNPCBodyOverrideAgent::GetOverrideSourceDirectory( void ) const
{
	return "Characters\\BodyAssetOverrides\\PerNPC";
}

PerNPCBodyOverrideAgent::PerNPCBodyOverrideAgent( IActorAssetData* Data ) :
	IPerNPCAssetOverrideAgent(Data)
{
	;//
}

bool PerRaceBodyOverrideAgent::GetEnabled( void ) const
{
	switch (Data->AssetType)
	{
	case IActorAssetData::kAssetType_Texture:
		return Settings::kBodyOverrideTexturePerRace.GetData().i != 0;
	case IActorAssetData::kAssetType_Model:
		return Settings::kBodyOverrideModelPerRace.GetData().i != 0;
	default:
		return false;
	} 
}

const char* PerRaceBodyOverrideAgent::GetOverrideSourceDirectory( void ) const
{
	return "Characters\\BodyAssetOverrides\\PerRace";
}

PerRaceBodyOverrideAgent::PerRaceBodyOverrideAgent( IActorAssetData* Data ) :
	IPerRaceAssetOverrideAgent(Data)
{
	;//
}


bool PerRaceBodyOverrideAgent::Query( std::string& OutOverridePath )
{
	bool Result = false;

	if (GetEnabled())
	{
		const char* RaceName = InstanceAbstraction::GetFormName(Data->Race);
		const char* PathSuffix = Data->GetComponentName();
		const char* BaseDir = Data->GetRootDirectory();
		const char* GenderPath = NULL;
		if (InstanceAbstraction::GetNPCFemale(Data->Actor))
			GenderPath = "F";
		else
			GenderPath = "M";

		if (RaceName)
		{
			char Buffer[MAX_PATH] = {0};
			FORMAT_STR(Buffer, "%s\\%s\\%s_%s.%s", GetOverrideSourceDirectory(), GenderPath, RaceName, PathSuffix, Data->GetFileExtension());

#ifndef NDEBUG
			_MESSAGE("Checking override path %s for NPC %08X", Buffer, Data->Actor->refID);
#endif // !NDEBUG

			std::string FullPath(BaseDir); FullPath += "\\" + std::string(Buffer);
			if (InstanceAbstraction::FileFinder::GetFileExists(FullPath.c_str()))
			{
				Result = true;
				OutOverridePath = Buffer;
			}
		}
	}

	return Result;
}

_DefineHookHdlr(TESRaceGetBodyTexture, 0x0052D4C9);
_DefineHookHdlr(TESRaceGetBodyModelA, 0x0047ABFA);
_DefineHookHdlr(TESRaceGetBodyModelB, 0x00523934);

void __cdecl SwapRaceBodyTexture(TESRace* Race, UInt8 BodyPart, TESNPC* NPC, InstanceAbstraction::BSString* OutTexPath, const char* Format, const char* OrgTexPath)
{	
	ActorBodyAssetData Data(ActorBodyAssetData::kAssetType_Texture, BodyPart, NPC, OrgTexPath);
	char OverrideTexPath[MAX_PATH] = {0};
	std::string ResultPath;

	ActorAssetOverriderKernel::Instance.ApplyOverride(&Data, ResultPath);
	FORMAT_STR(OverrideTexPath, "Textures\\%s", ResultPath.c_str());

	OutTexPath->Set(OverrideTexPath);
}

#define _hhName		TESRaceGetBodyTexture
_hhBegin()
{
	_hhSetVar(Retn, 0x0052D4CE);
	__asm
	{
		push	[ebp - 0x20]
		push	[ebp - 0x1C]
		push	edi
		call	SwapRaceBodyTexture
		add		esp, 0xC

		jmp		_hhGetVar(Retn)
	}
}

TESModel* __stdcall SwapRaceBodyModel(TESNPC* NPC, TESRace* Race, UInt32 Gender, UInt32 BodyPart)
{
	static TESModel* kModelSwapBuffer[5] = {0};
	if (kModelSwapBuffer[0] == NULL)
	{
		// sketchy, but this just might work
		for (int i = 0; i < 5; i++)
			kModelSwapBuffer[i] = (TESModel*)InstanceAbstraction::TESModel::CreateInstance();
	}

	// get the model thingy
	TESModel* Original = thisCall<TESModel*>(0x0052BE80, Race, Gender, BodyPart);
	TESModel* Swap = NULL;

	if (Original == NULL || Original->nifPath.m_data == NULL)
	{
		// we were never here, m'kay?
		return Original;
	}

	switch (BodyPart)
	{
	case ActorBodyAssetData::kBodyPart_UpperBody:
		Swap = kModelSwapBuffer[0];
		break;
	case ActorBodyAssetData::kBodyPart_LowerBody:
		Swap = kModelSwapBuffer[1];
		break;
	case ActorBodyAssetData::kBodyPart_Hand:
		Swap = kModelSwapBuffer[2];
		break;
	case ActorBodyAssetData::kBodyPart_Foot:
		Swap = kModelSwapBuffer[3];
		break;
	case ActorBodyAssetData::kBodyPart_Tail:
		Swap = kModelSwapBuffer[4];
		break;
	}

	if (Swap)
	{
		ActorBodyAssetData Data(ActorBodyAssetData::kAssetType_Model, BodyPart, NPC, Original->nifPath.m_data);
		char OverrideMeshPath[MAX_PATH] = {0};
		std::string ResultPath;

		ActorAssetOverriderKernel::Instance.ApplyOverride(&Data, ResultPath);
		FORMAT_STR(OverrideMeshPath, "%s", ResultPath.c_str());

		Swap->nifPath.Set(OverrideMeshPath);
	}
	else
		Swap = Original;

	return Swap;
}

#define _hhName		TESRaceGetBodyModelA
_hhBegin()
{
	_hhSetVar(Retn, 0x0047ABFF);
	__asm
	{
		mov		eax, [esp + 0x14]			// muck around with the stack frame, then you're gonna have a BadTime™
		push	ecx
		push	eax
		call	SwapRaceBodyModel
		jmp		_hhGetVar(Retn)
	}
}

#define _hhName		TESRaceGetBodyModelB
_hhBegin()
{
	_hhSetVar(Retn, 0x00523939);
	__asm
	{
		push	ecx
		push	edi
		call	SwapRaceBodyModel
		jmp		_hhGetVar(Retn)
	}
}


void PatchBodyOverride( void )
{
	if (InstanceAbstraction::EditorMode == false)
	{
		_MemHdlr(TESRaceGetBodyTexture).WriteJump();
		_MemHdlr(TESRaceGetBodyModelA).WriteJump();
		_MemHdlr(TESRaceGetBodyModelB).WriteJump();
	}
}

namespace BodyOverride
{
	void FixPlayerBodyModel( void )
	{
		if (*g_thePlayer)
		{
			UInt8 State = (*g_thePlayer)->isThirdPerson;
			(*g_thePlayer)->isThirdPerson = 1;
			(*g_thePlayer)->Update3D();
			(*g_thePlayer)->isThirdPerson = State;
		}
	}

	void HandleLoadGame( bool FixPlayer3D )
	{
		ScriptBodyOverrideAgent::TextureOverrides.Clear();
		ScriptBodyOverrideAgent::MeshOverrides.Clear();

		if (FixPlayer3D)
			FixPlayerBodyModel();
	}
}

