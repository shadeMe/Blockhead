#include "HeadOverride.h"

ScriptedActorAssetOverrider<ScriptedTextureOverrideData>		ScriptHeadOverrideAgent::TextureOverrides;
ScriptedActorAssetOverrider<ScriptedModelOverrideData>			ScriptHeadOverrideAgent::MeshOverrides;
FaceGenAgeTextureOverrider										FaceGenAgeTextureOverrider::Instance;

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
	// only used when overriding body parts without default assets
	return "Characters\\HeadAssetOverrides\\PerRace";
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
		char Buffer[MAX_PATH] = {0};
		const char* PathSuffix = Data->GetComponentName();
		const char* BaseDir = Data->GetRootDirectory();
		const char* GenderPath = NULL;
		if (InstanceAbstraction::GetNPCFemale(Data->Actor))
			GenderPath = "F";
		else
			GenderPath = "M";

		if (Data->AssetPath && strlen(Data->AssetPath))
		{
			// per-race handling is mostly unnecessary for head parts
			// we only handle those that are in need of gender variance
			if (GetComponentGenderVariant() == false)
			{
				std::string OriginalPath(Data->AssetPath);

				// remove extension
				OriginalPath.erase(OriginalPath.length() - 4, 4);
				FORMAT_STR(Buffer, "%s\\%s_%s.%s", BaseDir, OriginalPath.c_str(), GenderPath, Data->GetFileExtension());
#ifndef NDEBUG
				_MESSAGE("Checking override path %s for NPC '%s' (%08X)", Buffer, InstanceAbstraction::GetFormName(Data->Actor), Data->Actor->refID);
#endif // !NDEBUG

				if (InstanceAbstraction::FileFinder::GetFileExists(Buffer))
				{
					Result = true;
					FORMAT_STR(Buffer, "%s_%s.%s", OriginalPath.c_str(), GenderPath, Data->GetFileExtension());
					OutOverridePath = Buffer;
				}
			}
		}
		else
		{
			// fallback to the override directory when there's no default path
			const char* RaceName = InstanceAbstraction::GetFormName(Data->Race);
			if (RaceName)
			{
				// in the case ears, the gender component in the path must match that of the body part
				// for instance, EarsFemale overrides must be placed in the 'F' directory
				FORMAT_STR(Buffer, "%s\\%s\\%s_%s.%s", GetOverrideSourceDirectory(), GenderPath, RaceName, PathSuffix, Data->GetFileExtension());
#ifndef NDEBUG
				_MESSAGE("Checking override path %s for NPC '%s' (%08X)", Buffer, InstanceAbstraction::GetFormName(Data->Actor), Data->Actor->refID);
#endif // !NDEBUG

				std::string FullPath(BaseDir); FullPath += "\\" + std::string(Buffer);
				if (InstanceAbstraction::FileFinder::GetFileExists(FullPath.c_str()))
				{
					Result = true;
					OutOverridePath = Buffer;
				}
			}
		}
	}

	return Result;
}
const char* FaceGenAgeTextureOverrider::kOverrideSourceDirectory = "Characters\\AgeTextureOverrides";

FaceGenAgeTextureOverrider::FaceGenAgeTextureOverrider() :
	OverriddenHeadTextures(),
	ScriptOverrides(),
	Lock()
{
	;//
}

FaceGenAgeTextureOverrider::~FaceGenAgeTextureOverrider()
{
	OverriddenHeadTextures.clear();
	ResetAgeTextureScriptOverrides();
}

void FaceGenAgeTextureOverrider::TrackHeadOverride( Texture Duplicate, Texture Original )
{
	ScopedLock Guard(Lock);

	SME_ASSERT(Duplicate && Original);
	SME_ASSERT(OverriddenHeadTextures.count(Duplicate) == 0);
	OverriddenHeadTextures[Duplicate] = Original;
}

void FaceGenAgeTextureOverrider::UntrackHeadOverride( Texture Duplicate )
{
	ScopedLock Guard(Lock);

	if (OverriddenHeadTextures.count(Duplicate))
		OverriddenHeadTextures.erase(Duplicate);
}

void FaceGenAgeTextureOverrider::RegisterAgeTextureScriptOverride( TESNPC* NPC, const char* BasePath )
{
	ScopedLock Guard(Lock);

	SME_ASSERT(NPC && BasePath);

	ScriptOverrides[NPC->refID] = BasePath;
}

void FaceGenAgeTextureOverrider::UnregisterAgeTextureScriptOverride( TESNPC* NPC )
{
	ScopedLock Guard(Lock);

	SME_ASSERT(NPC);

	ScriptOverrides.erase(NPC->refID);
}

void FaceGenAgeTextureOverrider::ResetAgeTextureScriptOverrides( void )
{
	ScopedLock Guard(Lock);

	ScriptOverrides.clear();
}

bool FaceGenAgeTextureOverrider::TryGetClosestAgeTexture( std::string& OutPath, const char* BasePath, SInt32 Age, bool Female, bool UseGender ) const
{
	bool Result = false;

#ifndef NDEBUG
	_MESSAGE("Fetching closest texture for age %d @ %s...", Age, BasePath);
	gLog.Indent();
#endif

	if (Age <= 100)
	{
		const char* GenderPath = (Female == false ? "M" : "F");
		char Buffer[MAX_PATH] = {0};
		FORMAT_STR(Buffer, "Textures\\%s%s%d.dds", BasePath, (UseGender ? GenderPath : ""), Age);
		if (InstanceAbstraction::FileFinder::GetFileExists(Buffer))
		{
#ifndef NDEBUG
			_MESSAGE("Exact match for age - %s", Buffer);
#endif
			Result = true;
			OutPath = Buffer;
		}
		else
		{
			Age -= Age % 10;
			while (Age > 0)
			{
				FORMAT_STR(Buffer, "Textures\\%s%s%d.dds", BasePath, (UseGender ? GenderPath : ""), Age);
				if (InstanceAbstraction::FileFinder::GetFileExists(Buffer))
				{
#ifndef NDEBUG
					_MESSAGE("Closest age = %d @ %s", Age, Buffer);
#endif
					Result = true;
					OutPath = Buffer;
					break;
				}

				Age -= 10;
			}
		}
	}

#ifndef NDEBUG
	gLog.Outdent();
#endif

	return Result;
}

std::string FaceGenAgeTextureOverrider::GetAgeTexturePath( TESNPC* NPC, SInt32 Age, const char* CurrentBasePath, Texture HeadTexture ) const
{
	ScopedLock Guard(Lock);

	SME_ASSERT(NPC);
	std::string Result;

#ifndef NDEBUG
	_MESSAGE("Looking up age %d texture for NPC %08X...", Age, NPC->refID);
	gLog.Indent();
#endif

	while (true)
	{
#ifndef NDEBUG
		_MESSAGE("Checking script overrides...");
		gLog.Indent();
#endif
		// scripted overrides have the highest priority
		if (ScriptOverrides.count(NPC->refID))
		{
			Result.clear();
			if (TryGetClosestAgeTexture(Result, ScriptOverrides.at(NPC->refID).c_str(), Age, false, false))
			{
#ifndef NDEBUG
				gLog.Outdent();
#endif
				break;
			}
		}
#ifndef NDEBUG
		gLog.Outdent();
		_MESSAGE("Checking non-script overrides...");
		gLog.Indent();
#endif
		// check the override directory
		UInt32 FormID = NPC->refID & 0x00FFFFFF;
		TESFile* Plugin = InstanceAbstraction::GetOverrideFile(NPC, 0);
		if (Plugin)
		{
			char Buffer[MAX_PATH] = {0};
			FORMAT_STR(Buffer, "%s\\%s\\%08X_", kOverrideSourceDirectory, Plugin->name, FormID);
			Result.clear();
			if (TryGetClosestAgeTexture(Result, Buffer, Age, false, false))
			{
#ifndef NDEBUG
				gLog.Outdent();
#endif
				break;
			}
		}
#ifndef NDEBUG
		gLog.Outdent();
#endif

		std::string OrgBasePath(CurrentBasePath);
		if (OverriddenHeadTextures.count(HeadTexture))
		{
			// get the base head texture
			OrgBasePath = InstanceAbstraction::TESTexture::GetPath(OverriddenHeadTextures.at(HeadTexture))->m_data;
#ifndef NDEBUG
			_MESSAGE("Reset head asset path to %s", OrgBasePath.c_str());
#endif
		}
		else
		{
#ifndef NDEBUG
			_MESSAGE("No overrides, using current base path");
#endif
		}

		// remove extension
		OrgBasePath.erase(OrgBasePath.length() - 4, 4);
		Result.clear();
		TryGetClosestAgeTexture(Result, OrgBasePath.c_str(), Age, InstanceAbstraction::GetNPCFemale(NPC), true);

		break;
	}

#ifndef NDEBUG
	gLog.Outdent();
#endif

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
	_MESSAGE("Name: %s\tRace: %s", InstanceAbstraction::GetFormName(NPC), InstanceAbstraction::GetFormName(Race));

	if (FixingFaceNormals)
		_MESSAGE("Fixing FaceGen normals...");

//	FaceGenParams->DebugDump();
#endif

	// sanity check, remove invalid model/texture pointers
	for (int i = FaceGenHeadParameters::kFaceGenData__BEGIN; i < FaceGenHeadParameters::kFaceGenData__END; i++)
	{
		InstanceAbstraction::TESModel::Instance ThisModel = (InstanceAbstraction::TESModel::Instance)
													FaceGenParams->models.data[i];
		InstanceAbstraction::TESTexture::Instance ThisTexture = (InstanceAbstraction::TESTexture::Instance)
													FaceGenParams->textures.data[i];

		if (ThisModel)
		{
			if (InstanceAbstraction::TESModel::GetPath(ThisModel)->m_data == NULL)
			{
#ifndef NDEBUG
		//		_MESSAGE("Removed invalid model for head part %d", i);
#endif
				FaceGenParams->models.data[i] = NULL;
			}
		}

		if (ThisTexture)
		{
			if (InstanceAbstraction::TESTexture::GetPath(ThisTexture)->m_data == NULL)
			{
#ifndef NDEBUG
		//		_MESSAGE("Removed invalid texture for head part %d", i);
#endif
				FaceGenParams->textures.data[i] = NULL;
			}
		}
	}

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
			SME_ASSERT(OrgModelPath);
			InstanceAbstraction::TESModel::GetPath(NewModel)->Set(OrgModelPath);
		}

		// NPC will NULL when generating heads in the editor's Race edit dialog
		if (NPC == NULL)
		{
			if (NewModel)
				FaceGenParams->models.data[i] = (::TESModel*)NewModel;
		}
		else
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
			SME_ASSERT(OrgTexturePath);
			InstanceAbstraction::TESTexture::GetPath(NewTexture)->Set(OrgTexturePath);
		}

		if (NPC == NULL)
		{
			if (NewTexture)
				FaceGenParams->textures.data[i] = (::TESTexture*)NewTexture;
		}
		else
		{
			ActorHeadAssetData Data(ActorHeadAssetData::kAssetType_Texture, i, NPC, OrgTexturePath);
			char OverridePath[MAX_PATH] = {0};
			std::string ResultPath;

			bool ReplacePointer = true;
			bool OverrideOp = ActorAssetOverriderKernel::Instance.ApplyOverride(&Data, ResultPath);
			if (NonExtantTexture)
			{
				if (OverrideOp == false)
					ReplacePointer = false;
				else
					NewTexture = InstanceAbstraction::TESTexture::CreateInstance();
			}

			FORMAT_STR(OverridePath, "%s", ResultPath.c_str());

			if (ReplacePointer)
			{
				SME_ASSERT(NewTexture);

				// save the original TESTexture pointer of kFaceGenData_Head when an override is active
				// we check it later to fixup the age overlay texture paths
				if (i == FaceGenHeadParameters::kFaceGenData_Head && OverrideOp)
					FaceGenAgeTextureOverrider::Instance.TrackHeadOverride(NewTexture, OrgTexture);

				InstanceAbstraction::TESTexture::GetPath(NewTexture)->Set(OverridePath);
				FaceGenParams->textures.data[i] = (::TESTexture*)NewTexture;
			}
		}
	}

	if (FaceGenParams->hair)
	{
		InstanceAbstraction::TESHair::Instance NewHair = InstanceAbstraction::TESHair::CreateInstance();
		InstanceAbstraction::TESHair::Instance OldHair = (InstanceAbstraction::TESHair::Instance)FaceGenParams->hair;
		InstanceAbstraction::TESHair::CopyFlags(OldHair, NewHair);

		InstanceAbstraction::BSString* OldModel = InstanceAbstraction::TESModel::GetPath(InstanceAbstraction::TESHair::GetModel(OldHair));
		InstanceAbstraction::BSString* OldTexture = InstanceAbstraction::TESTexture::GetPath(InstanceAbstraction::TESHair::GetTexture(OldHair));
		InstanceAbstraction::BSString* NewModel = InstanceAbstraction::TESModel::GetPath(InstanceAbstraction::TESHair::GetModel(NewHair));
		InstanceAbstraction::BSString* NewTexture = InstanceAbstraction::TESTexture::GetPath(InstanceAbstraction::TESHair::GetTexture(NewHair));

		if (OldModel->m_data)
			NewModel->Set(OldModel->m_data);

		if (Settings::kHeadOverrideHairGenderVariantModel().i)
		{
			if (OldModel->m_data)
			{
				std::string AssetPath(OldModel->m_data);
				AssetPath.erase(AssetPath.length() - 4, 4);		// remove extension
				if (FaceGenParams->female)
					AssetPath += "-F.nif";
				else
					AssetPath += "-M.nif";

				std::string OverridePath = "Meshes\\" + AssetPath;
#ifndef NDEBUG
				_MESSAGE("Checking hair model override at %s", OverridePath.c_str());
				gLog.Indent();
#endif
				if (InstanceAbstraction::FileFinder::GetFileExists(OverridePath.c_str()))
				{
					NewModel->Set(AssetPath.c_str());
#ifndef NDEBUG
					_MESSAGE("Hair asset override applied");
#endif
				}
#ifndef NDEBUG
				gLog.Outdent();
#endif
			}
		}

		if (OldTexture->m_data)
			NewTexture->Set(OldTexture->m_data);

		if (Settings::kHeadOverrideHairGenderVariantTexture().i)
		{
			if (OldTexture->m_data)
			{
				std::string AssetPath(OldTexture->m_data);
				AssetPath.erase(AssetPath.length() - 4, 4);		// remove extension
				if (FaceGenParams->female)
					AssetPath += "-F.dds";
				else
					AssetPath += "-M.dds";

				std::string OverridePath = "Textures\\" + AssetPath;
#ifndef NDEBUG
				_MESSAGE("Checking hair texture override at %s", OverridePath.c_str());
				gLog.Indent();
#endif
				if (InstanceAbstraction::FileFinder::GetFileExists(OverridePath.c_str()))
				{
					NewTexture->Set(AssetPath.c_str());
#ifndef NDEBUG
					_MESSAGE("Hair asset override applied");
#endif
				}
#ifndef NDEBUG
				gLog.Outdent();
#endif
			}
		}

		FaceGenParams->hair = (::TESHair*)NewHair;
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
				FaceGenAgeTextureOverrider::Instance.UntrackHeadOverride(SneakyBugger);
				InstanceAbstraction::TESTexture::DeleteInstance(SneakyBugger);
			}
		}
	}

	if (FaceGenParams->hair)
		InstanceAbstraction::TESHair::DeleteInstance(FaceGenParams->hair);

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

bool TryGetNPC(TESNPC* NPC)
{
	// ### HACKY HACK HACK HACKETT HACK
	// easier than mapping FaceGenHeadParam instances to their corresponding NPCs though
	bool Result = false;
	__try
	{
		switch (*((UInt32*)NPC))
		{
		case 0x0094561C:		// editor vtbl
		case 0x00A53DD4:		// runtime vtbl
			Result = true;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		Result = false;
	}

	return Result;
}

const char* __stdcall DoBSFaceGetAgeTexturePathHook(FaceGenHeadParameters* HeadParams,
													TESNPC* NPC,
													InstanceAbstraction::BSString* OutPath,
													UInt32 Gender,
													SInt32 Age,
													const char* BasePath)
{
	static const InstanceAbstraction::MemAddr kCallAddr = { 0x00551A00, 0x005845F0 };

	SME_ASSERT(HeadParams && HeadParams->textures.numObjs);

	InstanceAbstraction::TESTexture::Instance OverriddenTexture = (InstanceAbstraction::TESTexture::Instance)
																HeadParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head];

	SME_ASSERT(OverriddenTexture);

	if (NPC)
	{
		if (TryGetNPC(NPC))
		{
			std::string AgeTexPath = FaceGenAgeTextureOverrider::Instance.GetAgeTexturePath(NPC, Age, BasePath, OverriddenTexture);
			if (AgeTexPath.length())
				OutPath->Set(AgeTexPath.c_str());
			else
				OutPath->Set("");

			return OutPath->m_data;
		}
		else
		{
#ifndef NDEBUG
			_MESSAGE("BSFaceGetAgeTexturePathHook - Bad NPC Pointer @ 0x%08X!", NPC);
#endif
		}
	}

	return cdeclCall<const char*>(kCallAddr(), OutPath, Gender, Age, BasePath);
}

static UInt32		kBSFaceGetAgeTexturePathRetnAddr = 0;

#define _hhName		BSFaceGetAgeTexturePath
_hhBegin()
{
	__asm
	{
		mov		eax, [esp + 0x1C]						// ### HACK HACK - volatile stack space, can get overwritten
		push	eax
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

	kBSFaceGenDoSomethingWithFaceGenNodeRetnAddr = kJustTheOne.BSFaceGenDoSomethingWithFaceGenNode() + 0x5;

	const InstanceAbstraction::MemAddr	kBSFaceGetAgeTexturePath = { 0x00555457, 0x00587D4D };

	_DefineJumpHdlr(PatchHook, kBSFaceGetAgeTexturePath(), (UInt32)&BSFaceGetAgeTexturePathHook);
	_MemHdlr(PatchHook).WriteJump();

	kBSFaceGetAgeTexturePathRetnAddr = kBSFaceGetAgeTexturePath() + 0x8;
}

namespace HeadOverride
{
	void HandleLoadGame( void )
	{
		ScriptHeadOverrideAgent::TextureOverrides.Clear();
		ScriptHeadOverrideAgent::MeshOverrides.Clear();
		FaceGenAgeTextureOverrider::Instance.ResetAgeTextureScriptOverrides();
	}
}