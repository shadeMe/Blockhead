#include "AssetOverride.h"

ActorAssetOverriderKernel		ActorAssetOverriderKernel::Instance;


IActorAssetData::IActorAssetData( UInt32 Type, AssetComponentT Component, TESNPC* Actor, const char* Path ) :
	AssetType(Type),
	AssetComponent(Component),
	Actor(Actor),
	Race(NULL),
	AssetPath(Path)
{
	SME_ASSERT(Actor && AssetType > kAssetType_Invalid && AssetType < kAssetType__MAX);

	Race = InstanceAbstraction::GetNPCRace(Actor);
}

const char* IActorAssetData::GetRootDirectory( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return "Textures";
	case kAssetType_Model:
		return "Meshes";
	default:
		return "<Unknown>";
	}
}

const char* IActorAssetData::GetFileExtension( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return "dds";
	case kAssetType_Model:
		return "nif";
	default:
		return "<Unknown>";
	}
}

const char* IActorAssetData::GetAssetTypeName( void ) const
{
	switch (AssetType)
	{
	case kAssetType_Texture:
		return "Texture";
	case kAssetType_Model:
		return "Model";
	default:
		return "<Unknown>";
	}
}

const std::string IActorAssetData::Describe( void ) const
{
	char Buffer[0x200] = {0};
	FORMAT_STR(Buffer, "%s %s for NPC %08X", GetComponentName(), GetAssetTypeName(), Actor->refID);
	return Buffer;
}

IAssetOverrideAgent::IAssetOverrideAgent( IActorAssetData* Data, UInt32 ID /*= kID_Default*/ ) :
	ID(ID),
	Data(Data)
{
	SME_ASSERT(Data);
}

bool IAssetOverrideAgent::operator<( const IAssetOverrideAgent& Second ) const
{
	return this->ID < Second.ID;
}

bool IAssetOverrideAgent::IsDefaultOverride( void ) const
{
	return ID == kID_Default;
}

bool IScriptedOverrideData::IsEmpty( void ) const
{
	return OverridePaths.size() == 0;
}

bool IScriptedOverrideData::Find( AssetComponentT Component, ComponentOverridePathMapT::iterator& Match )
{
	bool Result = false;
	if (OverridePaths.count(Component))
	{
		Match = OverridePaths.find(Component);
		Result = true;
	}

	return Result;
}

bool IScriptedOverrideData::Set( AssetComponentT Component, const char* Path )
{
	SME_ASSERT(Path);

	bool Result = false;

	if (strlen(Path) > 2)
	{
		if (VerifyPath(Path))
		{
			Result = true;
			OverridePaths[Component] = Path;
		}
	}

	return Result;
}

bool IScriptedOverrideData::Remove( AssetComponentT Component )
{
	ComponentOverridePathMapT::iterator Match = OverridePaths.end();

	if (Find(Component, Match))
	{
		OverridePaths.erase(Match);
	}

	return IsEmpty();
}

const char* IScriptedOverrideData::Get( AssetComponentT Component ) const
{
	if (OverridePaths.count(Component))
		return OverridePaths.at(Component).c_str();
	else
		return NULL;
}

void IScriptedOverrideData::Clear( void )
{
	OverridePaths.clear();
}

bool ScriptedTextureOverrideData::VerifyPath( const char* Path ) const
{
	std::string RelPath = "Textures\\" + std::string(Path);
	return InstanceAbstraction::FileFinder::GetFileExists(RelPath.c_str());
}

bool ScriptedModelOverrideData::VerifyPath( const char* Path ) const
{
	std::string RelPath = "Meshes\\" + std::string(Path);
	return InstanceAbstraction::FileFinder::GetFileExists(RelPath.c_str());
}

IScriptAssetOverrideAgent::IScriptAssetOverrideAgent( IActorAssetData* Data, IScriptedOverrideManager* Manager ) :
	IAssetOverrideAgent(Data, kID_Script),
	OverrideManager(Manager)
{
	;//
}

bool IScriptAssetOverrideAgent::Query( std::string& OutOverridePath )
{
	bool Result = false;

	const char* OverridePath = OverrideManager->GetOverridePath(Data);
	if (OverridePath)
	{
		Result = true;
		OutOverridePath = OverridePath;
	}

	return Result;
}

IPerNPCAssetOverrideAgent::IPerNPCAssetOverrideAgent( IActorAssetData* Data ) :
	IAssetOverrideAgent(Data, kID_NPC)
{
	;//
}

bool IPerNPCAssetOverrideAgent::Query( std::string& OutOverridePath )
{
	bool Result = false;

	if (GetEnabled())
	{
		UInt32 FormID = Data->Actor->refID & 0x00FFFFFF;
		TESFile* Plugin = InstanceAbstraction::GetOverrideFile(Data->Actor, 0);

		const char* PathSuffix = Data->GetComponentName();
		const char* BaseDir = Data->GetRootDirectory();

		if (Plugin)
		{
			char Buffer[MAX_PATH] = {0};
			FORMAT_STR(Buffer, "%s\\%s\\%08X_%s.%s", GetOverrideSourceDirectory(), Plugin->name, FormID, PathSuffix, Data->GetFileExtension());

#ifndef NDEBUG
			_MESSAGE("Checking override path %s for NPC %08X", Buffer, Data->Actor->refID);
#endif // !NDEBUG

			std::string FullPath(BaseDir); FullPath += "\\" + std::string(Buffer);
			if (InstanceAbstraction::FileFinder::GetFileExists(FullPath.c_str()))
			{
				Result = true;
				OutOverridePath = Buffer;
			}
		}
	}

	return Result;
}

IPerRaceAssetOverrideAgent::IPerRaceAssetOverrideAgent( IActorAssetData* Data ) :
	IAssetOverrideAgent(Data, kID_Race)
{
	;//
}

DefaultAssetOverrideAgent::DefaultAssetOverrideAgent( IActorAssetData* Data ) :
	IAssetOverrideAgent(Data)
{
	;//
}

bool DefaultAssetOverrideAgent::Query( std::string& OutOverridePath )
{
	if (Data->AssetPath)
	{
		OutOverridePath = Data->AssetPath;
		return true;
	}
	else
		return false;
}

bool ActorAssetOverriderKernel::SortComparator( const OverrideAgentHandleT& First, const OverrideAgentHandleT& Second )
{
	return (*First.get() < *Second.get());
}

void ActorAssetOverriderKernel::PrepareStack( IActorAssetData* Data )
{
	ResetStack();

	Data->GetOverrideAgents(AgentStack);
	SME_ASSERT(AgentStack.size());

	AgentStack.sort(SortComparator);
//	DumpStack();
}

void ActorAssetOverriderKernel::ResetStack( void )
{
	AgentStack.clear();
}

void ActorAssetOverriderKernel::DumpStack( void ) const
{
#ifndef NDEBUG
	std::string Buffer("Agent Stack : ");

	for (OverrideAgentListT::const_iterator Itr = AgentStack.begin(); Itr != AgentStack.end(); Itr++)
	{
		char Temp[0x50] = {0};
		FORMAT_STR(Temp, "%d > ", (*Itr)->ID);

		Buffer += Temp;
	}

	Buffer += "NULL";
	_MESSAGE("%s", Buffer.c_str());
#endif // !NDEBUG
}

ActorAssetOverriderKernel::ActorAssetOverriderKernel() :
	AgentStack(),
	Lock()
{
	;//
}

ActorAssetOverriderKernel::~ActorAssetOverriderKernel()
{
	ResetStack();
}

bool ActorAssetOverriderKernel::ApplyOverride(IActorAssetData* Data, std::string& OutOverridePath)
{
	ScopedLock Guard(Lock);

	SME_ASSERT(Data);
	bool Result = false;

	if (Data->IsValid())
	{
#ifndef NDEBUG
		_MESSAGE("Attempting to override %s...", Data->Describe().c_str());
		gLog.Indent();
#endif // !NDEBUG

		PrepareStack(Data);

		for (OverrideAgentListT::iterator Itr = AgentStack.begin(); Itr != AgentStack.end(); Itr++)
		{
			if ((*Itr)->Query(OutOverridePath))
			{		
				if ((*Itr)->IsDefaultOverride() == false)
				{
					Result = true;
#ifndef NDEBUG
					_MESSAGE("Override applied - New path: %s", OutOverridePath.c_str());
#endif // !NDEBUG
				}

				break;
			}
		}

#ifndef NDEBUG
		gLog.Outdent();
#endif // !NDEBUG

		ResetStack();
	}

	return Result;
}
