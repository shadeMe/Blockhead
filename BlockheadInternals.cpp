#include "BlockheadInternals.h"

IDebugLog					gLog("Blockhead.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSEIOInterface*			g_OBSEIOIntfc = NULL;

BlockheadINIManager			BlockheadINIManager::Instance;

SME::INI::INISetting		kGenderVariantHeadMeshes("GenderVariantHeadMeshes", "General",
													"Use different head models for male and female NPCs", (SInt32)1);

SME::INI::INISetting		kGenderVariantHeadTextures("GenderVariantHeadTextures", "General",
													   "Use different head textures for male and female NPCs", (SInt32)1);

SME::INI::INISetting		kAllowESPFacegenTextureUse("AllowESPFacegenTextureUse", "General",
													"Use editor-generated facegen textures for ESP files", (SInt32)0);


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

SME::INI::INISetting		kOverrideUpperBodyTexture("OverrideUpperBodyTexture", "BodyTextureOverride", "Per-NPC body texture override", (SInt32)0);
SME::INI::INISetting		kOverrideLowerBodyTexture("OverrideLowerBodyTexture", "BodyTextureOverride", "Per-NPC body texture override", (SInt32)0);
SME::INI::INISetting		kOverrideHandTexture("OverrideHandTexture", "BodyTextureOverride", "Per-NPC body texture override", (SInt32)0);
SME::INI::INISetting		kOverrideFootTexture("OverrideFootTexture", "BodyTextureOverride", "Per-NPC body texture override", (SInt32)0);
SME::INI::INISetting		kOverrideTailTexture("OverrideTailTexture", "BodyTextureOverride", "Per-NPC body texture override", (SInt32)0);


_DefineHookHdlr(RaceSexMenuPoser, 0x0040D658);
_DefineJumpHdlr(RaceSexMenuRender, 0x005CE629, 0x005CE650);
_DefineHookHdlr(PlayerInventory3DAnimSequenceQueue, 0x0066951A);
_DefineHookHdlr(TESRaceGetBodyTexture, 0x0052D4C9);

namespace InstanceAbstraction
{
	bool EditorMode = false;

	const MemAddr kTESRace_GetFaceGenHeadParameters		= { 0x0052CD50, 0x004E6AA0 };
	const MemAddr kBSFaceGen_DoSomethingWithFaceGenNode	= { 0x005551C0, 0x00587AE0 };

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

	RegisterSetting(&kRaceMenuPoserEnabled);
	RegisterSetting(&kRaceMenuPoserMovementSpeed);
	RegisterSetting(&kRaceMenuPoserRotationSpeed);

	RegisterSetting(&kInventoryIdleOverrideEnabled);
	RegisterSetting(&kInventoryIdleOverridePath_Idle);
	RegisterSetting(&kInventoryIdleOverridePath_HandToHandIdle);
	RegisterSetting(&kInventoryIdleOverridePath_HandToHandTorchIdle);
	RegisterSetting(&kInventoryIdleOverridePath_OneHandIdle);
	RegisterSetting(&kInventoryIdleOverridePath_OneHandTorchIdle);
	RegisterSetting(&kInventoryIdleOverridePath_TwoHandIdle);
	RegisterSetting(&kInventoryIdleOverridePath_StaffIdle);
	RegisterSetting(&kInventoryIdleOverridePath_BowIdle);

	RegisterSetting(&kOverrideUpperBodyTexture);
	RegisterSetting(&kOverrideLowerBodyTexture);
	RegisterSetting(&kOverrideHandTexture);
	RegisterSetting(&kOverrideFootTexture);
	RegisterSetting(&kOverrideTailTexture);


	if (CreateINI)
		Save();
}

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

struct OverrideTextureData
{
	InstanceAbstraction::TESTexture::Instance		Original;
	bool											HasOverride;			// set to true when the texture path has been modified by us
};

typedef std::map<InstanceAbstraction::TESTexture::Instance, OverrideTextureData> OverrideTextureMapT;
static OverrideTextureMapT g_OverriddenHeadTextures;

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
		OverrideTextureData OverrideTexData;
		OverrideTexData.Original = ExistingHeadTexture;
		OverrideTexData.HasOverride = FixupFaceGenHeadAssetPath(kSwap_HeadTexture, FaceGenParams,
														InstanceAbstraction::TESTexture::GetPath(ExistingHeadTexture),
														InstanceAbstraction::TESTexture::GetPath(NewHeadTexture));

		g_OverriddenHeadTextures[NewHeadTexture] = OverrideTexData;

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
		if (g_OverriddenHeadTextures.count(SneakyBugger))
		{
			g_OverriddenHeadTextures.erase(SneakyBugger);
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

void NiMatrix33_Multiply(NiMatrix33* LHS, NiMatrix33* RHS, NiMatrix33* OutResult = NULL)
{
	NiMatrix33 Buffer = {0};
	thisCall<NiMatrix33*>(0x007100A0, LHS, &Buffer, RHS);

	if (OutResult == NULL)
		memcpy(LHS, &Buffer, sizeof(NiMatrix33));
	else
		memcpy(OutResult, &Buffer, sizeof(NiMatrix33));
}

void NiMatrix33_DebugDump(NiMatrix33* Matrix, const char* Name)
{
	_MESSAGE("NiMatrix33 %s Dump:", Name);
	gLog.Indent();

	char Buffer[0x100] = {0};

	for (int i = 0; i < 9; i += 3)
	{
		FORMAT_STR(Buffer, "(%0.3f       %0.3f       %0.3f)", Matrix->data[i], Matrix->data[i + 1], Matrix->data[i + 2]);
		_MESSAGE(Buffer);
	}

	gLog.Outdent();
	_MESSAGE("\n");
}

void __stdcall PoseFace(void)
{
	if (InterfaceManager::GetSingleton())
	{
		if (InterfaceManager::GetSingleton()->MenuModeHasFocus(kMenuType_RaceSex) == false)
			return;
		else if (IsConsoleOpen())
			return;

		bool UpArrowDown = g_OBSEIOIntfc->IsKeyPressed(0xC8) || g_OBSEIOIntfc->IsKeyPressed(0x11);
		bool DownArrowDown = g_OBSEIOIntfc->IsKeyPressed(0xD0) || g_OBSEIOIntfc->IsKeyPressed(0x1F);
		bool LeftArrowDown = g_OBSEIOIntfc->IsKeyPressed(0xCB) || g_OBSEIOIntfc->IsKeyPressed(0x1E);
		bool RightArrowDown = g_OBSEIOIntfc->IsKeyPressed(0xCD) || g_OBSEIOIntfc->IsKeyPressed(0x20);
		bool ShiftKeyDown = g_OBSEIOIntfc->IsKeyPressed(0x2A) || g_OBSEIOIntfc->IsKeyPressed(0x36);
		bool TabKeyDown = g_OBSEIOIntfc->IsKeyPressed(0x0F);
		
		if (UpArrowDown == false && DownArrowDown == false &&
			ShiftKeyDown == false && TabKeyDown == false &&
			LeftArrowDown == false && RightArrowDown == false)
		{
			return;
		}

		OSInputGlobals* InputManager = (*g_osGlobals)->input;

		SME_ASSERT((*g_worldSceneGraph)->m_children.numObjs > 0);

		NiNode* WorldCameraRoot = (NiNode*)((SceneGraph*)(*g_worldSceneGraph))->m_children.data[0];
		NiCamera* WorldCamera = ((SceneGraph*)(*g_worldSceneGraph))->camera;

		NiMatrix33* CameraRootWorldRotate = &WorldCameraRoot->m_worldRotate;
		NiMatrix33* CameraRootLocalRotate = &WorldCameraRoot->m_localRotate;

		Vector3* CameraRootWorldTranslate = (Vector3*)&WorldCameraRoot->m_worldTranslate;
		Vector3* CameraRootLocalTranslate = (Vector3*)&WorldCameraRoot->m_localTranslate;

		if (UpArrowDown || DownArrowDown)
		{
			float MovementMultiplier = kRaceMenuPoserMovementSpeed.GetData().f;
			if (DownArrowDown)
				MovementMultiplier *= -1;

			Vector3 Offset(CameraRootWorldRotate->data[1], CameraRootWorldRotate->data[4], CameraRootWorldRotate->data[7]);
			Offset.Scale(MovementMultiplier);

			*CameraRootWorldTranslate += Offset;
			*CameraRootLocalTranslate += Offset;
		}
		
		if (LeftArrowDown || RightArrowDown)
		{
			float MovementMultiplier = kRaceMenuPoserMovementSpeed.GetData().f;
			if (LeftArrowDown)
				MovementMultiplier *= -1;

			Vector3 Offset(CameraRootWorldRotate->data[0], CameraRootWorldRotate->data[3], CameraRootWorldRotate->data[6]);
			Offset.Scale(MovementMultiplier);

			*CameraRootWorldTranslate += Offset;
			*CameraRootLocalTranslate += Offset;
		}

		if (ShiftKeyDown)
		{
			float RotationMultiplier = kRaceMenuPoserRotationSpeed.GetData().f;
			DIMOUSESTATE2* MouseState = &InputManager->unk1B20.mouseState;
			
			if (MouseState->lX || MouseState->lY)
			{
				NiMatrix33 Buffer = {0}, MulResult = {0};

				float XAlpha = (MouseState->lX / 100.0f * RotationMultiplier * 0.5) * 1.0f;
				float YAlpha = (MouseState->lY / 100.0f * RotationMultiplier * 0.5) * 1.0f;

				thisCall<void>(0x0070FDD0, &Buffer, XAlpha);		// initialize rotation transform matrices
				NiMatrix33_Multiply(&Buffer, CameraRootWorldRotate, &MulResult);

				thisCall<void>(0x0070FD30, &Buffer, YAlpha);
				NiMatrix33_Multiply(&MulResult, &Buffer);

				memcpy(CameraRootLocalRotate, &MulResult, sizeof(NiMatrix33));
			}
		}	

		thisCall<void>(0x00707370, WorldCameraRoot, 0.0, 1);		// traverse and update
	}
}

#define _hhName		RaceSexMenuPoser
_hhBegin()
{
	_hhSetVar(Retn, 0x0040D65D);
	_hhSetVar(Call, 0x0040C830);
	__asm
	{
		pushad
		call	PoseFace
		popad

		call	_hhGetVar(Call)
		jmp		_hhGetVar(Retn)
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
	
	if (g_OverriddenHeadTextures.count(OverriddenTexture))
	{
		OverrideTextureData& Data = g_OverriddenHeadTextures[OverriddenTexture];

		if (Data.HasOverride)
		{
			// the base head texture path has been overridden by us, use the original base path instead
			// age textures are already gender variant so we need to undo our damage			
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

enum
{
	kInventoryIdle_Idle		= 0,
	kInventoryIdle_H2H,
	kInventoryIdle_H2HTorch,
	kInventoryIdle_1H,
	kInventoryIdle_1HTorch,
	kInventoryIdle_2H,
	kInventoryIdle_2HTorch,
	kInventoryIdle_Staff,
	kInventoryIdle_StaffTorch,
	kInventoryIdle_Bow,
	kInventoryIdle_BowTorch,

	kInventoryIdle__MAX
};

void __stdcall OverrideInventoryIdles(void* AnimationSeqHolder, tList<char>* Idles, NiNode* AnimNode, TESObjectREFR* AnimRef)
{
	static const SME::INI::INISetting*			kOverrideNames[kInventoryIdle__MAX] =
	{
		&kInventoryIdleOverridePath_Idle,
		&kInventoryIdleOverridePath_HandToHandIdle,
		&kInventoryIdleOverridePath_HandToHandTorchIdle,
		&kInventoryIdleOverridePath_OneHandIdle,
		&kInventoryIdleOverridePath_OneHandTorchIdle,
		&kInventoryIdleOverridePath_TwoHandIdle,
		NULL,
		&kInventoryIdleOverridePath_StaffIdle,
		NULL,
		&kInventoryIdleOverridePath_BowIdle,
		NULL
	};

	static const char*		kInventoryIdleNames[kInventoryIdle__MAX] = 
	{
		"Idle.kf",
		"HandToHandIdle.kf",
		"HandToHandTorchIdle.kf",
		"OneHandIdle.kf",
		"OneHandTorchIdle.kf",
		"TwoHandIdle.kf",
		"TwoHandTorchIdle.kf",
		"StaffIdle.kf",
		"StaffTorchIdle.kf",
		"BowIdle.kf",
		"BowTorchIdle.kf"
	};

#ifndef NDEBUG
	_MESSAGE("Overriding inventory idles...");
	gLog.Indent();
#endif

	int IdleNameCounter = 0;
	for (tList<char>::Iterator Itr = Idles->Begin(); Itr.End() == false && Itr.Get(); ++Itr)
	{
		std::string FileName = strrchr(Itr.Get(), '\\') + 1;
		
		bool ValidIdle = false;
		while (IdleNameCounter < kInventoryIdle__MAX)
		{
			const char* Current = kInventoryIdleNames[IdleNameCounter];
			IdleNameCounter++;
			
			if (!_stricmp(FileName.c_str(), Current))
			{
				ValidIdle = true;
				break;
			}
		}

		if (ValidIdle == false)
		{
			SME_ASSERT((++Itr).End() == true);		// the current idle node has to be the last
			break;									// since they are added in sequence
		}

		const SME::INI::INISetting* OverrideName = kOverrideNames[IdleNameCounter - 1];
		if (OverrideName == NULL || strlen(OverrideName->GetData().s) < 4)
			continue;								// skip to the next idle path for those without overrides

#ifndef NDEBUG
		_MESSAGE("Attempting override for idle %s...", kInventoryIdleNames[IdleNameCounter - 1]);
		gLog.Indent();
#endif // !NDEBUG

		std::string Path = "Meshes\\" + std::string(Itr.Get());
		Path.erase(Path.rfind("\\") + 1);
		Path += std::string(OverrideName->GetData().s);

		if (InstanceAbstraction::FileFinder::GetFileExists(Path.c_str()))
		{
#ifndef NDEBUG
			_MESSAGE("Idle path %s switched to %s", Itr.Get(), Path.c_str());
#endif // !NDEBUG

			sprintf_s(Itr.Get(), 0x104, "%s", Path.c_str());
		}
		else
		{
#ifndef NDEBUG
			_MESSAGE("Override path %s invalid", Path.c_str());
#endif // !NEDEBUG
		}

#ifndef NDEBUG
		gLog.Outdent();
#endif // !NDEBUG
	}

#ifndef NDEBUG
	gLog.Outdent();
#endif // !NDEBUG


	thisCall<void>(0x00475D80, AnimationSeqHolder, Idles, AnimNode, AnimRef);
}

#define _hhName		PlayerInventory3DAnimSequenceQueue
_hhBegin()
{
	_hhSetVar(Retn, 0x0066951F);
	__asm
	{
		push	ecx
		call	OverrideInventoryIdles

		jmp		_hhGetVar(Retn)
	}
}

enum
{
	kRaceBodyTextureSkin_UpperBody	= 2,		// same for arms
	kRaceBodyTextureSkin_LowerBody	= 3,
	kRaceBodyTextureSkin_Hand		= 4,
	kRaceBodyTextureSkin_Foot		= 5,
	kRaceBodyTextureSkin_Tail		= 15,
};

static const char* kOverrideBodyTextureRootPath = "Characters\\BodyTextureOverrides";

void __cdecl SwapRaceBodyTexture(UInt8 SkinID, TESNPC* NPC, InstanceAbstraction::BSString* OutTexPath, const char* Format, const char* OrgTexPath)
{
	SME::INI::INISetting* OverrideSetting = NULL;
	const char* PathSuffix = NULL;
	SME_ASSERT(NPC && SkinID);

	char OverrideTexPath[MAX_PATH] = {0};
	FORMAT_STR(OverrideTexPath, "Textures\\%s", OrgTexPath);
	OutTexPath->Set(OverrideTexPath);

	switch (SkinID)
	{
	case kRaceBodyTextureSkin_UpperBody:
		OverrideSetting = &kOverrideUpperBodyTexture;
		PathSuffix = "UpperBody";
		break;
	case kRaceBodyTextureSkin_LowerBody:
		OverrideSetting = &kOverrideLowerBodyTexture;
		PathSuffix = "LowerBody";
		break;
	case kRaceBodyTextureSkin_Hand:
		OverrideSetting = &kOverrideHandTexture;
		PathSuffix = "Hand";
		break;
	case kRaceBodyTextureSkin_Foot:
		OverrideSetting = &kOverrideFootTexture;
		PathSuffix = "Foot";
		break;
	case kRaceBodyTextureSkin_Tail:
		OverrideSetting = &kOverrideTailTexture;
		PathSuffix = "Tail";
		break;
	default:
#ifndef NDEBUG
		_MESSAGE("Unknown biped skinID %d for NPC %08X", SkinID, NPC->refID);
#endif // !NDEBUG
		return;
	}

	if (OverrideSetting->GetData().i == 0)
		return;

	UInt32 FormID = NPC->refID & 0x00FFFFFF;

	FORMAT_STR(OverrideTexPath, "Textures\\%s\\%08X_%s.dds", kOverrideBodyTextureRootPath, FormID, PathSuffix);
	if (InstanceAbstraction::FileFinder::GetFileExists(OverrideTexPath))
	{
#ifndef NDEBUG
		_MESSAGE("Switching %s texture for NPC %08X from %s to %s", PathSuffix, NPC->refID, OrgTexPath, OverrideTexPath);
#endif // !NDEBUG

		OutTexPath->Set(OverrideTexPath);
	}
}

#define _hhName		TESRaceGetBodyTexture
_hhBegin()
{
	_hhSetVar(Retn, 0x0052D4CE);
	__asm
	{
		push	[ebp - 0x20]
		push	[ebp - 0x1C]
		call	SwapRaceBodyTexture
		add		esp, 0x8

		jmp		_hhGetVar(Retn)
	}
}

void BlockHeads( void )
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

	if (kAllowESPFacegenTextureUse.GetData().i)	
	{
		const InstanceAbstraction::MemAddr	kUseFaceGenHeadTextures = { 0x00524187, 0x004D5E27 };

		_DefineNopHdlr(PatchHook, kUseFaceGenHeadTextures(), 6);
		_MemHdlr(PatchHook).WriteNop();
	}

	if (InstanceAbstraction::EditorMode == false && kRaceMenuPoserEnabled.GetData().i)
	{
		_MemHdlr(RaceSexMenuPoser).WriteJump();
		_MemHdlr(RaceSexMenuRender).WriteJump();
	}

	if (InstanceAbstraction::EditorMode == false && kInventoryIdleOverrideEnabled.GetData().i)
	{
		_MemHdlr(PlayerInventory3DAnimSequenceQueue).WriteJump();
	}

	if (InstanceAbstraction::EditorMode == false)
	{
		_MemHdlr(TESRaceGetBodyTexture).WriteJump();
	}
}
