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

#include <boost\shared_ptr.hpp>

#include <SME_Prefix.h>
#include <MemoryHandler.h>
#include <INIManager.h>
#include <StringHelpers.h>

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
	extern SME::INI::INISetting				kGenderVariantHeadMeshes;
	extern SME::INI::INISetting				kGenderVariantHeadTextures;

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

	extern SME::INI::INISetting				kOverrideUpperBodyTexture;
	extern SME::INI::INISetting				kOverrideLowerBodyTexture;
	extern SME::INI::INISetting				kOverrideHandTexture;
	extern SME::INI::INISetting				kOverrideFootTexture;
	extern SME::INI::INISetting				kOverrideTailTexture;
}

_DeclareMemHdlr(RaceSexMenuPoser, "unrestricted camera movement in the racesex menu");
_DeclareMemHdlr(RaceSexMenuRender, "prevents the camera from being reset every frame");
_DeclareMemHdlr(PlayerInventory3DAnimSequenceQueue, "blind men in the market buying what they're sold");
_DeclareMemHdlr(TESRaceGetBodyTexture, "listen to Jesper Kyd - He's, as they say, da ballz");

enum
{
	kRaceBodyTextureSkin_UpperBody	= 2,					// same for arms
	kRaceBodyTextureSkin_LowerBody	= 3,
	kRaceBodyTextureSkin_Hand		= 4,
	kRaceBodyTextureSkin_Foot		= 5,
	kRaceBodyTextureSkin_Tail		= 15,
};

class ScriptedBodyTextureOverrideManager
{
	typedef UInt32					NPCHandleT;				// more pretension
	
	class OverrideData
	{
		enum
		{
			kOverridePath_UpperBody = 0,
			kOverridePath_LowerBody,
			kOverridePath_Hand,
			kOverridePath_Foot,
			kOverridePath_Tail,

			kOverridePath__MAX
		};

		std::string					OverridePaths[kOverridePath__MAX];

		bool						IsEmpty(void) const;

		std::string&				GetOverridePath(UInt32 BodyPath);
		const std::string&			GetOverridePath(UInt32 BodyPath) const;
	public:
		bool						Set(UInt32 BodyPart, const char* Path);		// returns true if successful, checks path
		bool						Remove(UInt32 BodyPart);					// returns true if the operation clears all overrides
		const char*					Get(UInt32 BodyPart) const;
	};

	typedef boost::shared_ptr<OverrideData>					OverrideDataHandleT;
	typedef std::map<NPCHandleT, OverrideDataHandleT>		OverrideDataStoreT;

	OverrideDataStoreT				DataStore;

	bool							Find(NPCHandleT NPC, OverrideDataStoreT::iterator& Match);
	static bool						IsValidBodyPart(UInt32 BodyPart);
public:
	ScriptedBodyTextureOverrideManager();

	bool							Add(TESNPC* NPC, UInt32 BodyPart, const char* OverridePath);		// path must be relative to Data\Textures
	void							Remove(TESNPC* NPC, UInt32 BodyPart);
	void							Clear(void);

	const char*						GetOverridePath(TESNPC* NPC, UInt32 BodyPart) const;

	static ScriptedBodyTextureOverrideManager			Instance;
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

	void*			FormHeap_Allocate(UInt32 Size);
	void			FormHeap_Free(void* Pointer);

	float			GetNPCFaceGenAge(TESNPC* NPC);
	void			SetNPCFaceGenAge(TESNPC* NPC, float Age);
}
