#pragma once

#include "BlockheadInternals.h"

_DeclareMemHdlr(TESRaceGetBodyTexture, "listen to Jesper Kyd - He's, as they say, da ballz");
_DeclareMemHdlr(TESRaceGetBodyModelA, "same as above, only this time it's John Petrucci");
_DeclareMemHdlr(TESRaceGetBodyModelB, "");


// body part IDs used by the game
enum
{
	kRaceBodyPart_UpperBody	= 2,					// same for arms
	kRaceBodyPart_LowerBody	= 3,
	kRaceBodyPart_Hand		= 4,
	kRaceBodyPart_Foot		= 5,
	kRaceBodyPart_Tail		= 15,
};

class BodyOverriderKernel;

class IBodyOverrideAgent
{
	friend class BodyOverriderKernel;
protected:
	const UInt32					ID;
	const UInt32					AssetType;

	const char*						GetRootDirectory(void) const;
	const char*						GetFileExtension(void) const;
public:
	// in order of precedence
	enum
	{
		kID_Script		= 0,			// scripted overrides
		kID_NPC			= 1,			// per-NPC 
		kID_Race		= 2,			// per-race & per-gender
		kID_Default		= 3,			// no override
	};

	enum
	{
		kAssetType_Invalid		= 0,

		kAssetType_Texture		= 1,
		kAssetType_Model		= 2,

		kAssetType__MAX
	};

	IBodyOverrideAgent(UInt32 ID = kID_Default, UInt32 AssetType = kAssetType_Invalid);

	virtual ~IBodyOverrideAgent() = 0
	{
		;//
	}

	bool						operator<(const IBodyOverrideAgent& Second) const;
	bool						IsDefaultOverride(void) const;

								// returns true if the override is applicable
	virtual bool				Query(TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath) = 0;

	static bool					IsValidBodyPart(UInt32 BodyPart);
	static const char*			GetBodyPartName(UInt32 BodyPart);
	
	static const char*			GetAssetTypeName(UInt32 AssetType);
};


class ScriptedOverrideData
{
protected:
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

	std::string&				GetOverridePath(UInt32 BodyPart);
	const std::string&			GetOverridePath(UInt32 BodyPart) const;

	virtual bool				VerifyPath(const char* Path) const = 0;
public:
	virtual ~ScriptedOverrideData() = 0
	{
		;//
	}

	bool						Set(UInt32 BodyPart, const char* Path);		// returns true if successful, checks path
	bool						Remove(UInt32 BodyPart);					// returns true if the operation clears all overrides
	const char*					Get(UInt32 BodyPart) const;
	void						Clear(void);
};

class ScriptedTextureOverrideData : public ScriptedOverrideData
{
protected:
	virtual bool				VerifyPath(const char* Path) const;
public:
	virtual ~ScriptedTextureOverrideData();
};

class ScriptedModelOverrideData : public ScriptedOverrideData
{
protected:
	virtual bool				VerifyPath(const char* Path) const;
public:
	virtual ~ScriptedModelOverrideData();
};

class IScriptedOverrideManager
{
public:
	virtual ~IScriptedOverrideManager() = 0
	{
		;//
	}

	virtual const char*				GetOverridePath(TESNPC* NPC, UInt32 BodyPart) const = 0;
};

template<typename OverrideT>
class ScriptedBodyAssetOverrideManager : public IScriptedOverrideManager
{
	typedef UInt32					NPCHandleT;				// Pretension, by Fry & Laurie

	typedef boost::shared_ptr<OverrideT>					OverrideDataHandleT;
	typedef std::map<NPCHandleT, OverrideDataHandleT>		OverrideDataStoreT;

	OverrideDataStoreT				DataStore;

	bool							Find(NPCHandleT NPC, typename OverrideDataStoreT::iterator& Match)
	{
		if (DataStore.count(NPC))
		{
			Match = DataStore.find(NPC);
			return true;
		}
		else
		{
			return false;
		}
	}

public:
	ScriptedBodyAssetOverrideManager() :
		IScriptedOverrideManager(),
		DataStore()
	{
		;//
	}

	virtual ~ScriptedBodyAssetOverrideManager()
	{
		Clear();
	}

	bool							Add(TESNPC* NPC, UInt32 BodyPart, const char* OverridePath)
	{
		SME_ASSERT(NPC && OverridePath);

		bool Result = false;

		if (IBodyOverrideAgent::IsValidBodyPart(BodyPart))
		{
			NPCHandleT NPCHandle = NPC->refID;
			if (DataStore.count(NPCHandle))
			{
				Result = DataStore[NPCHandle]->Set(BodyPart, OverridePath);
			}
			else
			{
				OverrideDataHandleT OverrideHandle(new OverrideT());

				Result = OverrideHandle->Set(BodyPart, OverridePath);
				if (Result)
				{
					DataStore[NPCHandle] = OverrideHandle;
				}
			}
		}

		return Result;
	}

	void							Remove(TESNPC* NPC, UInt32 BodyPart)
	{
		SME_ASSERT(NPC);

		if (IBodyOverrideAgent::IsValidBodyPart(BodyPart))
		{
			NPCHandleT NPCHandle = NPC->refID;
			OverrideDataStoreT::iterator Match = DataStore.end();

			if (Find(NPCHandle, Match))
			{
				if (DataStore[NPCHandle]->Remove(BodyPart))
				{
					DataStore.erase(Match);
				}
			}
		}
	}

	void							Clear(void)
	{
		DataStore.clear();
	}

	virtual const char*				GetOverridePath(TESNPC* NPC, UInt32 BodyPart) const
	{
		SME_ASSERT(NPC);

		const char* OverridePath = NULL;

		NPCHandleT NPCHandle = NPC->refID;

		if (DataStore.count(NPCHandle))
		{
			OverridePath = DataStore.at(NPCHandle)->Get(BodyPart);
		}

		return OverridePath;
	}
};

class ScriptBodyOverrideAgent : public IBodyOverrideAgent
{
	const IScriptedOverrideManager*			OverrideManager;
public:
	ScriptBodyOverrideAgent(UInt32 AssetType);
	virtual ~ScriptBodyOverrideAgent();

	virtual bool							Query(TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath);

	static ScriptedBodyAssetOverrideManager<ScriptedTextureOverrideData>		ScriptBodyTexOverrides;
	static ScriptedBodyAssetOverrideManager<ScriptedModelOverrideData>			ScriptBodyMeshOverrides;
};

class PerNPCBodyOverrideAgent : public IBodyOverrideAgent
{
	static const char*						kSourceDirectory;					// relative to the asset type's root directory

	bool									GetEnabled(void) const;
public:
	PerNPCBodyOverrideAgent(UInt32 AssetType);
	virtual ~PerNPCBodyOverrideAgent();

	virtual bool							Query(TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath);
}; 

class PerRaceBodyOverrideAgent : public IBodyOverrideAgent
{
	static const char*						kSourceDirectory;					// relative to the asset type's root directory

	bool									GetEnabled(void) const;
public:
	PerRaceBodyOverrideAgent(UInt32 AssetType);
	virtual ~PerRaceBodyOverrideAgent();

	virtual bool							Query(TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath);
}; 

class DefaultBodyOverrideAgent : public IBodyOverrideAgent
{
public:
	DefaultBodyOverrideAgent();
	virtual ~DefaultBodyOverrideAgent();

	virtual bool							Query(TESNPC* NPC, TESRace* Race, UInt32 BodyPart, const char* OriginalPath, std::string& OutOverridePath);
};

class BodyOverriderKernel
{
	typedef boost::shared_ptr<IBodyOverrideAgent>			OverrideAgentHandleT;
	typedef std::list<OverrideAgentHandleT>					OverrideAgentListT;

	static bool								SortComparator(OverrideAgentHandleT& First, OverrideAgentHandleT& Second);

	OverrideAgentListT						AgentStack;

	void									PrepareStack(UInt32 AssetType);
	void									ResetStack(void);
	void									DumpStack(void) const;
public:
	BodyOverriderKernel();
	~BodyOverriderKernel();
											
											// returns the override path, relative to the asset type's root directory
	std::string								ApplyOverride(UInt32 AssetType, UInt32 BodyPart, TESNPC* NPC, TESRace* Race, const char* OriginalPath);

	static BodyOverriderKernel				Instance;
};

void PatchBodyOverride(void);