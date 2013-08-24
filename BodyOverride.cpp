#include "BodyOverride.h"

_DefineHookHdlr(TESRaceGetBodyTexture, 0x0052D4C9);
_DefineHookHdlr(TESRaceGetBodyModelA, 0x0047ABFA);
_DefineHookHdlr(TESRaceGetBodyModelB, 0x00523934);


ScriptedBodyAssetOverrideManager<ScriptedTextureOverrideData>		ScriptBodyOverrideAgent::ScriptBodyTexOverrides;
ScriptedBodyAssetOverrideManager<ScriptedModelOverrideData>			ScriptBodyOverrideAgent::ScriptBodyMeshOverrides;

BodyOverriderKernel									BodyOverriderKernel::Instance;

const char*											PerNPCBodyOverrideAgent::kSourceDirectory = "Characters\\BodyAssetOverrides\\PerNPC";
const char*											PerRaceBodyOverrideAgent::kSourceDirectory = "Characters\\BodyAssetOverrides\\PerRace";


bool IBodyOverrideAgent::operator<( const IBodyOverrideAgent& Second ) const
{
	return this->ID < Second.ID;
}

bool IBodyOverrideAgent::IsDefaultOverride( void ) const
{
	return ID == kID_Default;
}

IBodyOverrideAgent::IBodyOverrideAgent( UInt32 ID /*= kID_Default*/, UInt32 AssetType /*= kAssetType_Invalid*/ ) :
	ID(ID),
	AssetType(AssetType)
{
	SME_ASSERT((ID == kID_Default || AssetType > kAssetType_Invalid) && AssetType < kAssetType__MAX);
}

bool IBodyOverrideAgent::IsValidBodyPart( UInt32 BodyPart )
{
	return (BodyPart == kRaceBodyPart_UpperBody || BodyPart == kRaceBodyPart_LowerBody ||
			BodyPart == kRaceBodyPart_Hand || BodyPart == kRaceBodyPart_Foot || BodyPart == kRaceBodyPart_Tail);
}

const char* IBodyOverrideAgent::GetBodyPartName( UInt32 BodyPart )
{
	SME_ASSERT(IsValidBodyPart(BodyPart));

	switch (BodyPart)
	{
	case kRaceBodyPart_UpperBody:
		return "UpperBody";
	case kRaceBodyPart_LowerBody:
		return "LowerBody";
	case kRaceBodyPart_Hand:
		return "Hand";
	case kRaceBodyPart_Foot:
		return "Foot";
	case kRaceBodyPart_Tail:
	default:
		return "Tail";
	}
}

const char* IBodyOverrideAgent::GetRootDirectory( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return "Textures";
	case kAssetType_Model:
	default:
		return "Meshes";
	}
}

const char* IBodyOverrideAgent::GetFileExtension( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return "dds";
	case kAssetType_Model:
	default:
		return "nif";
	}
}

const char* IBodyOverrideAgent::GetAssetTypeName( UInt32 AssetType )
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return "Texture";
	case kAssetType_Model:
		return "Model";
	default:
		return "<Unknown Asset Type>";
	}
}

bool ScriptedOverrideData::IsEmpty( void ) const
{
	int EmptyCount = 0;

	for (int i = 0; i < kOverridePath__MAX; i++)
	{
		if (OverridePaths[i].empty())
			EmptyCount++;
	}

	return EmptyCount == kOverridePath__MAX;
}

std::string& ScriptedOverrideData::GetOverridePath( UInt32 BodyPart )
{
	SME_ASSERT(IBodyOverrideAgent::IsValidBodyPart(BodyPart));

	switch (BodyPart)
	{
	case kRaceBodyPart_UpperBody:
		return OverridePaths[kOverridePath_UpperBody];
	case kRaceBodyPart_LowerBody:
		return OverridePaths[kOverridePath_LowerBody];
	case kRaceBodyPart_Hand:
		return OverridePaths[kOverridePath_Hand];
	case kRaceBodyPart_Foot:
		return OverridePaths[kOverridePath_Foot];
	default:
		return OverridePaths[kOverridePath_Tail];
	}
}

const std::string& ScriptedOverrideData::GetOverridePath( UInt32 BodyPart ) const
{
	SME_ASSERT(IBodyOverrideAgent::IsValidBodyPart(BodyPart));

	switch (BodyPart)
	{
	case kRaceBodyPart_UpperBody:
		return OverridePaths[kOverridePath_UpperBody];
	case kRaceBodyPart_LowerBody:
		return OverridePaths[kOverridePath_LowerBody];
	case kRaceBodyPart_Hand:
		return OverridePaths[kOverridePath_Hand];
	case kRaceBodyPart_Foot:
		return OverridePaths[kOverridePath_Foot];
	default:
		return OverridePaths[kOverridePath_Tail];
	}
}

bool ScriptedOverrideData::Set( UInt32 BodyPart, const char* Path )
{
	SME_ASSERT(Path && IBodyOverrideAgent::IsValidBodyPart(BodyPart));

	bool Result = false;

	if (strlen(Path) > 2)
	{
		if (VerifyPath(Path))
		{
			Result = true;
			GetOverridePath(BodyPart) = Path;
		}
	}

	return Result;
}

bool ScriptedOverrideData::Remove( UInt32 BodyPart )
{
	SME_ASSERT(IBodyOverrideAgent::IsValidBodyPart(BodyPart));

	GetOverridePath(BodyPart).clear();
	return IsEmpty();
}

const char* ScriptedOverrideData::Get( UInt32 BodyPart ) const
{
	SME_ASSERT(IBodyOverrideAgent::IsValidBodyPart(BodyPart));

	const std::string& Path = GetOverridePath(BodyPart);
	if (Path.empty())
		return NULL;
	else
		return Path.c_str();
}

void ScriptedOverrideData::Clear( void )
{
	for (int i = 0; i < kOverridePath__MAX; i++)
	{
		OverridePaths[i].clear();
	}
}

ScriptedTextureOverrideData::~ScriptedTextureOverrideData()
{
	;//
}

bool ScriptedTextureOverrideData::VerifyPath( const char* Path ) const
{
	std::string RelPath = "Textures\\" + std::string(Path);
	return InstanceAbstraction::FileFinder::GetFileExists(RelPath.c_str());
}

ScriptedModelOverrideData::~ScriptedModelOverrideData()
{
	;//
}

bool ScriptedModelOverrideData::VerifyPath( const char* Path ) const
{
	std::string RelPath = "Meshes\\" + std::string(Path);
	return InstanceAbstraction::FileFinder::GetFileExists(RelPath.c_str());
}


ScriptBodyOverrideAgent::ScriptBodyOverrideAgent( UInt32 AssetType ) :
	IBodyOverrideAgent(kID_Script, AssetType)
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		OverrideManager = &ScriptBodyTexOverrides;
		break;
	case kAssetType_Model:
		OverrideManager = &ScriptBodyMeshOverrides;
		break;
	}
}

ScriptBodyOverrideAgent::~ScriptBodyOverrideAgent()
{
	;//
}

bool ScriptBodyOverrideAgent::Query( TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath )
{
	bool Result = false;

	const char* OverridePath = OverrideManager->GetOverridePath(NPC, BodyPart);
	if (OverridePath)
	{
		Result = true;
		OutOverridePath = OverridePath;
	}

	return Result;
}

PerNPCBodyOverrideAgent::PerNPCBodyOverrideAgent( UInt32 AssetType ) :
	IBodyOverrideAgent(kID_NPC, AssetType)
{
	;//
}

PerNPCBodyOverrideAgent::~PerNPCBodyOverrideAgent()
{
	;//
}

bool PerNPCBodyOverrideAgent::Query( TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath )
{
	bool Result = false;
	
	if (GetEnabled())
	{
		UInt32 FormID = NPC->refID & 0x00FFFFFF;
		ModEntry::Data* Plugin = thisCall<ModEntry::Data*>(0x0046B680, NPC, 0);

		const char* PathSuffix = GetBodyPartName(BodyPart);
		const char* BaseDir = GetRootDirectory();

		if (Plugin)
		{
			char Buffer[MAX_PATH] = {0};
			FORMAT_STR(Buffer, "%s\\%s\\%08X_%s.%s", kSourceDirectory, Plugin->name, FormID, PathSuffix, GetFileExtension());

#ifndef NDEBUG
			_MESSAGE("Checking override path %s for NPC %08X", Buffer, NPC->refID);
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

bool PerNPCBodyOverrideAgent::GetEnabled( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return Settings::kOverrideTexturePerNPC.GetData().i != 0;
	case kAssetType_Model:
		return Settings::kOverrideModelPerNPC.GetData().i != 0;
	default:
		return false;
	} 
}

PerRaceBodyOverrideAgent::PerRaceBodyOverrideAgent( UInt32 AssetType ) :
	IBodyOverrideAgent(kID_Race, AssetType)
{
	;//
}

PerRaceBodyOverrideAgent::~PerRaceBodyOverrideAgent()
{
	;//
}

bool PerRaceBodyOverrideAgent::Query( TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath )
{
	bool Result = false;

	if (GetEnabled())
	{
		const char* RaceName = Race->GetFullName()->name.m_data;
		const char* PathSuffix = GetBodyPartName(BodyPart);
		const char* BaseDir = GetRootDirectory();
		const char* GenderPath = NULL;
		if (NPC->actorBaseData.IsFemale())
			GenderPath = "F";
		else
			GenderPath = "M";

		if (RaceName)
		{
			char Buffer[MAX_PATH] = {0};
			FORMAT_STR(Buffer, "%s\\%s\\%s_%s.%s", kSourceDirectory, GenderPath, RaceName, PathSuffix, GetFileExtension());

#ifndef NDEBUG
			_MESSAGE("Checking override path %s for NPC %08X", Buffer, NPC->refID);
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

bool PerRaceBodyOverrideAgent::GetEnabled( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return Settings::kOverrideTexturePerRace.GetData().i != 0;
	case kAssetType_Model:
		return Settings::kOverrideModelPerRace.GetData().i != 0;
	default:
		return false;
	} 
}


DefaultBodyOverrideAgent::DefaultBodyOverrideAgent() :
	IBodyOverrideAgent()
{
	;//
}

DefaultBodyOverrideAgent::~DefaultBodyOverrideAgent()
{
	;//
}

bool DefaultBodyOverrideAgent::Query( TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath )
{
	if (OriginalPath)
	{
		OutOverridePath = OriginalPath;
		return true;
	}
	else
		return false;
}


bool BodyOverriderKernel::SortComparator( OverrideAgentHandleT First, OverrideAgentHandleT Second )
{
	return (*First) < (*Second);
}

void BodyOverriderKernel::PrepareStack( UInt32 AssetType )
{
	ResetStack();
	
	OverrideAgentHandleT Script(new ScriptBodyOverrideAgent(AssetType));
	OverrideAgentHandleT NPC(new PerNPCBodyOverrideAgent(AssetType));
	OverrideAgentHandleT Race(new PerRaceBodyOverrideAgent(AssetType));
	OverrideAgentHandleT Default(new DefaultBodyOverrideAgent());

	AgentStack.push_back(NPC);
	AgentStack.push_back(Script);
	AgentStack.push_back(Race);
	AgentStack.push_back(Default);

	AgentStack.sort(SortComparator);
}

void BodyOverriderKernel::ResetStack( void )
{
	AgentStack.clear();
}

void BodyOverriderKernel::DumpStack( void ) const
{
#ifndef NDEBUG
	std::string Buffer("Agent Stack : ");

	for (OverrideAgentListT::const_iterator Itr = AgentStack.begin(); Itr != AgentStack.end(); Itr++)
	{
		char Temp[0x50] = {0};
		FORMAT_STR(Temp, "%d > ", (*Itr)->ID);

		Buffer += Temp;
	}

	Buffer += "NULL";
	_MESSAGE("%s", Buffer.c_str());
#endif // !NDEBUG
}

BodyOverriderKernel::BodyOverriderKernel() :
	AgentStack()
{
	;//
}

BodyOverriderKernel::~BodyOverriderKernel()
{
	ResetStack();
}

std::string BodyOverriderKernel::ApplyOverride( UInt32 AssetType, UInt32 BodyPart, TESNPC* NPC, TESRace* Race, const char* OriginalPath )
{
	SME_ASSERT(AssetType && NPC && Race);

	std::string Result = "";

	if (IBodyOverrideAgent::IsValidBodyPart(BodyPart))
	{
#ifndef NDEBUG
		_MESSAGE("Attempting to apply %s override for NPC %08X %s...", IBodyOverrideAgent::GetAssetTypeName(AssetType), NPC->refID, IBodyOverrideAgent::GetBodyPartName(BodyPart));
		gLog.Indent();
#endif // !NDEBUG

		
		PrepareStack(AssetType);

		bool Overridden = false;
		for (OverrideAgentListT::iterator Itr = AgentStack.begin(); Itr != AgentStack.end(); Itr++)
		{
			if ((*Itr)->Query(NPC, Race, BodyPart, OriginalPath, Result))
			{
				// not strictly true, given the default override agent
				Overridden = true;				
#ifndef NDEBUG
				if ((*Itr)->IsDefaultOverride() == false)
					_MESSAGE("Override applied - New path: %s", Result.c_str());
#endif // !NDEBUG

				break;
			}
		}

#ifndef NDEBUG
		gLog.Outdent();
#endif // !NDEBUG

		ResetStack();
	}

	return Result;
}


void __cdecl SwapRaceBodyTexture(TESRace* Race, UInt8 BodyPart, TESNPC* NPC, InstanceAbstraction::BSString* OutTexPath, const char* Format, const char* OrgTexPath)
{	
	char OverrideTexPath[MAX_PATH] = {0};
	FORMAT_STR(OverrideTexPath, "Textures\\%s",
			BodyOverriderKernel::Instance.ApplyOverride(IBodyOverrideAgent::kAssetType_Texture, BodyPart, NPC, Race, OrgTexPath).c_str());

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
	static TESModel*								kModelSwapBuffer[5] = {0};
	if (kModelSwapBuffer[0] == NULL)
	{
		// sketchy, but this just might work
		for (int i = 0; i < 5; i++)
			kModelSwapBuffer[i] = (TESModel*)InstanceAbstraction::TESModel::CreateInstance();
	}

	// get the model thingy
	TESModel* Original = thisCall<TESModel*>(0x0052BE80, Race, Gender, BodyPart);
	TESModel* Swap = NULL;

	switch (BodyPart)
	{
	case kRaceBodyPart_UpperBody:
		Swap = kModelSwapBuffer[0];
		break;
	case kRaceBodyPart_LowerBody:
		Swap = kModelSwapBuffer[1];
		break;
	case kRaceBodyPart_Hand:
		Swap = kModelSwapBuffer[2];
		break;
	case kRaceBodyPart_Foot:
		Swap = kModelSwapBuffer[3];
		break;
	case kRaceBodyPart_Tail:
		Swap = kModelSwapBuffer[4];
		break;
	}

	if (Swap)
	{
		char OverrideTexPath[MAX_PATH] = {0};
		FORMAT_STR(OverrideTexPath, "%s", 
			BodyOverriderKernel::Instance.ApplyOverride(IBodyOverrideAgent::kAssetType_Model, BodyPart, NPC, Race,
			(Original ? Original->nifPath.m_data : NULL)).c_str());

		if (strlen(OverrideTexPath))
			Swap->nifPath.Set(OverrideTexPath);
		else
			Swap = Original;		// will be an empty string when Original == NULL and there are no active overrides
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
