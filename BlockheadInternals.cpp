#include "BlockheadInternals.h"


namespace Interfaces
{
	PluginHandle					kOBSEPluginHandle = kPluginHandle_Invalid;

	OBSEScriptInterface*			kOBSEScript = NULL;
	OBSEArrayVarInterface*			kOBSEArrayVar = NULL;
	OBSESerializationInterface*		kOBSESerialization = NULL;
	OBSEStringVarInterface*			kOBSEStringVar = NULL;
	OBSEMessagingInterface*			kOBSEMessaging = NULL;
	OBSEIOInterface*				kOBSEIO = NULL;
	CSEIntelliSenseInterface*		kCSEIntelliSense = NULL;
	CSEConsoleInterface*			kCSEConsole = NULL;
}

BlockheadINIManager			BlockheadINIManager::Instance;

namespace Settings
{
	SME::INI::INISetting		kGenderVariantHeadMeshes("GenderVariantHeadMeshes", "General",
														"Use different head models for male and female NPCs", (SInt32)1);

	SME::INI::INISetting		kGenderVariantHeadTextures("GenderVariantHeadTextures", "General",
														   "Use different head textures for male and female NPCs", (SInt32)1);

	SME::INI::INISetting		kRaceMenuPoserEnabled("Enabled", "RaceMenuPoser",
														   "Allow unrestricted camera movement in the RaceSex menu", (SInt32)1);

	SME::INI::INISetting		kRaceMenuPoserMovementSpeed("MovementSpeed", "RaceMenuPoser",
													  "Camera movement speed in the RaceSex menu", 1.5f);

	SME::INI::INISetting		kRaceMenuPoserRotationSpeed("RotationSpeed", "RaceMenuPoser",
													  "Camera rotation speed in the RaceSex menu", 2.0f);

	SME::INI::INISetting		kInventoryIdleOverrideEnabled("Enabled", "InventoryIdleOverride",
															  "Override the animations used in the inventory screen", (SInt32)0);

	SME::INI::INISetting		kInventoryIdleOverridePath_Idle("Idle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_HandToHandIdle("HandToHandIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_HandToHandTorchIdle("HandToHandTorchIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_OneHandIdle("OneHandIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_OneHandTorchIdle("OneHandTorchIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_TwoHandIdle("TwoHandIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_StaffIdle("StaffIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_BowIdle("BowIdle", "InventoryIdleOverride", "Idle replacer", "");

	SME::INI::INISetting		kOverrideTexturePerNPC("OverrideTexturePerNPC", "BodyOverride", "Per-NPC body texture override", (SInt32)1);
	SME::INI::INISetting		kOverrideTexturePerRace("OverrideTexturePerRace", "BodyOverride", "Per-Race body texture override", (SInt32)1);
	SME::INI::INISetting		kOverrideModelPerNPC("OverrideModelPerNPC", "BodyOverride", "Per-NPC body model override", (SInt32)1);
	SME::INI::INISetting		kOverrideModelPerRace("OverrideModelPerRace", "BodyOverride", "Per-Race body model override", (SInt32)1);
}



namespace InstanceAbstraction
{
	bool EditorMode = false;

	const MemAddr kTESRace_GetFaceGenHeadParameters		= { 0x0052CD50, 0x004E6AA0 };	// f(TESRace*, TESNPC*, FaceGenHeadParameters*)
	const MemAddr kBSFaceGen_DoSomethingWithFaceGenNode	= { 0x005551C0, 0x00587AE0 };	// f(BSFaceGenNiNode*, FaceGenHeadParameters*)
	const MemAddr kBSFaceGen_GetAge						= { 0x00553B30, 0x00586FB0 };	// f(FaceGenHeadParameters::Unk18*, 0, 0)
	const MemAddr kTESNPC_SetFaceGenAge					= { 0x00527860, 0x0 };			// f(TESNPC*, int)

	const MemAddr kFormHeap_Allocate				= { 0x00401F00, 0x00401E80 };
	const MemAddr kFormHeap_Free					= { 0x00401F20, 0x00401EA0 };
	
	const MemAddr kTESModel_Ctor					= { 0x0046D7E0, 0x0049D040 };
	const MemAddr kTESModel_Dtor					= { 0x0046D850, 0x0049CF40 };
	
	const MemAddr kTESTexture_Ctor					= { 0x0046FFD0, 0x004A3FF0 };
	const MemAddr kTESTexture_Dtor					= { 0x00470040, 0x004A4050 };

	const MemAddr kFaceGenHeadParameters_Ctor		= { 0x00527C90, 0x004D8DC0 };
	const MemAddr kFaceGenHeadParameters_Dtor		= { 0x00526CE0, 0x004D88C0 };
	
	const MemAddr kFileFinder_Singleton				= { 0x00B33A04, 0x00A0DE8C };

	namespace TESModel
	{
		Instance CreateInstance( BSString** OutPath )
		{
			Instance NewInstance = (Instance)FormHeap_Allocate((InstanceAbstraction::EditorMode == false ? 0x18 : 0x24));
			thisCall<void>(kTESModel_Ctor(), NewInstance);

			if (OutPath)
				*OutPath = GetPath(NewInstance);

			return NewInstance;
		}

		void DeleteInstance( Instance Model )
		{
			thisCall<void>(kTESModel_Dtor(), Model);
			FormHeap_Free(Model);
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
			Instance NewInstance = (Instance)FormHeap_Allocate((InstanceAbstraction::EditorMode == false ? 0xC : 0x18));
			thisCall<void>(kTESTexture_Ctor(), NewInstance);

			if (OutPath)
				*OutPath = GetPath(NewInstance);

			return NewInstance;
		}

		void DeleteInstance( Instance Texture )
		{
			thisCall<void>(kTESTexture_Dtor(), Texture);
			FormHeap_Free(Texture);
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
			((::BSStringT*)this)->Set(String);
	}

	float GetNPCFaceGenAge( TESNPC* NPC )
	{
		SME_ASSERT(NPC);

		FaceGenHeadParameters* Params = (FaceGenHeadParameters*)FormHeap_Allocate(sizeof(FaceGenHeadParameters));
		thisCall<void>(kFaceGenHeadParameters_Ctor(), Params);

		// this call will fail miserably when called in the editor (TESNPC definition mismatch)
		// [SamuelJohnson]but I care not![/SamuelJohnson]
		thisCall<void>(kTESRace_GetFaceGenHeadParameters(), NPC->race.race, NPC, Params);
		float Age = cdeclCall<float>(kBSFaceGen_GetAge(), Params, 0 , 0);

		FormHeap_Free(Params);
		return Age;
	}

	void SetNPCFaceGenAge( TESNPC* NPC, int Age )
	{
		SME_ASSERT(NPC);

		thisCall<void>(kTESNPC_SetFaceGenAge(), NPC, Age);
	}

	void* FormHeap_Allocate( UInt32 Size )
	{
		typedef void* (* _fn)(UInt32);
		const _fn fn = (_fn)kFormHeap_Allocate();

		return fn(Size);
	}

	void FormHeap_Free( void* Pointer )
	{
		typedef void (* _fn)(void*);
		const _fn fn = (_fn)kFormHeap_Free();

		return fn(Pointer);
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

	RegisterSetting(&Settings::kGenderVariantHeadMeshes);
	RegisterSetting(&Settings::kGenderVariantHeadTextures);

	RegisterSetting(&Settings::kRaceMenuPoserEnabled);
	RegisterSetting(&Settings::kRaceMenuPoserMovementSpeed);
	RegisterSetting(&Settings::kRaceMenuPoserRotationSpeed);

	RegisterSetting(&Settings::kInventoryIdleOverrideEnabled);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_Idle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_HandToHandIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_HandToHandTorchIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_OneHandIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_OneHandTorchIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_TwoHandIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_StaffIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_BowIdle);

	RegisterSetting(&Settings::kOverrideTexturePerNPC);
	RegisterSetting(&Settings::kOverrideTexturePerRace);
	RegisterSetting(&Settings::kOverrideModelPerNPC);
	RegisterSetting(&Settings::kOverrideModelPerRace);

	if (CreateINI)
		Save();
}
