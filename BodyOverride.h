#pragma once

#include "AssetOverride.h"

_DeclareMemHdlr(TESRaceGetBodyTexture, "listen to Jesper Kyd - He's, as they say, da ballz");
_DeclareMemHdlr(TESRaceGetBodyModelA, "same as above, only this time it's John Petrucci");
_DeclareMemHdlr(TESRaceGetBodyModelB, "");


class ActorBodyAssetData : public IActorAssetData
{
public:
	// body part IDs used by the game
	enum
	{
		kBodyPart_UpperBody	= 2,					// same for arms
		kBodyPart_LowerBody	= 3,
		kBodyPart_Hand		= 4,
		kBodyPart_Foot		= 5,
		kBodyPart_Tail		= 15,
	};

	ActorBodyAssetData(UInt32 Type, AssetComponentT Component, TESNPC* Actor, const char* Path);
	virtual ~ActorBodyAssetData()
	{
		;//
	}

	virtual bool								IsValid(void) const;
	virtual const char*							GetComponentName(void) const;

	virtual void								GetOverrideAgents(OverrideAgentListT& List);
};

class ScriptBodyOverrideAgent : public IScriptAssetOverrideAgent
{
public:
	ScriptBodyOverrideAgent(IActorAssetData* Data);
	virtual ~ScriptBodyOverrideAgent()
	{
		;//
	}

	static ScriptedActorAssetOverrider<ScriptedTextureOverrideData>		TextureOverrides;
	static ScriptedActorAssetOverrider<ScriptedModelOverrideData>		MeshOverrides;
};

class PerNPCBodyOverrideAgent : public IPerNPCAssetOverrideAgent
{
protected:
	virtual bool							GetEnabled(void) const;
	virtual const char*						GetOverrideSourceDirectory(void) const;
public:
	PerNPCBodyOverrideAgent(IActorAssetData* Data);
	virtual ~PerNPCBodyOverrideAgent()
	{
		;//
	}
};

class PerRaceBodyOverrideAgent : public IPerRaceAssetOverrideAgent
{
protected:
	virtual bool							GetEnabled(void) const;
	virtual const char*						GetOverrideSourceDirectory(void) const;
public:
	PerRaceBodyOverrideAgent(IActorAssetData* Data);
	virtual ~PerRaceBodyOverrideAgent()
	{
		;//
	}

	virtual bool				Query(std::string& OutOverridePath);
}; 

void PatchBodyOverride(void);

namespace BodyOverride
{
	// the engine caches 3D body model data (as it doesn't expect the mesh after game init), causing mismatching models when loading a save game where the gender of the NPC/player has changed
	// easiest thing to do would be to quit to the main menu before loading the save, which flushes the cache
	// we'll just update the PC's model as it's the most ostentatious
	void FixPlayerBodyModel(void);

	void HandleLoadGame(bool FixPlayer3D);
}