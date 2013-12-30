#include "HeadOverride.h"

ScriptedActorAssetOverrider<ScriptedTextureOverrideData>		ScriptHeadOverrideAgent::TextureOverrides;
ScriptedActorAssetOverrider<ScriptedModelOverrideData>			ScriptHeadOverrideAgent::MeshOverrides;
FaceGenOverrideAgeSwapper										FaceGenOverrideAgeSwapper::Instance;

ActorHeadAssetData::ActorHeadAssetData( UInt32 Type, AssetComponentT Component, TESNPC* Actor, const char* Path ) :
	IActorAssetData(Type, Component, Actor, Path)
{
	;//
}

bool ActorHeadAssetData::IsValid( void ) const
{
	switch (AssetComponent)
	{
	case FaceGenHeadParameters::kFaceGenData_Head:
	case FaceGenHeadParameters::kFaceGenData_EarsMale:
	case FaceGenHeadParameters::kFaceGenData_EarsFemale:
	case FaceGenHeadParameters::kFaceGenData_Mouth:
	case FaceGenHeadParameters::kFaceGenData_TeethLower:
	case FaceGenHeadParameters::kFaceGenData_TeethUpper:
	case FaceGenHeadParameters::kFaceGenData_Tongue:
		return true;
	case FaceGenHeadParameters::kFaceGenData_EyesLeft:
	case FaceGenHeadParameters::kFaceGenData_EyesRight:
		if (AssetType == kAssetType_Texture)				// eyes don't have corresponding textures, the TESEyes class takes care of that
			return false;
		else
			return true;
	default:
		return false;
	}
}

const char* ActorHeadAssetData::GetComponentName( void ) const
{
	switch (AssetComponent)
	{
	case FaceGenHeadParameters::kFaceGenData_Head:
		return "Head";
	case FaceGenHeadParameters::kFaceGenData_EarsMale:
		return "EarsMale";
	case FaceGenHeadParameters::kFaceGenData_EarsFemale:
		return "EarsFemale";
	case FaceGenHeadParameters::kFaceGenData_Mouth:
		return "Mouth";
	case FaceGenHeadParameters::kFaceGenData_TeethLower:
		return "TeethLower";
	case FaceGenHeadParameters::kFaceGenData_TeethUpper:
		return "TeethUpper";
	case FaceGenHeadParameters::kFaceGenData_Tongue:
		return "Tongue";
	case FaceGenHeadParameters::kFaceGenData_EyesLeft:
		return "EyesLeft";
	case FaceGenHeadParameters::kFaceGenData_EyesRight:
		return "EyesRight";
	default:
		return "Unknown";
	}
}

void ActorHeadAssetData::GetOverrideAgents( OverrideAgentListT& List )
{
	OverrideAgentHandleT Script(new ScriptHeadOverrideAgent(this));
	OverrideAgentHandleT NPC(new PerNPCHeadOverrideAgent(this));
	OverrideAgentHandleT Race(new PerRaceHeadOverrideAgent(this));
	OverrideAgentHandleT Default(new DefaultAssetOverrideAgent(this));

	List.push_back(NPC);
	List.push_back(Script);
	List.push_back(Race);
	List.push_back(Default);
}


ScriptHeadOverrideAgent::ScriptHeadOverrideAgent( IActorAssetData* Data ) :
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

bool PerNPCHeadOverrideAgent::GetEnabled( void ) const
{
	switch (Data->AssetType)
	{
	case IActorAssetData::kAssetType_Texture:
		return Settings::kHeadOverrideTexturePerNPC.GetData().i != 0;
	case IActorAssetData::kAssetType_Model:
		return Settings::kHeadOverrideModelPerNPC.GetData().i != 0;
	default:
		return false;
	} 
}

const char* PerNPCHeadOverrideAgent::GetOverrideSourceDirectory( void ) const
{
	return "Characters\\HeadAssetOverrides\\PerNPC";
}

PerNPCHeadOverrideAgent::PerNPCHeadOverrideAgent( IActorAssetData* Data ) :
	IPerNPCAssetOverrideAgent(Data)
{
	;//
}

bool PerRaceHeadOverrideAgent::GetEnabled( void ) const
{
	switch (Data->AssetType)
	{
	case IActorAssetData::kAssetType_Texture:
		return Settings::kHeadOverrideTexturePerRace.GetData().i != 0;
	case IActorAssetData::kAssetType_Model:
		return Settings::kHeadOverrideModelPerRace.GetData().i != 0;
	default:
		return false;
	} 
}

const char* PerRaceHeadOverrideAgent::GetOverrideSourceDirectory( void ) const
{
	// left unimplemented as we keep to the original path
	SME_ASSERT(1 == 0);

	return NULL;
}

PerRaceHeadOverrideAgent::PerRaceHeadOverrideAgent( IActorAssetData* Data ) :
	IPerRaceAssetOverrideAgent(Data)
{
	;//
}


bool PerRaceHeadOverrideAgent::GetComponentGenderVariant( void ) const
{
	switch (Data->AssetComponent)
	{
	case FaceGenHeadParameters::kFaceGenData_EarsMale:
	case FaceGenHeadParameters::kFaceGenData_EarsFemale:
		return true;
	default:
		return false;
	}
}

bool PerRaceHeadOverrideAgent::Query( std::string& OutOverridePath )
{
	bool Result = false;

	if (GetEnabled())
	{
		// per-race handling is mostly unnecessary for head parts
		// we only handle those that are in need of gender variance
		if (GetComponentGenderVariant() == false && Data->AssetPath)
		{
			std::string OriginalPath(Data->AssetPath);

			const char* PathSuffix = Data->GetComponentName();
			const char* BaseDir = Data->GetRootDirectory();
			const char* GenderPath = NULL;
			if (InstanceAbstraction::GetNPCFemale(Data->Actor))
				GenderPath = "F";
			else
				GenderPath = "M";

			// remove extension
			OriginalPath.erase(OriginalPath.length() - 4, 4);

			char Buffer[MAX_PATH] = {0};
			FORMAT_STR(Buffer, "%s\\%s_%s.%s", BaseDir, OriginalPath.c_str(), GenderPath, Data->GetFileExtension());

#ifndef NDEBUG
			_MESSAGE("Checking override path %s for NPC %08X", Buffer, Data->Actor->refID);
#endif // !NDEBUG

			if (InstanceAbstraction::FileFinder::GetFileExists(Buffer))
			{
				Result = true;
				FORMAT_STR(Buffer, "%s_%s.%s", OriginalPath.c_str(), GenderPath, Data->GetFileExtension());
				OutOverridePath = Buffer;
			}
		}
	}

	return Result;
}

FaceGenOverrideAgeSwapper::FaceGenOverrideAgeSwapper() :
	OverriddenHeadTextures(),
	Lock()
{
	;//
}

FaceGenOverrideAgeSwapper::~FaceGenOverrideAgeSwapper()
{
	OverriddenHeadTextures.clear();
}

void FaceGenOverrideAgeSwapper::RegisterOverride( Texture Duplicate, Texture Original )
{
	ScopedLock Guard(Lock);

	SME_ASSERT(Duplicate && Original);
	SME_ASSERT(OverriddenHeadTextures.count(Duplicate) == 0); 
	OverriddenHeadTextures[Duplicate] = Original;
}

void FaceGenOverrideAgeSwapper::UnregisterOverride( Texture Duplicate )
{
	ScopedLock Guard(Lock);

	if (OverriddenHeadTextures.count(Duplicate))
		OverriddenHeadTextures.erase(Duplicate);
}

const char* FaceGenOverrideAgeSwapper::LookupOriginalPath( Texture Duplicate ) const
{
	ScopedLock Guard(Lock);

	const char* Result = NULL;
	if (OverriddenHeadTextures.count(Duplicate))
		Result = InstanceAbstraction::TESTexture::GetPath(OverriddenHeadTextures.at(Duplicate))->m_data;

	return Result;
}


void SwapFaceGenHeadData(TESRace* Race, FaceGenHeadParameters* FaceGenParams, TESNPC* NPC, bool FixingFaceNormals)
{
	// swap the head model/texture pointer with a newly allocated one
	// to allow for the changing of the asset paths	
#ifndef NDEBUG
	if (NPC)
	{
		if (NPC->refID == 0x7)
			_MESSAGE("Generating FaceGen head for the player character...");
		else
			_MESSAGE("Generating FaceGen head for NPC %08X...", NPC->refID);
	}

	gLog.Indent();
	if (FixingFaceNormals)
		_MESSAGE("Fixing FaceGen normals...");
#endif

	for (int i = FaceGenHeadParameters::kFaceGenData__BEGIN; i < FaceGenHeadParameters::kFaceGenData__END; i++)
	{
		InstanceAbstraction::TESModel::Instance OrgModel = (InstanceAbstraction::TESModel::Instance)
																FaceGenParams->models.data[i];
		InstanceAbstraction::TESTexture::Instance OrgTexture = (InstanceAbstraction::TESTexture::Instance)
																	FaceGenParams->textures.data[i];

		bool NonExtantModel = (OrgModel == NULL), NonExtantTexture = (OrgTexture == NULL);

		const char* OrgModelPath = NULL;
		InstanceAbstraction::TESModel::Instance NewModel = NULL;
		if (NonExtantModel == false)
		{
			// body part's model component is already allocated, so business as usual
			NewModel = InstanceAbstraction::TESModel::CreateInstance();
			OrgModelPath = InstanceAbstraction::TESModel::GetPath(OrgModel)->m_data;
			InstanceAbstraction::TESModel::GetPath(NewModel)->Set(OrgModelPath);
		}

		// NPC will NULL when generating heads in the editor's Race edit dialog
		if (NPC)
		{
			ActorHeadAssetData Data(ActorHeadAssetData::kAssetType_Model, i, NPC, OrgModelPath);
			char OverridePath[MAX_PATH] = {0};
			std::string ResultPath;

			bool ReplacePointer = true;
			bool OverrideOp = ActorAssetOverriderKernel::Instance.ApplyOverride(&Data, ResultPath);
			if (NonExtantModel)
			{
				if (OverrideOp == false)
				{
					// no overrides for nonextant part, don't bother replacing the pointer
					ReplacePointer = false;
				}
				else
				{
					// we've got an override, allocate a new pointer
					NewModel = InstanceAbstraction::TESModel::CreateInstance();
				}
			}

			FORMAT_STR(OverridePath, "%s", ResultPath.c_str());

			if (ReplacePointer)
			{
				SME_ASSERT(NewModel);
				InstanceAbstraction::TESModel::GetPath(NewModel)->Set(OverridePath);

				// finally swap the pointers, which will be released in the subsequent call to the facegen parameter object's dtor
				FaceGenParams->models.data[i] = (::TESModel*)NewModel;

				// eyes need special casing because Bethesda
				if (i == FaceGenHeadParameters::kFaceGenData_EyesLeft)
					FaceGenParams->eyeLeft = (::TESModel*)NewModel;
				else if (i == FaceGenHeadParameters::kFaceGenData_EyesRight)
					FaceGenParams->eyeRight = (::TESModel*)NewModel;
			}
		}
		
		// the same for the texture component
		const char* OrgTexturePath = NULL;
		InstanceAbstraction::TESTexture::Instance NewTexture = NULL;
		if (NonExtantTexture == false)
		{
			NewTexture = InstanceAbstraction::TESTexture::CreateInstance();
			OrgTexturePath = InstanceAbstraction::TESTexture::GetPath(OrgTexture)->m_data;
			InstanceAbstraction::TESTexture::GetPath(NewTexture)->Set(OrgTexturePath);
		}

		if (NPC)
		{
			ActorHeadAssetData Data(ActorHeadAssetData::kAssetType_Texture, i, NPC, OrgTexturePath);
			char OverridePath[MAX_PATH] = {0};
			std::string ResultPath;

			bool ReplacePointer = true;
			bool OverrideOp = ActorAssetOverriderKernel::Instance.ApplyOverride(&Data, ResultPath);
			if (NonExtantModel)
			{
				if (OverrideOp == false)
				{
					ReplacePointer = false;
				}
				else
				{
					NewTexture = InstanceAbstraction::TESTexture::CreateInstance();
				}
			}

			FORMAT_STR(OverridePath, "%s", ResultPath.c_str());

			if (ReplacePointer)
			{
				SME_ASSERT(NewTexture);

				// save the original TESTexture pointer of kFaceGenData_Head when an override is active
				// we check it later to fixup the age overlay texture paths
				if (i == FaceGenHeadParameters::kFaceGenData_Head && OverrideOp)
				{
					FaceGenOverrideAgeSwapper::Instance.RegisterOverride(NewTexture, OrgTexture);
				}

				InstanceAbstraction::TESTexture::GetPath(NewTexture)->Set(OverridePath);
				FaceGenParams->textures.data[i] = (::TESTexture*)NewTexture;
			}
		}
	}	

#ifndef NDEBUG
	gLog.Outdent();
#endif
}


void __stdcall DoTESRaceGetFaceGenHeadParametersHook(TESRace* Race, FaceGenHeadParameters* FaceGenParams, TESNPC* NPC)
{
	// call original function to get the parameters
	thisCall<void>(InstanceAbstraction::kTESRace_GetFaceGenHeadParameters(), Race, NPC, FaceGenParams);

	SwapFaceGenHeadData(Race, FaceGenParams, NPC, false);
}

void __declspec(naked) TESRaceGetFaceGenHeadParametersHook(void)
{
	__asm
	{
		push	[esp + 0x4]
		push	[esp + 0xC]
		push	ecx
		call	DoTESRaceGetFaceGenHeadParametersHook
		retn	0x8
	}
}

void __stdcall DoFaceGenHeadParametersDtorHook(FaceGenHeadParameters* FaceGenParams)
{
	for (int i = FaceGenHeadParameters::kFaceGenData__BEGIN; i < FaceGenHeadParameters::kFaceGenData__END; i++)
	{
		if (i < FaceGenParams->models.numObjs)
		{
			InstanceAbstraction::TESModel::Instance SneakyBugger = (InstanceAbstraction::TESModel::Instance)
																FaceGenParams->models.data[i];
			
			// emancipate the bugger, unto death
			if (SneakyBugger)
				InstanceAbstraction::TESModel::DeleteInstance(SneakyBugger);
		}

		if (i < FaceGenParams->textures.numObjs)
		{
			InstanceAbstraction::TESTexture::Instance SneakyBugger = (InstanceAbstraction::TESTexture::Instance)
																FaceGenParams->textures.data[i];

			if (SneakyBugger)
			{
				// remove the cached override data
				FaceGenOverrideAgeSwapper::Instance.UnregisterOverride(SneakyBugger);
				InstanceAbstraction::TESTexture::DeleteInstance(SneakyBugger);
			}
		}
	}	

	thisCall<void>(InstanceAbstraction::kFaceGenHeadParameters_Dtor(), FaceGenParams);
}

void __declspec(naked) FaceGenHeadParametersDtorHook(void)
{
	__asm
	{
		push	ecx
		call	DoFaceGenHeadParametersDtorHook
		retn
	}
}

void __cdecl DoBSFaceGenDoSomethingWithFaceGenNodeHook(TESNPC* NPC, NiNode* FaceGenNode, FaceGenHeadParameters* HeadParams)
{
	SwapFaceGenHeadData(InstanceAbstraction::GetNPCRace(NPC), HeadParams, NPC, true);

	cdeclCall<void>(InstanceAbstraction::kBSFaceGen_DoSomethingWithFaceGenNode(), FaceGenNode, HeadParams);
}

static UInt32		kBSFaceGenDoSomethingWithFaceGenNodeRetnAddr = 0;

void __declspec(naked) BSFaceGenDoSomethingWithFaceGenNodeHook(void)
{
	__asm
	{
		push	ebx
		call	DoBSFaceGenDoSomethingWithFaceGenNodeHook
		add		esp, 0x4				// account for the extra arg
		jmp		kBSFaceGenDoSomethingWithFaceGenNodeRetnAddr
	}
}

const char* __stdcall DoBSFaceGetAgeTexturePathHook(FaceGenHeadParameters* HeadParams,
													InstanceAbstraction::BSString* OutPath,
													UInt32 Gender,
													UInt32 Age,
													const char* BasePath)
{
	static const InstanceAbstraction::MemAddr kCallAddr = { 0x00551A00, 0x005845F0 };

	SME_ASSERT(HeadParams && HeadParams->textures.numObjs);

	InstanceAbstraction::TESTexture::Instance OverriddenTexture = (InstanceAbstraction::TESTexture::Instance)
																HeadParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head];

	SME_ASSERT(OverriddenTexture);
	
	const char* OriginalPath = FaceGenOverrideAgeSwapper::Instance.LookupOriginalPath(OverriddenTexture);
	if (OriginalPath)
	{
		// the base head texture path has been overridden by us, use the original base path instead
		// age textures are already gender variant so revert out changes		
#ifndef NDEBUG
		_MESSAGE("Reset head asset path %s to %s for age texture generation [G=%d, A=%d]", BasePath, OriginalPath, Gender, Age);
#endif
		BasePath = OriginalPath;
	}

	return cdeclCall<const char*>(kCallAddr(), OutPath, Gender, Age, BasePath);
}

static UInt32		kBSFaceGetAgeTexturePathRetnAddr = 0;

#define _hhName		BSFaceGetAgeTexturePath
_hhBegin()
{
	__asm
	{
		test	InstanceAbstraction::EditorMode, 1
		jnz		EDITOR									// head param data is stored in a different register in the runtime

		push	ecx		
		jmp		WEITER
	EDITOR:
		push	ebp
	WEITER:
		call	DoBSFaceGetAgeTexturePathHook
		jmp		kBSFaceGetAgeTexturePathRetnAddr		// our call will take care of the stack pointer
	}
}


void PatchHeadOverride( void )
{
	struct PatchSiteEins
	{
		InstanceAbstraction::MemAddr	TESRaceGetFaceGenHeadParameters;
		InstanceAbstraction::MemAddr	FaceGenHeadParametersDtor;

		PatchSiteEins(UInt32 GameA, UInt32 EditorA, UInt32 GameB, UInt32 EditorB)
		{
			TESRaceGetFaceGenHeadParameters.Game = GameA;
			TESRaceGetFaceGenHeadParameters.Editor = EditorA;

			FaceGenHeadParametersDtor.Game = GameB;
			FaceGenHeadParametersDtor.Editor = EditorB;
		}
	};

	std::vector<PatchSiteEins> HookLocations;		// 7 in-game and 3 in-editor

	HookLocations.push_back(PatchSiteEins(0x00528BF5, 0x004D9693, 0x00528C17, 0x004D9785));
	HookLocations.push_back(PatchSiteEins(0x00529301, 0x004DA46B, 0x00529356, 0x004DA48D));
	HookLocations.push_back(PatchSiteEins(0x0052966E, 0x004E739B, 0x00529702, 0x004E7405));
	HookLocations.push_back(PatchSiteEins(0x0052E03B, 0, 0x0052E0A5, 0));
	HookLocations.push_back(PatchSiteEins(0x005C7720, 0, 0x005C777A, 0));
	HookLocations.push_back(PatchSiteEins(0x005C7AC1, 0, 0x005C7B1B, 0));
	HookLocations.push_back(PatchSiteEins(0x005C936F, 0, 0x005C93C8, 0));

	for (int i = 0; i < HookLocations.size(); i++)
	{
		PatchSiteEins& Site = HookLocations[i];

		_DefineCallHdlr(PatchHookA, Site.TESRaceGetFaceGenHeadParameters(), TESRaceGetFaceGenHeadParametersHook);
		_DefineCallHdlr(PatchHookB, Site.FaceGenHeadParametersDtor(), FaceGenHeadParametersDtorHook);

		_MemHdlr(PatchHookA).WriteCall();
		_MemHdlr(PatchHookB).WriteCall();
	}

	struct PatchSiteZwei
	{
		InstanceAbstraction::MemAddr	BSFaceGenDoSomethingWithFaceGenNode;
		InstanceAbstraction::MemAddr	FaceGenHeadParametersDtor;

		PatchSiteZwei(UInt32 GameA, UInt32 EditorA, UInt32 GameB, UInt32 EditorB)
		{
			BSFaceGenDoSomethingWithFaceGenNode.Game = GameA;
			BSFaceGenDoSomethingWithFaceGenNode.Editor = EditorA;

			FaceGenHeadParametersDtor.Game = GameB;
			FaceGenHeadParametersDtor.Editor = EditorB;
		}
	};

	// special case for the facegen model normal fixing code
	// we only patch one of the two consecutive calls to the BSFaceGen function as the swapping is a one-time procedure
	const PatchSiteZwei kJustTheOne(0x005289BB, 0x004DA22F, 0x005289E3, 0x004DA257);		
	_DefineJumpHdlr(PatchHookA, kJustTheOne.BSFaceGenDoSomethingWithFaceGenNode(), BSFaceGenDoSomethingWithFaceGenNodeHook);
	_DefineCallHdlr(PatchHookB, kJustTheOne.FaceGenHeadParametersDtor(), FaceGenHeadParametersDtorHook);

	_MemHdlr(PatchHookA).WriteJump();
	_MemHdlr(PatchHookB).WriteCall();																							

	const InstanceAbstraction::MemAddr	kBSFaceGetAgeTexturePath = { 0x00555457, 0x00587D4D };

	_DefineJumpHdlr(PatchHook, kBSFaceGetAgeTexturePath(), (UInt32)&BSFaceGetAgeTexturePathHook);
	_MemHdlr(PatchHook).WriteJump();
	
	kBSFaceGetAgeTexturePathRetnAddr = kBSFaceGetAgeTexturePath() + 0x8;
	kBSFaceGenDoSomethingWithFaceGenNodeRetnAddr = kJustTheOne.BSFaceGenDoSomethingWithFaceGenNode() + 0x5;
}

namespace HeadOverride
{
	void HandleLoadGame( void )
	{
		ScriptHeadOverrideAgent::TextureOverrides.Clear();
		ScriptHeadOverrideAgent::MeshOverrides.Clear();
	}
}
