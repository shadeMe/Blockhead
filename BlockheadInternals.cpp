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

namespace InstanceAbstraction
{
	bool EditorMode = false;

	const MemAddr kTESRace_GetFaceGenHeadParameters	= { 0x0052CD50, 0x004E6AA0 };

	const MemAddr kFormHeap_Allocate				= { 0x00401F00, 0x00401E80 };
	const MemAddr kFormHeap_Free					= { 0x00401F20, 0x00401EA0 };
	
	const MemAddr kTESModel_Ctor					= { 0x0046D7E0, 0x0049D040 };
	const MemAddr kTESModel_Dtor					= { 0x0046D850, 0x0049CF40 };
	
	const MemAddr kTESTexture_Ctor					= { 0x0046FFD0, 0x004A3FF0 };
	const MemAddr kTESTexture_Dtor					= { 0x00470040, 0x004A4050 };

	const MemAddr kFaceGenHeadParameters_Dtor		= { 0x00526CE0, 0x004D88C0 };
	
	const MemAddr kFileFinder_Singleton				= { 0x00B33A04, 0x00A0DE8C };

	namespace TESModel
	{
		Instance CreateInstance( BSString** OutPath )
		{
			typedef Instance (* _fn)(UInt32 Size);
			const _fn fn = (_fn)kFormHeap_Allocate();

			Instance NewInstance = fn((InstanceAbstraction::EditorMode == false ? 0x18 : 0x24));
			thisCall<void>(kTESModel_Ctor(), NewInstance);

			if (OutPath)
				*OutPath = GetPath(NewInstance);

			return NewInstance;
		}

		void DeleteInstance( Instance Model )
		{
			typedef Instance (* _fn)(Instance Model);
			const _fn fn = (_fn)kFormHeap_Free();

			thisCall<void>(kTESModel_Dtor(), Model);
			fn(Model);
		}

		BSString* GetPath( Instance Model )
		{
			return (BSString*)((UInt32)Model + 0x4);
		}
	}

	namespace TESTexture
	{
		Instance CreateInstance( BSString** OutPath )
		{
			typedef Instance (* _fn)(UInt32 Size);
			const _fn fn = (_fn)kFormHeap_Allocate();

			Instance NewInstance = fn((InstanceAbstraction::EditorMode == false ? 0xC : 0x18));
			thisCall<void>(kTESTexture_Ctor(), NewInstance);

			if (OutPath)
				*OutPath = GetPath(NewInstance);

			return NewInstance;
		}

		void DeleteInstance( Instance Texture )
		{
			typedef Instance (* _fn)(Instance Texture);
			const _fn fn = (_fn)kFormHeap_Free();

			thisCall<void>(kTESTexture_Dtor(), Texture);
			fn(Texture);
		}

		BSString* GetPath( Instance Texture )
		{
			return (BSString*)((UInt32)Texture + 0x4);
		}
	}

	namespace FileFinder
	{
		bool GetFileExists( const char* Path )
		{
			::FileFinder** Singleton = (::FileFinder**)kFileFinder_Singleton();

			return (*Singleton)->FindFile(Path, 0, 0, -1) != ::FileFinder::kFileStatus_NotFound;
		}
	}

	void BSString::Set( const char* String )
	{
		if (InstanceAbstraction::EditorMode)
			thisCall<bool>(0x004051E0, this, String, 0);
		else
			((::String*)this)->Set(String);
	}

}

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

bool SwapFaceGenHeadData(UInt8 Swap, FaceGenHeadParameters* Params, InstanceAbstraction::BSString* OldPath, InstanceAbstraction::BSString* NewPath)
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
		if (kGenderVariantHeadMeshes.GetData().i == 0)
			return false;

		Extension = ".nif";
		BasePath = "Meshes\\";
		break;
	case kSwap_HeadTexture:
		if (kGenderVariantHeadTextures.GetData().i == 0)
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

void __stdcall DoTESRaceGetFaceGenHeadParametersHook(TESRace* Race, FaceGenHeadParameters* FaceGenParams, TESNPC* NPC)
{
	// call original function to get the parameters
	thisCall<void>(InstanceAbstraction::kTESRace_GetFaceGenHeadParameters(), Race, NPC, FaceGenParams);

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
#ifndef NDEBUG
		_MESSAGE("Gadzooks! Why the heck is the TESModel/TESTexture BaseFormComponent NULL?!");
#endif
		// if the head model/texture should be NULL for whatever reason, slinky away
	}
	else
	{
		InstanceAbstraction::TESModel::GetPath(NewHeadModel)->Set(InstanceAbstraction::TESModel::GetPath(ExistingHeadModel)->m_data);
		InstanceAbstraction::TESTexture::GetPath(NewHeadTexture)->Set(InstanceAbstraction::TESTexture::GetPath(ExistingHeadTexture)->m_data);
		
		if (NPC)
		{
#ifndef NDEBUG
			if (NPC->refID == 0x7)
				_MESSAGE("Generating FaceGen head for the player character...");
			else
				_MESSAGE("Generating FaceGen head for NPC %08X...", NPC->refID);
			
			gLog.Indent();
#endif
			// check gender and append the appropriate postfix
			// if there's no asset at the new path, revert to the old one
			SwapFaceGenHeadData(kSwap_HeadModel, FaceGenParams,
								InstanceAbstraction::TESModel::GetPath(ExistingHeadModel),
								InstanceAbstraction::TESModel::GetPath(NewHeadModel));

			SwapFaceGenHeadData(kSwap_HeadTexture, FaceGenParams,
								InstanceAbstraction::TESTexture::GetPath(ExistingHeadTexture),
								InstanceAbstraction::TESTexture::GetPath(NewHeadTexture));

#ifndef NDEBUG
			gLog.Outdent();
#endif
		}
	}

	// finally swap the pointers, which will be released in the subsequent call to the facegen parameter object's dtor
	FaceGenParams->models.data[FaceGenHeadParameters::kFaceGenData_Head] = (::TESModel*)NewHeadModel;
	FaceGenParams->textures.data[FaceGenHeadParameters::kFaceGenData_Head] = (::TESTexture*)NewHeadTexture;
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

void BlockHeads( void )
{
	struct PatchSite
	{
		InstanceAbstraction::MemAddr	TESRaceGetFaceGenHeadParameters;
		InstanceAbstraction::MemAddr	FaceGenHeadParametersDtor;

		PatchSite(UInt32 GameA, UInt32 EditorA, UInt32 GameB, UInt32 EditorB)
		{
			TESRaceGetFaceGenHeadParameters.Game = GameA;
			TESRaceGetFaceGenHeadParameters.Editor = EditorA;

			FaceGenHeadParametersDtor.Game = GameB;
			FaceGenHeadParametersDtor.Editor = EditorB;
		}
	};

	std::vector<PatchSite> HookLocations;		// 7 in-game and 3 in-editor

	HookLocations.push_back(PatchSite(0x00528BF5, 0x004D9693, 0x00528C17, 0x004D9785));
	HookLocations.push_back(PatchSite(0x00529301, 0x004DA46B, 0x00529356, 0x004DA48D));
	HookLocations.push_back(PatchSite(0x0052966E, 0x004E739B, 0x00529702, 0x004E7405));
	HookLocations.push_back(PatchSite(0x0052E03B, 0, 0x0052E0A5, 0));
	HookLocations.push_back(PatchSite(0x005C7720, 0, 0x005C777A, 0));
	HookLocations.push_back(PatchSite(0x005C7AC1, 0, 0x005C7B1B, 0));
	HookLocations.push_back(PatchSite(0x005C936F, 0, 0x005C93C8, 0));

	for (int i = 0; i < HookLocations.size(); i++)
	{
		PatchSite& Site = HookLocations[i];
		
		_DefineCallHdlr(PatchHookA, Site.TESRaceGetFaceGenHeadParameters(), TESRaceGetFaceGenHeadParametersHook);
		_DefineCallHdlr(PatchHookB, Site.FaceGenHeadParametersDtor(), FaceGenHeadParametersDtorHook);

		_MemHdlr(PatchHookA).WriteCall();
		_MemHdlr(PatchHookB).WriteCall();
	}

	if (kAllowESPFacegenTextureUse.GetData().i)	
	{
		const InstanceAbstraction::MemAddr	kUseFaceGenHeadTextures = { 0x00524187, 0x004D5E27 };

		_DefineNopHdlr(PatchHook, kUseFaceGenHeadTextures(), 6);
		_MemHdlr(PatchHook).WriteNop();
	}
}
