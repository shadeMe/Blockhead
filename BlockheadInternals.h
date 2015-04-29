#pragma once

#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse/GameAPI.h"
#include "obse/GameObjects.h"
#include "obse/GameData.h"
#include "obse/GameMenus.h"
#include "obse/GameOSDepend.h"
#include "obse/NiAPI.h"
#include "obse/NiObjects.h"
#include "obse/NiTypes.h"
#include "obse/ParamInfos.h"
#include "obse/GameTasks.h"

#include "common\ICriticalSection.h"
#include "common\IDirectoryIterator.h"
#include <boost\shared_ptr.hpp>

#include <SME_Prefix.h>
#include <MemoryHandler.h>
#include <INIManager.h>
#include <StringHelpers.h>
#include <MiscGunk.h>

#define CSEAPI_NO_CODA		1
#include "Construction Set Extender\CSEInterfaceAPI.h"

using namespace SME;
using namespace SME::MemoryHandler;

namespace Interfaces
{
	extern PluginHandle						kOBSEPluginHandle;

	extern OBSEScriptInterface*				kOBSEScript;
	extern OBSEArrayVarInterface*			kOBSEArrayVar;
	extern OBSESerializationInterface*		kOBSESerialization;
	extern OBSEStringVarInterface*			kOBSEStringVar;
	extern OBSEMessagingInterface*			kOBSEMessaging;
	extern OBSEIOInterface*					kOBSEIO;
	extern CSEIntelliSenseInterface*		kCSEIntelliSense;
	extern CSEConsoleInterface*				kCSEConsole;
}

class RaceSexMenu;

class BlockheadINIManager : public INI::INIManager
{
public:
	void								Initialize(const char* INIPath, void* Parameter);

	static BlockheadINIManager			Instance;
};

namespace Settings
{
	extern SME::INI::INISetting				kRaceMenuPoserEnabled;
	extern SME::INI::INISetting				kRaceMenuPoserMovementSpeed;
	extern SME::INI::INISetting				kRaceMenuPoserRotationSpeed;

	extern SME::INI::INISetting				kInventoryIdleOverrideEnabled;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_HandToHandIdle;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_HandToHandTorchIdle;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_OneHandIdle;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_OneHandTorchIdle;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_TwoHandIdle;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_StaffIdle;
	extern SME::INI::INISetting				kInventoryIdleOverridePath_BowIdle;

	extern SME::INI::INISetting				kBodyOverrideTexturePerNPC;
	extern SME::INI::INISetting				kBodyOverrideTexturePerRace;
	extern SME::INI::INISetting				kBodyOverrideModelPerNPC;
	extern SME::INI::INISetting				kBodyOverrideModelPerRace;

	extern SME::INI::INISetting				kHeadOverrideTexturePerNPC;
	extern SME::INI::INISetting				kHeadOverrideTexturePerRace;
	extern SME::INI::INISetting				kHeadOverrideModelPerNPC;
	extern SME::INI::INISetting				kHeadOverrideModelPerRace;

	extern SME::INI::INISetting				kHeadOverrideHairGenderVariantModel;
	extern SME::INI::INISetting				kHeadOverrideHairGenderVariantTexture;

	extern SME::INI::INISetting				kAnimOverridePerNPC;
	extern SME::INI::INISetting				kAnimOverridePerRace;
}

class ScopedLock
{
	ICriticalSection&			Lock;
public:
	ScopedLock(ICriticalSection& Lock);
	~ScopedLock();
};

// C4+?
class FaceGenHeadParameters
{
public:
	// FaceGen data block, stores age, etc
	// 18
	struct UnkData18
	{
		UInt32			unk00;
		UInt32			unk04;
		UInt32			unk08[3];
		UInt32			unk14;
	};

	// act as indices into the arrays
	// kFaceGenData_EyesLeft/Right have empty TESTexture* (not always though?)/NiTexture* and their models are picked from explicit pointers, rather than the model array
	enum
	{
		kFaceGenData__BEGIN			= 0,

		kFaceGenData_Head			= 0,
		kFaceGenData_EarsMale,
		kFaceGenData_EarsFemale,
		kFaceGenData_Mouth,
		kFaceGenData_TeethLower,
		kFaceGenData_TeethUpper,
		kFaceGenData_Tongue,
		kFaceGenData_EyesLeft,
		kFaceGenData_EyesRight		= 8,

		kFaceGenData__END
	};

	UnkData18							headData[4];						// 00
	::TESHair*							hair;								// 60
	RGBA								hairColor;							// 64
	float								hairLength;							// 68
	TESEyes*							eyes;								// 6C
	UInt32								female;								// 70 - set to 1 if female
	NiTArray<::TESModel*>				models;								// 74
	NiTArray<::TESTexture*>				textures;							// 84
	NiTArray<const char*>				nodeNames;							// 94
	NiTArray<NiPointer<NiTexture>>		sourceTextures;						// A4 - NiSourceTexture*, populated with the editor-exported textures
	UInt8								useFaceGenTexturing;				// B4 - value of INI setting bFaceGenTexturing:General, if set, uses the source textures
	UInt8								padB5[3];
	::TESModel*							eyeLeft;							// B8 - no idea why they aren't using the model array, it gets populated with the eye models too
	::TESModel*							eyeRight;							// BC
	SInt32								unkC0;								// init to -1

	void								DebugDump(void);
};
STATIC_ASSERT(sizeof(FaceGenHeadParameters::UnkData18) == 0x18);
STATIC_ASSERT(sizeof(FaceGenHeadParameters) == 0xC4);

typedef ModEntry::Data					TESFile;

// not very pretty but better than having to switch b'ween 2 class definitions
namespace InstanceAbstraction
{
	extern bool				EditorMode;

	struct MemAddr
	{
		UInt32		Game;
		UInt32		Editor;

		UInt32		operator()() const
		{
			if (EditorMode)
				return Editor;
			else
				return Game;
		}
	};

	extern const MemAddr	kTESRace_GetFaceGenHeadParameters;
	extern const MemAddr	kBSFaceGen_DoSomethingWithFaceGenNode;
	extern const MemAddr	kBSFaceGen_GetAge;
	extern const MemAddr	kTESNPC_SetFaceGenAge;
	extern const MemAddr	kFormHeap_Allocate;
	extern const MemAddr	kFormHeap_Free;
	extern const MemAddr	kTESModel_Ctor;
	extern const MemAddr	kTESModel_Dtor;
	extern const MemAddr	kTESTexture_Ctor;
	extern const MemAddr	kTESTexture_Dtor;
	extern const MemAddr	kFaceGenHeadParameters_Ctor;
	extern const MemAddr	kFaceGenHeadParameters_Dtor;
	extern const MemAddr	kFileFinder_Singleton;
	extern const MemAddr	kTESForm_GetOverrideFile;

	class BSString
	{
	public:
		char*					m_data;
		UInt16					m_dataLen;
		UInt16					m_bufLen;

		void					Set(const char* String);
	};

	namespace TESModel
	{
		typedef void*			Instance;

		Instance				CreateInstance(BSString** OutPath = NULL);
		void					DeleteInstance(Instance Model);

		BSString*				GetPath(Instance Model);
	}

	namespace TESTexture
	{
		typedef void*			Instance;

		Instance				CreateInstance(BSString** OutPath = NULL);
		void					DeleteInstance(Instance Texture);

		BSString*				GetPath(Instance Texture);
	}

	namespace FileFinder
	{
		bool					GetFileExists(const char* Path);
	}

	namespace TESHair
	{
		typedef void*			Instance;

		Instance				CreateInstance();
		void					DeleteInstance(Instance Hair);

		void					CopyFlags(Instance From, Instance To);
		TESModel::Instance		GetModel(Instance Hair);
		TESTexture::Instance	GetTexture(Instance Hair);
	}

	void*			FormHeap_Allocate(UInt32 Size);
	void			FormHeap_Free(void* Pointer);

	float			GetNPCFaceGenAge(TESNPC* NPC);
	void			SetNPCFaceGenAge(TESNPC* NPC, int Age);

	TESRace*		GetNPCRace(TESNPC* NPC);
	bool			GetNPCFemale(TESNPC* NPC);

	const char*		GetFormName(TESForm* Form);

	TESFile*		GetOverrideFile(TESForm* Form, int Index = -1);
}
