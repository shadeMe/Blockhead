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

void BlockheadINIManager::Initialize( const char* INIPath, void* Parameter )
{
	this->INIFilePath = INIPath;
	_MESSAGE("INI Path: %s", INIPath);

	RegisterSetting(&Settings::kRaceMenuPoserEnabled);
	RegisterSetting(&Settings::kRaceMenuPoserMovementSpeed);
	RegisterSetting(&Settings::kRaceMenuPoserRotationSpeed);

	RegisterSetting(&Settings::kInventoryIdleOverrideEnabled);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_HandToHandIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_HandToHandTorchIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_OneHandIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_OneHandTorchIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_TwoHandIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_StaffIdle);
	RegisterSetting(&Settings::kInventoryIdleOverridePath_BowIdle);

	RegisterSetting(&Settings::kBodyOverrideTexturePerNPC);
	RegisterSetting(&Settings::kBodyOverrideTexturePerRace);
	RegisterSetting(&Settings::kBodyOverrideModelPerNPC);
	RegisterSetting(&Settings::kBodyOverrideModelPerRace);

	RegisterSetting(&Settings::kHeadOverrideTexturePerNPC);
	RegisterSetting(&Settings::kHeadOverrideTexturePerRace);
	RegisterSetting(&Settings::kHeadOverrideModelPerNPC);
	RegisterSetting(&Settings::kHeadOverrideModelPerRace);

	RegisterSetting(&Settings::kHeadOverrideHairGenderVariantModel);
	RegisterSetting(&Settings::kHeadOverrideHairGenderVariantTexture);

	RegisterSetting(&Settings::kAnimOverridePerNPC);
	RegisterSetting(&Settings::kAnimOverridePerRace);

	Save();
}

namespace Settings
{
	SME::INI::INISetting		kRaceMenuPoserEnabled("Enabled", "RaceMenuPoser",
														   "Allow unrestricted camera movement in the RaceSex menu", (SInt32)1);

	SME::INI::INISetting		kRaceMenuPoserMovementSpeed("MovementSpeed", "RaceMenuPoser",
													  "Camera movement speed in the RaceSex menu", 1.5f);

	SME::INI::INISetting		kRaceMenuPoserRotationSpeed("RotationSpeed", "RaceMenuPoser",
													  "Camera rotation speed in the RaceSex menu", 2.0f);

	SME::INI::INISetting		kInventoryIdleOverrideEnabled("Enabled", "InventoryIdleOverride",
															  "Override the animations used in the inventory screen", (SInt32)0);

	SME::INI::INISetting		kInventoryIdleOverridePath_HandToHandIdle("HandToHandIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_HandToHandTorchIdle("HandToHandTorchIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_OneHandIdle("OneHandIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_OneHandTorchIdle("OneHandTorchIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_TwoHandIdle("TwoHandIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_StaffIdle("StaffIdle", "InventoryIdleOverride", "Idle replacer", "");
	SME::INI::INISetting		kInventoryIdleOverridePath_BowIdle("BowIdle", "InventoryIdleOverride", "Idle replacer", "");

	SME::INI::INISetting		kBodyOverrideTexturePerNPC("OverrideTexturePerNPC", "BodyOverride", "Per-NPC body texture override", (SInt32)1);
	SME::INI::INISetting		kBodyOverrideTexturePerRace("OverrideTexturePerRace", "BodyOverride", "Per-Race body texture override", (SInt32)1);
	SME::INI::INISetting		kBodyOverrideModelPerNPC("OverrideModelPerNPC", "BodyOverride", "Per-NPC body model override", (SInt32)1);
	SME::INI::INISetting		kBodyOverrideModelPerRace("OverrideModelPerRace", "BodyOverride", "Per-Race body model override", (SInt32)1);

	SME::INI::INISetting		kHeadOverrideTexturePerNPC("OverrideTexturePerNPC", "HeadOverride", "Per-NPC head texture override", (SInt32)1);
	SME::INI::INISetting		kHeadOverrideTexturePerRace("OverrideTexturePerRace", "HeadOverride", "Per-Race head texture override", (SInt32)1);
	SME::INI::INISetting		kHeadOverrideModelPerNPC("OverrideModelPerNPC", "HeadOverride", "Per-NPC head model override", (SInt32)1);
	SME::INI::INISetting		kHeadOverrideModelPerRace("OverrideModelPerRace", "HeadOverride", "Per-Race head model override", (SInt32)1);

	SME::INI::INISetting		kHeadOverrideHairGenderVariantModel("GenderVariantHairModels", "HeadOverride", "Hair model override", (SInt32)1);
	SME::INI::INISetting		kHeadOverrideHairGenderVariantTexture("GenderVariantHairTextures", "HeadOverride", "Hair texture override", (SInt32)1);

	SME::INI::INISetting		kAnimOverridePerNPC("OverridePerNPC", "AnimationOverride", "Per-NPC animation override", (SInt32)1);
	SME::INI::INISetting		kAnimOverridePerRace("OverridePerRace", "AnimationOverride", "Per-Race animation override", (SInt32)1);
}

void FaceGenHeadParameters::DebugDump( void )
{
#ifndef NDEBUG
	_MESSAGE("HeadParams Dump:");
	gLog.Indent();

	_MESSAGE("Gender: %s", (female ? "F" : "M"));
	_MESSAGE("FaceGen Texturing: %d", useFaceGenTexturing);
	if (hair)
	{
		_MESSAGE("Hair:");
		gLog.Indent();
		_MESSAGE("Pointer = %08X", hair);
		_MESSAGE("ID = %08X", hair->refID);
		_MESSAGE("Color = %08X", hairColor);
		_MESSAGE("Length = %f", hairLength);
		gLog.Outdent();
	}

	if (eyes)
	{
		_MESSAGE("Eyes:");
		gLog.Indent();
		_MESSAGE("Pointer = %08X", eyes);
		_MESSAGE("ID = %08X", eyes->refID);
		if (eyeLeft)
			_MESSAGE("Left = %s", InstanceAbstraction::TESModel::GetPath(eyeLeft)->m_data);
		if (eyeRight)
			_MESSAGE("Right = %s", InstanceAbstraction::TESModel::GetPath(eyeRight)->m_data);
		gLog.Outdent();
	}

	_MESSAGE("Parts:");
	gLog.Indent();
	for (int i = FaceGenHeadParameters::kFaceGenData__BEGIN; i < FaceGenHeadParameters::kFaceGenData__END; i++)
	{
		switch (i)
		{
		case FaceGenHeadParameters::kFaceGenData_Head:
			_MESSAGE("Head >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_EarsMale:
			_MESSAGE("EarsMale >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_EarsFemale:
			_MESSAGE("EarsFemale >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_Mouth:
			_MESSAGE("Mouth >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_TeethLower:
			_MESSAGE("TeethLower >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_TeethUpper:
			_MESSAGE("TeethUpper >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_Tongue:
			_MESSAGE("Tongue >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_EyesLeft:
			_MESSAGE("EyesLeft >>");
			break;
		case FaceGenHeadParameters::kFaceGenData_EyesRight:
			_MESSAGE("EyesRight >>");
			break;
		}

		InstanceAbstraction::TESModel::Instance ThisModel = (InstanceAbstraction::TESModel::Instance)models.data[i];
		InstanceAbstraction::TESTexture::Instance ThisTexture = (InstanceAbstraction::TESTexture::Instance)textures.data[i];
		gLog.Indent();
		std::string Buffer("Model = ");
		const char* Path = NULL;

		if (ThisModel == NULL)
			Buffer += "NULL";
		else if ((Path = InstanceAbstraction::TESModel::GetPath(ThisModel)->m_data) == NULL)
			Buffer += "";
		else
			Buffer += std::string(Path);
		_MESSAGE("%s", Buffer.c_str());
		Buffer = "Texture = ";
		if (ThisTexture == NULL)
			Buffer += "NULL";
		else if ((Path = InstanceAbstraction::TESTexture::GetPath(ThisTexture)->m_data) == NULL)
			Buffer += "";
		else
			Buffer += std::string(Path);
		_MESSAGE("%s", Buffer.c_str());
		gLog.Outdent();
	}
	gLog.Outdent();

	gLog.Outdent();
#endif // !NDEBUG
}

ScopedLock::ScopedLock( ICriticalSection& Lock ) :
	Lock(Lock)
{
	Lock.Enter();
}

ScopedLock::~ScopedLock()
{
	Lock.Leave();
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

	const MemAddr kTESForm_GetOverrideFile			= { 0x0046B680, 0x00495FE0 };

	const MemAddr kTESHair_Ctor						= { 0x005200F0, 0x004D1340 };
	const MemAddr kTESHair_Dtor						= { 0x00520030, 0x004D1280 };

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

	namespace TESHair
	{
		Instance CreateInstance()
		{
			Instance NewInstance = (Instance)FormHeap_Allocate((InstanceAbstraction::EditorMode == false ? 0x4C : 0x70));
			thisCall<void>(kTESHair_Ctor(), NewInstance);

			return NewInstance;
		}

		void DeleteInstance(Instance Hair)
		{
			thisCall<void>(kTESHair_Dtor(), Hair);
			FormHeap_Free(Hair);
		}

		TESModel::Instance GetModel(Instance Hair)
		{
			if (EditorMode == false)
				return (TESModel::Instance)&((::TESHair*)Hair)->model;
			else
				return (TESModel::Instance)((UInt32)Hair + 0x30);
		}

		TESTexture::Instance GetTexture(Instance Hair)
		{
			if (EditorMode == false)
				return (TESModel::Instance)&((::TESHair*)Hair)->texture;
			else
				return (TESModel::Instance)((UInt32)Hair + 0x54);
		}

		void CopyFlags(Instance From, Instance To)
		{
			if (EditorMode == false)
				((::TESHair*)To)->::TESHair::flags = ((::TESHair*)From)->::TESHair::flags;
			else
				*((UInt8*)((UInt32)To + 0x6C)) = *((UInt8*)((UInt32)From + 0x6C));
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

		thisCall<void>(kTESRace_GetFaceGenHeadParameters(), GetNPCRace(NPC), NPC, Params);
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

	TESRace* GetNPCRace( TESNPC* NPC )
	{
		SME_ASSERT(NPC);

		if (InstanceAbstraction::EditorMode)
			return *((TESRace**)((UInt32)NPC + 0x11C));
		else
			return NPC->race.race;
	}

	TESFile* GetOverrideFile( TESForm* Form, int Index /*= -1*/ )
	{
		return thisCall<TESFile*>(kTESForm_GetOverrideFile(), Form, Index);
	}

	bool GetNPCFemale( TESNPC* NPC )
	{
		if (InstanceAbstraction::EditorMode)
			return thisCall<SInt32>(0x004C8F50, NPC);
		else
			return NPC->actorBaseData.IsFemale();
	}

	const char* GetFormName( TESForm* Form )
	{
		if (InstanceAbstraction::EditorMode)
			return cdeclCall<const char*>(0x00414130, Form);
		else if (Form->GetFullName())
			return Form->GetFullName()->name.m_data;
		else
			return NULL;
	}
}