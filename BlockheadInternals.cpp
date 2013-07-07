#include "BlockheadInternals.h"

IDebugLog					gLog("Blockhead.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
BlockheadINIManager			BlockheadINIManager::Instance;

SME::INI::INISetting		kGenderVariantHeadMeshes("GenderVariantHeadMeshes", "General",
													"Use different head models for male and female NPCs", (SInt32)1);

SME::INI::INISetting		kGenderVariantHeadTextures("GenderVariantHeadTextures", "General",
													   "Use different head textures for male and female NPCs", (SInt32)1);

SME::INI::INISetting		kAllowESPFacegenTextureUse("AllowESPFacegenTextureUse", "General",
													"Use editor-generated facegen textures for ESP files", (SInt32)0);

void BlockheadINIManager::Initialize( const char* INIPath, void* Parameter )
{
	this->INIFilePath = INIPath;
	_MESSAGE("INI Path: %s", INIPath);

	std::fstream INIStream(INIPath, std::fstream::in);
	bool CreateINI = false;

	if (INIStream.fail())
	{
		_MESSAGE("INI File not found; Creating one...");
		CreateINI = true;
	}

	INIStream.close();
	INIStream.clear();

	RegisterSetting(&kGenderVariantHeadMeshes);
	RegisterSetting(&kGenderVariantHeadTextures);
	RegisterSetting(&kAllowESPFacegenTextureUse);

	if (CreateINI)
		Save();
}

enum
{
	kSwap_HeadModel			= 0,
	kSwap_HeadTexture
};

bool SwapFaceGenHeadData(UInt8 Swap, TESNPC* NPC, String* OldPath, String* NewPath)
{
	std::string Extension;
	std::string Postfix;
	std::string BasePath;

	if (NPC == NULL || OldPath->m_data == NULL)
		return false;

	switch (Swap)
	{
	case kSwap_HeadModel:
		if (kGenderVariantHeadMeshes.GetData().i == 0)
			return false;

		Extension = ".nif";
		BasePath = "Meshes\\";
		break;
	case kSwap_HeadTexture:
		if (kGenderVariantHeadTextures.GetData().i == 0)
			return false;

		Extension = ".nif";
		BasePath = "Textures\\";
		break;
	}

	if (NPC->actorBaseData.IsFemale())
		Postfix = "_f";
	else
		Postfix = "_m";

	std::string AssetPath(OldPath->m_data);

	// remove extension
	AssetPath.erase(AssetPath.length() - 4, 4);
	AssetPath += Postfix + Extension;

	std::string FindPath = BasePath + AssetPath;

	if ((*g_FileFinder)->FindFile(FindPath.c_str(), 0, 0, -1) != FileFinder::kFileStatus_NotFound)
	{
		// asset found at the new path
		NewPath->Set(AssetPath.c_str());

#ifndef NDEBUG
		_MESSAGE("Swapped head asset path %s to %s", OldPath->m_data, AssetPath.c_str());
#endif

		return true;
	}

	return false;
}

void __stdcall DoTESRaceGenerateFaceGenHeadHook(TESRace* Race, FaceGenHeadParameters* FaceGenParams, TESNPC* NPC)
{
	// call original function to get the parameters
	thisCall<void>(0x0052CD50, Race, NPC, FaceGenParams);

	// swap the head model/texture pointer with a newly allocated one
	// to allow for the changing of the asset paths
	TESModel* NewHeadModel = (TESModel*)FormHeap_Allocate(sizeof(TESModel));
	thisCall<void>(0x0046D7E0, NewHeadModel);	// ctor
	TESTexture* NewHeadTexture = (TESTexture*)FormHeap_Allocate(sizeof(TESTexture));
	thisCall<void>(0x0046FFD0, NewHeadTexture);	// ctor

	TESModel* ExistingHeadModel = FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head];
	TESTexture* ExistingHeadTexture = FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head];

	if (ExistingHeadModel == NULL || ExistingHeadTexture == NULL)
	{
		// if the head model/texture should be NULL for whatever reason, slinky away
	}
	else
	{
		NewHeadModel->nifPath.Set(ExistingHeadModel->nifPath.m_data);
		NewHeadTexture->ddsPath.Set(ExistingHeadTexture->ddsPath.m_data);

		if (NPC && NPC->refID == 0x7)
		{
			// better not to touch the player character
		}
		else
		{
			// check gender and append the appropriate postfix
			// if there's no asset at the new path, revert to the old one
			SwapFaceGenHeadData(kSwap_HeadModel, NPC, &ExistingHeadModel->nifPath, &NewHeadModel->nifPath);
			SwapFaceGenHeadData(kSwap_HeadTexture, NPC, &ExistingHeadTexture->ddsPath, &NewHeadTexture->ddsPath);
		}
	}

	// finally swap the pointers, which will be released in the subsequent call to the facegen parameter object's dtor
	FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head] = NewHeadModel;
	FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head] = NewHeadTexture;
}

void __declspec(naked) TESRaceGenerateFaceGenHeadHook(void)
{
	__asm
	{
//		int		3
		push	[esp + 0x4]
		push	[esp + 0xC]
		push	ecx
		call	DoTESRaceGenerateFaceGenHeadHook
		retn	0x8
	}
}

void __stdcall DoFaceGenHeadParametersDtorHook(FaceGenHeadParameters* FaceGenParams)
{
	// sanity check
	if (FaceGenParams->models.numObjs)
	{
		TESModel* SneakyBugger = FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head];

		// emancipate the bugger, unto death
		thisCall<void>(0x0046D850, SneakyBugger);	// dtor
		FormHeap_Free(SneakyBugger);
	}

	if (FaceGenParams->textures.numObjs)
	{
		TESTexture* SneakyBugger = FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head];

		thisCall<void>(0x00470040, SneakyBugger);
		FormHeap_Free(SneakyBugger);
	}

	thisCall<void>(0x00526CE0, FaceGenParams);		// dtor
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

_DefineNopHdlr(UseFaceGenHeadTextures, 0x00524187, 0x0052418D - 0x00524187);

void BlockHeads( void )
{
	static const UInt32 PatchCallSites = 7;

	struct PatchSite
	{
		UInt32	TESRaceGenerateFaceGenHead;
		UInt32	FaceGenHeadParametersDtor;
	} HookLocations[PatchCallSites] =
	{
		{ 0x00528BF5, 0x00528C17 },
		{ 0x00529301, 0x00529356 },
		{ 0x0052966E, 0x00529702 },
		{ 0x0052E03B, 0x0052E0A5 },
		{ 0x005C7720, 0x005C777A },
		{ 0x005C7AC1, 0x005C7B1B },
		{ 0x005C936F, 0x005C93C8 }
	};

	for (int i = 0; i < PatchCallSites; i++)
	{
		PatchSite& Site = HookLocations[i];
		
		_DefineCallHdlr(PatchHookA, Site.TESRaceGenerateFaceGenHead, TESRaceGenerateFaceGenHeadHook);
		_DefineCallHdlr(PatchHookB, Site.FaceGenHeadParametersDtor, FaceGenHeadParametersDtorHook);

		
		_MemHdlr(PatchHookA).WriteCall();
		_MemHdlr(PatchHookB).WriteCall();
	}

	if (kAllowESPFacegenTextureUse.GetData().i)	
		_MemHdlr(UseFaceGenHeadTextures).WriteNop();
}
