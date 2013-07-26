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

#include "[Libraries]\SME Sundries\SME_Prefix.h"
#include "[Libraries]\SME Sundries\MemoryHandler.h"
#include "[Libraries]\SME Sundries\INIManager.h"
#include "[Libraries]\SME Sundries\StringHelpers.h"

using namespace SME;
using namespace SME::MemoryHandler;

extern IDebugLog					gLog;
extern PluginHandle					g_pluginHandle;
extern OBSEIOInterface*				g_OBSEIOIntfc;

class RaceSexMenu;

class BlockheadINIManager : public INI::INIManager
{
public:
	void								Initialize(const char* INIPath, void* Parameter);

	static BlockheadINIManager			Instance;
};

extern SME::INI::INISetting				kGenderVariantHeadMeshes;
extern SME::INI::INISetting				kGenderVariantHeadTextures;
extern SME::INI::INISetting				kAllowESPFacegenTextureUse;

extern SME::INI::INISetting				kRaceMenuPoserEnabled;
extern SME::INI::INISetting				kRaceMenuPoserMovementSpeed;
extern SME::INI::INISetting				kRaceMenuPoserRotationSpeed;

extern SME::INI::INISetting				kInventoryIdleOverrideEnabled;
extern SME::INI::INISetting				kInventoryIdleOverridePath_Idle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_HandToHandIdle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_HandToHandTorchIdle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_OneHandIdle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_OneHandTorchIdle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_TwoHandIdle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_StaffIdle;
extern SME::INI::INISetting				kInventoryIdleOverridePath_BowIdle;


// C4+?
class FaceGenHeadParameters
{
public:
	// 18
	struct UnkData18
	{
		UInt32			unk00;
		UInt32			unk04;
		UInt32			unk08[3];
		UInt32			unk14;
	};

	// act as indices into the arrays
	// kFaceGenData_EyesLeft/Right have empty TESTexture/NiTexture* 
	enum
	{
		kFaceGenData_Head			= 0,
		kFaceGenData_EarsMale,
		kFaceGenData_EarsFemale,
		kFaceGenData_Mouth,
		kFaceGenData_TeethLower,
		kFaceGenData_TeethUpper,
		kFaceGenData_Tongue,
		kFaceGenData_EyesLeft,
		kFaceGenData_EyesRight		= 8,

		kFaceGenData__MAX
	};

	UnkData18							unk00[4];							// 00
	TESHair*							hair;								// 60
	RGBA								hairColor;							// 64
	float								unk68;								// 68 - hair length
	TESEyes*							eyes;								// 6C
	UInt32								female;								// 70 - set to 1 if female
	NiTArray<::TESModel*>				models;								// 74
	NiTArray<::TESTexture*>				textures;							// 84
	NiTArray<const char*>				nodeNames;							// 94
	NiTArray<NiPointer<NiTexture>>		sourceTextures;						// A4 - NiSourceTexture*, populated from the editor-exported textures
	UInt8								unkB4;
	UInt8								padB5[3];
	UInt32								unkB8[2];
	SInt32								unkC0;								// init to -1
};
STATIC_ASSERT(sizeof(FaceGenHeadParameters::UnkData18) == 0x18);
STATIC_ASSERT(sizeof(FaceGenHeadParameters) == 0xC4);

_DeclareMemHdlr(RaceSexMenuPoser, "unrestricted camera movement in the racesex menu");
_DeclareMemHdlr(RaceSexMenuRender, "prevents the camera from being reset every frame");
_DeclareMemHdlr(PlayerInventory3DAnimSequenceQueue, "blind men in the market buying what they're sold");

void BlockHeads(void);

// not very pretty but better than having to switch b'ween 2 class definitions
namespace InstanceAbstraction
{
	extern bool					EditorMode;

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
}
