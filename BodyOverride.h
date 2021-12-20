#pragma once

#include "AssetOverride.h"

_DeclareMemHdlr(TESRaceGetBodyTexture, "swaps body textures");
_DeclareMemHdlr(TESRaceGetBodyModelA, "swaps body models");
_DeclareMemHdlr(TESRaceGetBodyModelB, "");
_DeclareMemHdlr(TESRaceGetBodyEGT, "swaps facegen texture model");

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

	static const std::vector<const char*>		ValidComponentNames;
	static const char*							OverrideFolderName;
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
	static ScriptedActorAssetOverrider<ScriptedModelOverrideData>		FaceGenTextureOverrides;
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
	void FixPlayerBodyModel(void);

	void HandleLoadGame(bool FixPlayer3D);
}