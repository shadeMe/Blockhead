#include "EquipmentOverride.h"

ActorEquipmentOverrider				ActorEquipmentOverrider::Instance;

ActorEquipmentOverrider::OverrideHandler::Result::Result(OverrideHandlerIdentifierT Source, UInt32 Priority, TESForm* ModelSource, TESModel* OverrideModel)
{
	this->SourceID = Source;
	this->Priority = Priority;
	this->ModelSource = ModelSource;
	this->OverrideModel = OverrideModel;
}

TESForm * ActorEquipmentOverrider::OverrideHandler::Result::GetModelSource() const
{
	return ModelSource;
}

TESModel * ActorEquipmentOverrider::OverrideHandler::Result::GetModel() const
{
	return OverrideModel;
}

bool ActorEquipmentOverrider::OverrideHandler::Result::SortComparator(const Result & LHS, const Result & RHS)
{
	if (LHS.Priority > RHS.Priority)
		return true;
	else if (RHS.Priority > LHS.Priority)
		return false;
	else
	{
		// priorities are equal, sort by ID
		// last registered wins
		return LHS.SourceID > RHS.SourceID;
	}
}


ActorEquipmentOverrider::OverrideHandler::OverrideHandler(OverrideHandlerIdentifierT ID,
														  Script* Handler,
														  TESObjectREFR* Ref /*= NULL*/,
														  TESNPC* NPC /*= NULL*/,
														  TESRace* Race /*= NULL*/,
														  TESForm* EquippedItem /*= NULL*/)
{
	this->ID = ID;
	this->Handler = Handler;
	this->Filters.Reference = Ref;
	this->Filters.NPC = NPC;
	this->Filters.Race = Race;
	this->Filters.EquippedItem = EquippedItem;
}

bool ActorEquipmentOverrider::OverrideHandler::HandleEquipment(ActorBodyModelData * ModelData,
															   TESForm* ModelSource,
															   TESObjectREFR * Ref,
															   TESNPC * NPC,
															   OverrideResultListT & OutResults) const
{
	_MESSAGE("Handler %d | Filters: REF(%08X), NPC(%s %08X), RACE(%s %08X), ITEM(%s %08X)",
			 ID,
			 Filters.Reference ? Filters.Reference->refID : 0,
			 Filters.NPC ? InstanceAbstraction::GetFormName(Filters.NPC) : "",
			 Filters.NPC ? Filters.NPC->refID : 0,
			 Filters.Race ? InstanceAbstraction::GetFormName(Filters.Race) : "",
			 Filters.Race ? Filters.Race->refID : 0,
			 Filters.EquippedItem ? InstanceAbstraction::GetFormName(Filters.EquippedItem) : "",
			 Filters.EquippedItem ? Filters.EquippedItem->refID : 0);
	gLog.Indent();

	bool Result = false;
	TESRace* Race = NPC->race.race;
	UInt32 Female = NPC->actorBaseData.IsFemale();

	// check filters
	bool FiltersMatch = true;
	if (Filters.Reference && Filters.Reference != Ref)
		FiltersMatch = false;
	else if (Filters.NPC && Filters.NPC != NPC)
		FiltersMatch = false;
	else if (Filters.Race && Filters.Race != Race)
		FiltersMatch = false;
	else if (Filters.EquippedItem && Filters.EquippedItem != ModelSource)
		FiltersMatch = false;

	if (FiltersMatch)
	{
		TESForm* OrgModelSource = ModelSource;
		OBSEArrayElement HandlerResult;

		// call function script and extract results
		if (Interfaces::kOBSEScript->CallFunction(Handler, Ref, NULL, &HandlerResult, 4, OrgModelSource, NPC, Race, Female))
		{
			if (HandlerResult.IsValid())
			{
				OBSEArrayVarInterface::Array* ResultData = HandlerResult.Array();
				if (ResultData)
				{
					UInt32 Size = Interfaces::kOBSEArrayVar->GetArraySize(ResultData);
					if (Size == kResultArray__Size)
					{
						OBSEArrayElement Priority, OverrideModelSource;
						Interfaces::kOBSEArrayVar->GetElement(ResultData, kResultArrayIndex_Priority, Priority);
						Interfaces::kOBSEArrayVar->GetElement(ResultData, kResultArrayIndex_OverrideModelSource, OverrideModelSource);

						if (Priority.IsValid() && Priority.GetType() == OBSEArrayElement::kType_Numeric &&
							OverrideModelSource.IsValid() && OverrideModelSource.GetType() == OBSEArrayElement::kType_Form)
						{
							TESForm* Override = OverrideModelSource.Form();
							double ApplyPriority = Priority.Number();
							if (ApplyPriority < 0)
								ApplyPriority = 0;
							else if (ApplyPriority > 100)
								ApplyPriority = 100;

							if (Settings::kEquipmentOverrideCheckOverrideSourceType().i == 0 || Override->typeID == OrgModelSource->typeID)
							{
								TESBipedModelForm* NewBiped = OBLIVION_CAST(Override, TESForm, TESBipedModelForm);
								SME_ASSERT(NewBiped);

								TESModel* NewModel = &NewBiped->bipedModel[0];
								if (Female)
									NewModel = &NewBiped->bipedModel[1];

								if (NewModel->nifPath.m_dataLen != 0)
								{
									OverrideHandler::Result OutData(ID, (UInt32)ApplyPriority, Override, NewModel);
									OutResults.push_back(OutData);
									Result = true;

									_MESSAGE("Added override item with priority %d, model source %s (%08X), model %s",
											 (UInt32)ApplyPriority, InstanceAbstraction::GetFormName(Override), Override->refID, NewModel->nifPath.m_data);

								}
								else
								{
									_MESSAGE("Invalid path for override model source %08X", Override->refID);
								}
							}
							else
							{
								_MESSAGE("Model source type mismatch. Expected %d but received %d", (UInt32)OrgModelSource->typeID, (UInt32)Override->typeID);
							}
						}
						else
						{
							_MESSAGE("Couldn't extract result array arguments. Expected types %d and %d but received %d and %d",
									 (UInt32)OBSEArrayElement::kType_Numeric,
									 (UInt32)OBSEArrayElement::kType_Form,
									 (UInt32)Priority.GetType(), (UInt32)OverrideModelSource.GetType());
						}
					}
					else
					{
						_MESSAGE("Unexpected result array size. Expected %d but received %d", (UInt32)kResultArray__Size, Size);
					}
				}
				else
				{
					_MESSAGE("Handler script returned an invalid result of type %d", (UInt32)HandlerResult.GetType());
				}
			}
			else
			{
				_MESSAGE("Handler script returned no result");
			}
		}
		else
		{
			_MESSAGE("Handler script call failed for script %08X", Handler->refID);
		}
	}
	else
	{
		_MESSAGE("Filter mismatch");
	}

	gLog.Outdent();
	return Result;
}

bool ActorEquipmentOverrider::GetEnabled() const
{
	return Settings::kEquipmentOverrideEnabled().i != 0;
}

ActorEquipmentOverrider::ActorEquipmentOverrider() :
	HandlerTable(),
	NextID(InvalidID + 1),
	OverrideInProgress(false)
{
	;//
}

ActorEquipmentOverrider::~ActorEquipmentOverrider()
{
	ClearHandlers();
}

bool ActorEquipmentOverrider::RegisterHandler(Script* UserFunction,
											  TESObjectREFR* FilterRef,
											  TESNPC* FilterNPC,
											  TESRace* FilterRace,
											  TESForm* FilterEquippeditem,
											  OverrideHandlerIdentifierT& OutID)
{
	bool Result = false;

	if (GetEnabled())
	{
		if (NextID < 0xFFFFFF)
		{
			if (OverrideInProgress == false)
			{
				if (Interfaces::kOBSEScript->IsUserFunction(UserFunction))
				{
					OverrideHandlerIdentifierT NewID = NextID++;
					OverrideHandler NewHandler(NewID, UserFunction, FilterRef, FilterNPC, FilterRace, FilterEquippeditem);
					HandlerTable.insert(std::make_pair(NewID, NewHandler));

					OutID = NewID;
					Result = true;
				}
				else
				{
					_MESSAGE("Couldn't register equipment override handler - Script %08X is not a user-defined function script", UserFunction->refID);
				}
			}
			else
			{
				_MESSAGE("Attempting to register a new equipment override handler while an override operation is in progress");
			}
		}
		else
		{
			_MESSAGE("Equipment override handler limit reached!");
		}
	}


	return Result;
}

bool ActorEquipmentOverrider::UnregisterHandler(OverrideHandlerIdentifierT ID)
{
	bool Result = false;
	if (GetEnabled())
	{
		if (OverrideInProgress == false)
		{
			if (ID != ActorEquipmentOverrider::InvalidID && HandlerTable.count(ID))
			{
				HandlerTable.erase(ID);
				Result = true;
			}
			else
			{
				_MESSAGE("Couldn't unregister equipment override handler - Invalid ID %d", ID);
			}
		}
		else
		{
			_MESSAGE("Attempting to unregister an equipment override handler while an override operation is in progress");
		}
	}


	return Result;
}

void ActorEquipmentOverrider::ClearHandlers()
{
	if (OverrideInProgress == false)
	{
		HandlerTable.clear();
	}
	else
	{
		_MESSAGE("Attempting to clear equipment override handlers while an override operation is in progress");
	}
}

void ActorEquipmentOverrider::ApplyOverride(int BodyPart, ActorBodyModelData * ModelData, TESForm* ModelSource)
{
	if (HandlerTable.size() == 0)
		return;
	else if (GetEnabled() == false)
		return;

	SME_ASSERT(OverrideInProgress == false);
	SME::MiscGunk::ScopedSetter<bool> Sentry(OverrideInProgress, true);

	TESObjectREFR* CurrentRef = ModelData->parentRef;
	SME_ASSERT(CurrentRef && CurrentRef->baseForm && CurrentRef->baseForm->typeID == kFormType_NPC);
	TESNPC* CurrentNPC = OBLIVION_CAST(CurrentRef->baseForm, TESForm, TESNPC);
	TESBipedModelForm* CurrentBiped = OBLIVION_CAST(ModelSource, TESForm, TESBipedModelForm);
	SME_ASSERT(CurrentBiped);

	ActorBodyModelData::BodyPartData* CurrentBodyPart = NULL;
	if (BodyPart == -1
		|| (((CurrentBiped->partMask & (1 << ActorBodyModelData::kBodyPart_LeftRing)) == false)				// need to check for rings additionally as TESNPC::InitWorn passes those as the partID
		&& ((CurrentBiped->partMask & (1 << ActorBodyModelData::kBodyPart_RightRing)) == false)))
	{
		// pick the first part in the part mask, like the engine does
		UInt32 PartID = ActorBodyModelData::kBodyPart_Head;
		while ((PartID == ActorBodyModelData::kBodyPart_Hair &&	(CurrentBiped->partMask & (1 << ActorBodyModelData::kBodyPart_Head)) == true)
			   || (CurrentBiped->partMask & (1 << PartID)) == false)
		{
			PartID++;
			if (PartID >= ActorBodyModelData::kBodyPart__MAX)
			{
				// the biped item has no slots, so bugger off
				return;
			}
		}

		BodyPart = PartID;
	}

	SME_ASSERT(BodyPart >= ActorBodyModelData::kBodyPart_Head && BodyPart < ActorBodyModelData::kBodyPart__MAX);
	CurrentBodyPart = &ModelData->bodyParts[BodyPart];

	_MESSAGE("Attempting to override equipment model for NPC Ref %s (%08X) | Body Part = %d, Source Item = %s (%08X)",
			 InstanceAbstraction::GetFormName(CurrentNPC), CurrentRef->refID, BodyPart, InstanceAbstraction::GetFormName(ModelSource), ModelSource->refID);
	gLog.Indent();

	OverrideHandler::OverrideResultListT Overrides;
	for each (auto Itr in HandlerTable)
	{
		OverrideHandler& ThisHandler = Itr.second;
		ThisHandler.HandleEquipment(ModelData, ModelSource, CurrentRef, CurrentNPC, Overrides);
	}

	if (Overrides.size())
	{
		// sort the overrides and get the top override
		Overrides.sort(OverrideHandler::Result::SortComparator);
		OverrideHandler::Result& FinalOverride = Overrides.front();

		TESForm* NewSource = FinalOverride.GetModelSource();
		TESModel* NewModel = FinalOverride.GetModel();

		CurrentBodyPart->modelSource = NewSource;
		CurrentBodyPart->model = NewModel;

		_MESSAGE("Applied override: Model Source - %s (%08X), Model Path - %s", InstanceAbstraction::GetFormName(NewSource), NewSource->refID, NewModel->nifPath.m_data);
	}
	else
	{
		_MESSAGE("No overrides found");
	}

	gLog.Outdent();
}


_DefineHookHdlr(TESBipedModelFormGetBodyPartModel, 0x00469230);

void __stdcall SwapEquipmentModelData(ActorBodyModelData* ModelData, TESForm* ModelSource, TESModel* Model, int BodyPart)
{
	// call the original function
	thisCall<void>(0x0047AA00, ModelData, ModelSource, Model, BodyPart);
	ActorEquipmentOverrider::Instance.ApplyOverride(BodyPart, ModelData, ModelSource);
}

#define _hhName		TESBipedModelFormGetBodyPartModel
_hhBegin()
{
	_hhSetVar(Retn, 0x00469235);
	__asm
	{
		push	ecx
		call	SwapEquipmentModelData
		jmp		_hhGetVar(Retn)
	}
}



void PatchEquipmentOverride(void)
{
	if (InstanceAbstraction::EditorMode == false)
	{
		_MemHdlr(TESBipedModelFormGetBodyPartModel).WriteJump();
	}
}

namespace EquipmentOverride
{
	void HandleLoadGame(void)
	{
		ActorEquipmentOverrider::Instance.ClearHandlers();
	}
}