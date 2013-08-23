#include "FaceGen.h"
enum
{
	kSwap_HeadModel			= 0,
	kSwap_HeadTexture
};

bool FixupFaceGenHeadAssetPath(UInt8 Swap, FaceGenHeadParameters* Params, InstanceAbstraction::BSString* OldPath, InstanceAbstraction::BSString* NewPath)
{
	std::string Extension;
	std::string Postfix;
	std::string BasePath;

	SME_ASSERT(Params && OldPath && NewPath);

	if (OldPath->m_data == NULL)
		return false;

	switch (Swap)
	{
	case kSwap_HeadModel:
		if (Settings::kGenderVariantHeadMeshes.GetData().i == 0)
			return false;

		Extension = ".nif";
		BasePath = "Meshes\\";
		break;
	case kSwap_HeadTexture:
		if (Settings::kGenderVariantHeadTextures.GetData().i == 0)
			return false;

		Extension = ".dds";
		BasePath = "Textures\\";
		break;
	}

	if (Params->female)
		Postfix = "_f";
	else
		Postfix = "_m";

	std::string AssetPath(OldPath->m_data);

	// remove extension
	AssetPath.erase(AssetPath.length() - 4, 4);
	AssetPath += Postfix + Extension;

	std::string FindPath = BasePath + AssetPath;
	SME::StringHelpers::MakeLower(FindPath);

	if (InstanceAbstraction::FileFinder::GetFileExists(FindPath.c_str()))
	{
		// asset found at the new path
		NewPath->Set(AssetPath.c_str());

#ifndef NDEBUG
		_MESSAGE("Swapped head asset path %s to %s", OldPath->m_data, AssetPath.c_str());
#endif

		return true;
	}
	else
	{
#ifndef NDEBUG
		_MESSAGE("Couldn't find head asset swap at %s. Using the unisex asset", FindPath.c_str());
#endif
	}

	return false;
}

struct HeadTextureOverrideData
{
	InstanceAbstraction::TESTexture::Instance		Original;
	bool											HasOverride;			// set to true when the texture path has been modified by us
};

typedef std::map<InstanceAbstraction::TESTexture::Instance, HeadTextureOverrideData> OverrideHeadTextureMapT;
static OverrideHeadTextureMapT OverriddenHeadTextureTempDataStore;

void SwapFaceGenHeadData(TESRace* Race, FaceGenHeadParameters* FaceGenParams, TESNPC* NPC, bool FixingFaceNormals)
{
	// swap the head model/texture pointer with a newly allocated one
	// to allow for the changing of the asset paths	
	InstanceAbstraction::TESModel::Instance NewHeadModel = InstanceAbstraction::TESModel::CreateInstance();
	InstanceAbstraction::TESTexture::Instance NewHeadTexture = InstanceAbstraction::TESTexture::CreateInstance();

	InstanceAbstraction::TESModel::Instance ExistingHeadModel = (InstanceAbstraction::TESModel::Instance)
																FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head];
	InstanceAbstraction::TESTexture::Instance ExistingHeadTexture = (InstanceAbstraction::TESTexture::Instance)
																	FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head];

	if (ExistingHeadModel == NULL || ExistingHeadTexture == NULL)
	{
		// if the head model/texture should be NULL for whatever reason, slinky away
		_MESSAGE("Gadzooks! Why the heck is the TESModel/TESTexture BaseFormComponent NULL?!");
		
		if (Race && NPC)
			_MESSAGE("TESRace = %08X, TESNPC = %08X", Race->refID, NPC->refID);
	}
	else
	{
		InstanceAbstraction::TESModel::GetPath(NewHeadModel)->Set(InstanceAbstraction::TESModel::GetPath(ExistingHeadModel)->m_data);
		InstanceAbstraction::TESTexture::GetPath(NewHeadTexture)->Set(InstanceAbstraction::TESTexture::GetPath(ExistingHeadTexture)->m_data);
		
#ifndef NDEBUG
		if (NPC)
		{
			if (NPC->refID == 0x7)
				_MESSAGE("Generating FaceGen head for the player character...");
			else
				_MESSAGE("Generating FaceGen head for NPC %08X...", NPC->refID);
		}
		else if (FixingFaceNormals)
			_MESSAGE("Fixing FaceGen normals...");

		gLog.Indent();
#endif
		// check gender and append the appropriate postfix
		// if there's no asset at the new path, revert to the old one
		FixupFaceGenHeadAssetPath(kSwap_HeadModel, FaceGenParams,
								InstanceAbstraction::TESModel::GetPath(ExistingHeadModel),
								InstanceAbstraction::TESModel::GetPath(NewHeadModel));

		// save the original TESTexture pointer and the result of the swap op to the override map
		// we check it later to fixup the age overlay texture paths
		HeadTextureOverrideData OverrideTexData;
		OverrideTexData.Original = ExistingHeadTexture;
		OverrideTexData.HasOverride = FixupFaceGenHeadAssetPath(kSwap_HeadTexture, FaceGenParams,
														InstanceAbstraction::TESTexture::GetPath(ExistingHeadTexture),
														InstanceAbstraction::TESTexture::GetPath(NewHeadTexture));

		OverriddenHeadTextureTempDataStore[NewHeadTexture] = OverrideTexData;

#ifndef NDEBUG
		gLog.Outdent();
#endif
	}

	// finally swap the pointers, which will be released in the subsequent call to the facegen parameter object's dtor
	FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head] = (::TESModel*)NewHeadModel;
	FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head] = (::TESTexture*)NewHeadTexture;
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
	// sanity check
	if (FaceGenParams->models.numObjs)
	{
		InstanceAbstraction::TESModel::Instance SneakyBugger = (InstanceAbstraction::TESModel::Instance)
																FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head];
		// emancipate the bugger, unto death
		InstanceAbstraction::TESModel::DeleteInstance(SneakyBugger);
	}

	if (FaceGenParams->textures.numObjs)
	{
		InstanceAbstraction::TESTexture::Instance SneakyBugger = (InstanceAbstraction::TESTexture::Instance)
																FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head];

		// remove the cached override data
		if (OverriddenHeadTextureTempDataStore.count(SneakyBugger))
		{
			OverriddenHeadTextureTempDataStore.erase(SneakyBugger);
		}

		InstanceAbstraction::TESTexture::DeleteInstance(SneakyBugger);
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

void __cdecl BSFaceGenDoSomethingWithFaceGenNodeHook(NiNode* FaceGenNode, FaceGenHeadParameters* HeadParams)
{
	SwapFaceGenHeadData(NULL, HeadParams, NULL, true);

	cdeclCall<void>(InstanceAbstraction::kBSFaceGen_DoSomethingWithFaceGenNode(), FaceGenNode, HeadParams);
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
	
	if (OverriddenHeadTextureTempDataStore.count(OverriddenTexture))
	{
		HeadTextureOverrideData& Data = OverriddenHeadTextureTempDataStore[OverriddenTexture];

		if (Data.HasOverride)
		{
			// the base head texture path has been overridden by us, use the original base path instead
			// age textures are already gender variant so revert out changes		
			const char* OriginalPath = InstanceAbstraction::TESTexture::GetPath(Data.Original)->m_data;

#ifndef NDEBUG
			_MESSAGE("Reset head asset path %s to %s for age texture generation [G=%d, A=%d]", BasePath, OriginalPath, Gender, Age);
#endif
			BasePath = OriginalPath;
		}
	}
	else
	{
		// this should never happen
		_MESSAGE("By the powers of Grey-Skull! BSFaceGen::GetAgeTexturePath hook couldn't find override texture data!");
		_MESSAGE("BasePath = %s, G=%d, A=%d", BasePath, Gender, Age);
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


void PatchFaceGen( void )
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
	_DefineCallHdlr(PatchHookA, kJustTheOne.BSFaceGenDoSomethingWithFaceGenNode(), BSFaceGenDoSomethingWithFaceGenNodeHook);
	_DefineCallHdlr(PatchHookB, kJustTheOne.FaceGenHeadParametersDtor(), FaceGenHeadParametersDtorHook);

	_MemHdlr(PatchHookA).WriteCall();
	_MemHdlr(PatchHookB).WriteCall();																							

	const InstanceAbstraction::MemAddr	kBSFaceGetAgeTexturePath = { 0x00555457, 0x00587D4D };

	_DefineJumpHdlr(PatchHook, kBSFaceGetAgeTexturePath(), (UInt32)&BSFaceGetAgeTexturePathHook);
	_MemHdlr(PatchHook).WriteJump();
	kBSFaceGetAgeTexturePathRetnAddr = kBSFaceGetAgeTexturePath() + 0x8;
}
