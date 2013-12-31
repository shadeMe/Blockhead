#pragma once

#include "AssetOverride.h"


class ActorHeadAssetData : public IActorAssetData
{
public:
	ActorHeadAssetData(UInt32 Type, AssetComponentT Component, TESNPC* Actor, const char* Path);
	virtual ~ActorHeadAssetData()
	{
		;//
	}

	virtual bool							IsValid(void) const;
	virtual const char*						GetComponentName(void) const;

	virtual void							GetOverrideAgents(OverrideAgentListT& List);
};

class ScriptHeadOverrideAgent : public IScriptAssetOverrideAgent
{
public:
	ScriptHeadOverrideAgent(IActorAssetData* Data);
	virtual ~ScriptHeadOverrideAgent()
	{
		;//
	}

	static ScriptedActorAssetOverrider<ScriptedTextureOverrideData>		TextureOverrides;
	static ScriptedActorAssetOverrider<ScriptedModelOverrideData>		MeshOverrides;
};

class PerNPCHeadOverrideAgent : public IPerNPCAssetOverrideAgent
{
protected:
	virtual bool							GetEnabled(void) const;
	virtual const char*						GetOverrideSourceDirectory(void) const;
public:
	PerNPCHeadOverrideAgent(IActorAssetData* Data);
	virtual ~PerNPCHeadOverrideAgent()
	{
		;//
	}
};

class PerRaceHeadOverrideAgent : public IPerRaceAssetOverrideAgent
{
protected:
	virtual bool							GetEnabled(void) const;
	virtual const char*						GetOverrideSourceDirectory(void) const;

	bool									GetComponentGenderVariant(void) const;			// returns true for cmpts that are gender variant by default
public:
	PerRaceHeadOverrideAgent(IActorAssetData* Data);
	virtual ~PerRaceHeadOverrideAgent()
	{
		;//
	}

	virtual bool							Query(std::string& OutOverridePath);
}; 

class FaceGenAgeTextureOverrider
{
public:
	typedef InstanceAbstraction::TESTexture::Instance	Texture;
private:
	static const char*									kOverrideSourceDirectory;

	typedef std::map<Texture, Texture>					OverrideHeadTextureMapT;
	typedef std::map<NPCHandleT, std::string>			AgeTextureBasePathMapT;

	OverrideHeadTextureMapT								OverriddenHeadTextures;		// maps new allocations to their old ones
	AgeTextureBasePathMapT								ScriptOverrides;				
	mutable ICriticalSection							Lock;

	bool												TryGetClosestAgeTexture(std::string& OutPath,
																				const char* BasePath,
																				SInt32 Age,
																				bool Female,
																				bool UseGender) const;
public:
	FaceGenAgeTextureOverrider();
	~FaceGenAgeTextureOverrider();

	void												TrackHeadOverride(Texture Duplicate, Texture Original);
	void												UntrackHeadOverride(Texture Duplicate);

	void												RegisterAgeTextureScriptOverride(TESNPC* NPC, const char* BasePath);
	void												UnregisterAgeTextureScriptOverride(TESNPC* NPC);
	void												ResetAgeTextureScriptOverrides(void);

	std::string											GetAgeTexturePath(TESNPC* NPC, SInt32 Age, const char* CurrentBasePath, Texture HeadTexture) const;

	static FaceGenAgeTextureOverrider					Instance;
};

void PatchHeadOverride(void);

namespace HeadOverride
{
	void HandleLoadGame(void);
}