#pragma once

#include "BlockheadInternals.h"

class ActorAssetOverriderKernel;
class IAssetOverrideAgent;
class IScriptAssetOverrideAgent;
class IPerRaceAssetOverrideAgent;
class IPerNPCAssetOverrideAgent;

typedef SInt32											AssetComponentT;
typedef boost::shared_ptr<IAssetOverrideAgent>			OverrideAgentHandleT;
typedef std::list<OverrideAgentHandleT>					OverrideAgentListT;

class IActorAssetData
{
public:
	enum
	{
		kAssetType_Invalid		= 0,

		kAssetType_Texture		= 1,
		kAssetType_Model		= 2,

		kAssetType__MAX
	};

	UInt32							AssetType;
	AssetComponentT					AssetComponent;
	TESNPC*							Actor;
	TESRace*						Race;
	const char*						AssetPath;			// the original asset path

	IActorAssetData(UInt32 Type, AssetComponentT Component, TESNPC* Actor, const char* Path);
	virtual ~IActorAssetData() = 0
	{
		;//
	}

	const char*									GetRootDirectory(void) const;
	const char*									GetFileExtension(void) const;
	const char*									GetAssetTypeName(void) const;
	
	virtual const std::string					Describe(void) const;
	virtual bool								IsValid(void) const = 0;				// checks cmpt type, etc. if invalid, overriding is disabled
	virtual const char*							GetComponentName(void) const = 0;

	virtual void								GetOverrideAgents(OverrideAgentListT& List) = 0;
};

class IAssetOverrideAgent
{
	friend class ActorAssetOverriderKernel;
protected:
	const UInt32					ID;
	IActorAssetData*				Data;
public:
	// in order of precedence
	enum
	{
		kID_Script		= 0,			// scripted overrides
		kID_NPC			= 1,			// per-NPC 
		kID_Race		= 2,			// per-race & per-gender
		kID_Default		= 3,			// no override
	};

	IAssetOverrideAgent(IActorAssetData* Data, UInt32 ID = kID_Default);
	virtual ~IAssetOverrideAgent() = 0
	{
		;//
	}

	bool						operator<(const IAssetOverrideAgent& Second) const;
	bool						IsDefaultOverride(void) const;
									
	virtual bool				Query(std::string& OutOverridePath) = 0;		// returns true if the override is applicable
};

class IScriptedOverrideData
{
protected:
	typedef std::map<AssetComponentT, std::string>		ComponentOverridePathMapT;

	ComponentOverridePathMapT	OverridePaths;

	bool						IsEmpty(void) const;
	bool						Find(AssetComponentT Component, ComponentOverridePathMapT::iterator& Match);

	virtual bool				VerifyPath(const char* Path) const = 0;
public:
	virtual ~IScriptedOverrideData() = 0
	{
		;//
	}
	
	bool						Set(AssetComponentT Component, const char* Path);		// returns true if successful, checks path
	bool						Remove(AssetComponentT Component);						// returns true if the operation clears all overrides
	const char*					Get(AssetComponentT Component) const;
	void						Clear(void);
};

class ScriptedTextureOverrideData : public IScriptedOverrideData
{
protected:
	virtual bool				VerifyPath(const char* Path) const;
public:
	virtual ~ScriptedTextureOverrideData()
	{
		;//
	}
};

class ScriptedModelOverrideData : public IScriptedOverrideData
{
protected:
	virtual bool				VerifyPath(const char* Path) const;
public:
	virtual ~ScriptedModelOverrideData()
	{
		;//
	}
};

class IScriptedOverrideManager
{
public:
	virtual ~IScriptedOverrideManager() = 0
	{
		;//
	}

	virtual const char*				GetOverridePath(TESNPC* NPC, AssetComponentT Component) const = 0;
	virtual const char*				GetOverridePath(IActorAssetData* Data) const = 0;
};

template<typename OverrideT>
class ScriptedActorAssetOverrider : public IScriptedOverrideManager
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
	ScriptedActorAssetOverrider() :
		IScriptedOverrideManager(),
		DataStore()
	{
		;//
	}

	virtual ~ScriptedActorAssetOverrider()
	{
		Clear();
	}

	bool							Add(TESNPC* NPC, AssetComponentT Component, const char* OverridePath)
	{
		SME_ASSERT(NPC && OverridePath);

		bool Result = false;

		NPCHandleT NPCHandle = NPC->refID;
		if (DataStore.count(NPCHandle))
		{
			Result = DataStore[NPCHandle]->Set(Component, OverridePath);
		}
		else
		{
			OverrideDataHandleT OverrideHandle(new OverrideT());

			Result = OverrideHandle->Set(Component, OverridePath);
			if (Result)
			{
				DataStore[NPCHandle] = OverrideHandle;
			}
		}

		return Result;
	}

	void							Remove(TESNPC* NPC, AssetComponentT Component)
	{
		SME_ASSERT(NPC);

		NPCHandleT NPCHandle = NPC->refID;
		OverrideDataStoreT::iterator Match = DataStore.end();

		if (Find(NPCHandle, Match))
		{
			if (DataStore[NPCHandle]->Remove(Component))
			{
				DataStore.erase(Match);
			}
		}
	}

	void							Clear(void)
	{
		DataStore.clear();
	}

	virtual const char*				GetOverridePath(TESNPC* NPC, AssetComponentT Component) const
	{
		SME_ASSERT(NPC);

		const char* OverridePath = NULL;

		NPCHandleT NPCHandle = NPC->refID;

		if (DataStore.count(NPCHandle))
		{
			OverridePath = DataStore.at(NPCHandle)->Get(Component);
		}

		return OverridePath;
	}

	virtual const char*				GetOverridePath(IActorAssetData* Data) const
	{
		SME_ASSERT(Data);

		return GetOverridePath(Data->Actor, Data->AssetComponent);
	}
};

class IScriptAssetOverrideAgent : public IAssetOverrideAgent
{
protected:
	const IScriptedOverrideManager*			OverrideManager;
public:
	IScriptAssetOverrideAgent(IActorAssetData* Data, IScriptedOverrideManager* Manager);
	virtual ~IScriptAssetOverrideAgent() = 0
	{
		;//
	}

	virtual bool				Query(std::string& OutOverridePath);
};

class IPerNPCAssetOverrideAgent : public IAssetOverrideAgent
{
protected:
	virtual bool							GetEnabled(void) const = 0;
	virtual const char*						GetOverrideSourceDirectory(void) const = 0;
public:
	IPerNPCAssetOverrideAgent(IActorAssetData* Data);
	virtual ~IPerNPCAssetOverrideAgent()
	{
		;//
	}

	virtual bool				Query(std::string& OutOverridePath);
}; 

class IPerRaceAssetOverrideAgent : public IAssetOverrideAgent
{
protected:
	virtual bool							GetEnabled(void) const = 0;
	virtual const char*						GetOverrideSourceDirectory(void) const = 0;
public:
	IPerRaceAssetOverrideAgent(IActorAssetData* Data);
	virtual ~IPerRaceAssetOverrideAgent()
	{
		;//
	}
}; 

class DefaultAssetOverrideAgent : public IAssetOverrideAgent
{
public:
	DefaultAssetOverrideAgent(IActorAssetData* Data);
	virtual ~DefaultAssetOverrideAgent()
	{
		;//
	}

	virtual bool				Query(std::string& OutOverridePath);
};

class ActorAssetOverriderKernel
{
	static bool								SortComparator(const OverrideAgentHandleT& First, const OverrideAgentHandleT& Second);

	OverrideAgentListT						AgentStack;
	mutable ICriticalSection				Lock;

	void									PrepareStack(IActorAssetData* Data);
	void									ResetStack(void);
	void									DumpStack(void) const;
public:
	ActorAssetOverriderKernel();
	~ActorAssetOverriderKernel();
											
											// returns true if overridden, override path's relative to the asset type's root directory
	bool									ApplyOverride(IActorAssetData* Data, std::string& OutOverridePath);

	static ActorAssetOverriderKernel		Instance;
};