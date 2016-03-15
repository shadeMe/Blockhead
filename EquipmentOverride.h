#pragma once

#include "BlockheadInternals.h"


class ActorEquipmentOverrider
{
public:
	typedef UInt32						OverrideHandlerIdentifierT;
private:
	class OverrideHandler
	{
	public:
		class Result
		{
			OverrideHandlerIdentifierT		SourceID;
			UInt32							Priority;
			TESForm*						ModelSource;
			TESModel*						OverrideModel;
		public:
			Result(OverrideHandlerIdentifierT Source, UInt32 Priority, TESForm* ModelSource, TESModel* OverrideModel);

			TESForm*						GetModelSource() const;
			TESModel*						GetModel() const;

			static bool						SortComparator(const Result& LHS, const Result& RHS);
		};

		typedef std::list<Result>			OverrideResultListT;
	private:
		OverrideHandlerIdentifierT		ID;
		Script*							Handler;		// user-defined function

		struct
		{
			TESObjectREFR*				Reference;
			TESNPC*						NPC;
			TESRace*					Race;
			TESForm*					EquippedItem;
		}								Filters;

		enum
		{
			kResultArrayIndex_Priority					= 0,
			kResultArrayIndex_OverrideModelSource		= 1,

			kResultArray__Size
		};
	public:
		OverrideHandler(OverrideHandlerIdentifierT ID, Script* Handler, TESObjectREFR* Ref = NULL, TESNPC* NPC = NULL, TESRace* Race = NULL, TESForm* EquippedItem = NULL);

		bool							HandleEquipment(ActorBodyModelData* ModelData,
														TESForm* ModelSource,
														TESObjectREFR* Ref,
														TESNPC* NPC,
														OverrideResultListT& OutResults) const;			// if an override was found, adds the result to the list and returns true. returns false otherwise
	};

	typedef std::map<OverrideHandlerIdentifierT, OverrideHandler>		OverrideHandlerMapT;

	OverrideHandlerMapT					HandlerTable;
	OverrideHandlerIdentifierT			NextID;
	bool								OverrideInProgress;

	bool								GetEnabled() const;
public:
	ActorEquipmentOverrider();
	~ActorEquipmentOverrider();

	static const OverrideHandlerIdentifierT				InvalidID = 0;

	bool								RegisterHandler(Script* UserFunction,
														TESObjectREFR* FilterRef,
														TESNPC* FilterNPC,
														TESRace* FilterRace,
														TESForm* FilterEquippeditem,
														OverrideHandlerIdentifierT& OutID);
	bool								UnregisterHandler(OverrideHandlerIdentifierT ID);
	void								ClearHandlers();

	void								ApplyOverride(int BodyPart, ActorBodyModelData* ModelData, TESForm* ModelSource);

	static ActorEquipmentOverrider		Instance;
};


_DeclareMemHdlr(TESBipedModelFormGetBodyPartModel, "applies overrides when the actor's body models are refreshed");

void PatchEquipmentOverride(void);

namespace EquipmentOverride
{
	void HandleLoadGame(void);
}
