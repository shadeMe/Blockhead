#include "AnimationOverride.h"

ActorAnimationOverrider			ActorAnimationOverrider::Instance;
const std::string				ActorAnimationOverrider::kOverrideFileTag = "_BLKD_";

UInt32 ActorAnimationOverrider::GetSpecialAnims( TESNPC* NPC, const char* Tag, AnimationFileListT& OutFiles ) const
{
	SME_ASSERT(NPC && Tag);

	std::string SkeletonPath = (NPC->model.nifPath.m_data ? NPC->model.nifPath.m_data : "");
	int Count = 0;

	if (SkeletonPath.size())
	{
		SkeletonPath.erase(SkeletonPath.rfind("\\"));
		OutFiles.clear();

		std::string SourceDir = "Data\\Meshes\\" + SkeletonPath + "\\SpecialAnims\\";;
		std::string SearchFilter = "*" + kOverrideFileTag + std::string(Tag) + ".kf";

		for (IDirectoryIterator Itr(SourceDir.c_str(), SearchFilter.c_str()); Itr.Done() == false; Itr.Next())
		{
			OutFiles.push_back(Itr.Get()->cFileName);
			Count++;
		}
	}

	return Count;
}

void ActorAnimationOverrider::ClearOverrides( TESNPC* NPC ) const
{
	SME_ASSERT(NPC);

	TESAnimation* AnimData = &NPC->animation;
	if (AnimData->data.animationName || AnimData->data.next)
	{
		std::vector<const char*> Delinquents;
		for (TESAnimation::AnimationNode* Itr = &AnimData->data; Itr && Itr->Info(); Itr = Itr->Next())
		{
			std::string File(Itr->Info()), Comparand(kOverrideFileTag);

			SME::StringHelpers::MakeLower(File);
			SME::StringHelpers::MakeLower(Comparand);

			if (File.find(Comparand) != std::string::npos)
			{
				// our override animation, queue it for removal
				Delinquents.push_back(Itr->Info());
			}
		}

		for (std::vector<const char*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
		{
			DEBUG_MESSAGE("Override Disabled - %s", *Itr);

			AnimationVisitor Visitor(&AnimData->data);
			Visitor.RemoveString(*Itr);
		}
	}
}

void ActorAnimationOverrider::ApplyOverrides( TESNPC* NPC, AnimationFileListT& Files ) const
{
	SME_ASSERT(NPC);

	TESAnimation* AnimData = &NPC->animation;
	for (AnimationFileListT::iterator Itr = Files.begin(); Itr != Files.end(); Itr++)
	{
		AnimationVisitor Visitor(&AnimData->data);
		char* Path = const_cast<char*>(Itr->c_str());

		if (!Visitor.FindString(Path))
		{
			UInt32 Len = strlen(Path);
			TESAnimation::AnimationNode* NewNode = (TESAnimation::AnimationNode*)FormHeap_Allocate(sizeof(TESAnimation::AnimationNode));
			NewNode->animationName = (char*)FormHeap_Allocate(Len + 1);
			strcpy_s(NewNode->animationName, Len + 1, Path);
			NewNode->next = NULL;

			Visitor.Append(NewNode);

			DEBUG_MESSAGE("Override Enabled - %s", Path);
		}
	}
}

void ActorAnimationOverrider::Override( TESNPC* NPC ) const
{
	SME_ASSERT(NPC);

#ifndef NDEBUG
	DEBUG_MESSAGE("Attempting to override Animations for NPC %08X...", NPC->refID);
	gLog.Indent();
#endif // !NDEBUG

	ClearOverrides(NPC);

	if (GetBlacklisted(NPC))
	{
#ifndef NDEBUG
		DEBUG_MESSAGE("Blacklisted, skipping...");
#endif // !NDEBUG
	}
	else
	{
		TESRace* Race = InstanceAbstraction::GetNPCRace(NPC);
		if (Race == NULL)
		{
#ifndef NDEBUG
			DEBUG_MESSAGE("No race?! The gall! We are not amused, not the slightest!");
#endif // !NDEBUG
		}
		else
		{
			const char* RaceName = InstanceAbstraction::GetFormName(Race);
			const char* GenderPath = NULL;
			if (InstanceAbstraction::GetNPCFemale(NPC))
				GenderPath = "F";
			else
				GenderPath = "M";

			UInt32 FormID = NPC->refID & 0x00FFFFFF;
			TESFile* Plugin = InstanceAbstraction::GetOverrideFile(NPC, 0);

			if (Settings::kAnimOverridePerRace.GetData().i)
			{
				if (RaceName && strlen(RaceName) > 2)
				{
					char Buffer[0x200] = {0};
					FORMAT_STR(Buffer, "PERRACE_%s_%s", RaceName, GenderPath);

					AnimationFileListT Overrides;
					if (GetSpecialAnims(NPC, Buffer, Overrides))
					{
#ifndef NDEBUG
						DEBUG_MESSAGE("Per-Race:");
						gLog.Indent();
#endif // !NDEBUG
						ApplyOverrides(NPC, Overrides);
#ifndef NDEBUG
						gLog.Outdent();
#endif // !NDEBUG
					}
				}
			}

			if (Settings::kAnimOverridePerNPC.GetData().i && Plugin)
			{
				char Buffer[0x200] = {0};
				FORMAT_STR(Buffer, "PERNPC_%s_%08X", Plugin->name, FormID);

				AnimationFileListT Overrides;
				if (GetSpecialAnims(NPC, Buffer, Overrides))
				{
#ifndef NDEBUG
					DEBUG_MESSAGE("Per-NPC:");
					gLog.Indent();
#endif // !NDEBUG
					ApplyOverrides(NPC, Overrides);
#ifndef NDEBUG
					gLog.Outdent();
#endif // !NDEBUG
				}
			}
		}
	}

#ifndef NDEBUG
	gLog.Outdent();
#endif // !NDEBUG
}

ActorAnimationOverrider::ActorAnimationOverrider() :
	Blacklist()
{
	;//
}

ActorAnimationOverrider::~ActorAnimationOverrider()
{
	ClearBlacklist();
}

void ActorAnimationOverrider::AddToBlacklist( TESNPC* NPC )
{
	SME_ASSERT(NPC);

	if (std::find(Blacklist.begin(), Blacklist.end(), NPC->refID) == Blacklist.end())
		Blacklist.push_back(NPC->refID);
}

void ActorAnimationOverrider::RemoveFromBlacklist( TESNPC* NPC )
{
	SME_ASSERT(NPC);

	NPCListT::const_iterator Match = std::find(Blacklist.begin(), Blacklist.end(), NPC->refID);
	if (Match != Blacklist.end())
		Blacklist.erase(Match);
}

void ActorAnimationOverrider::ClearBlacklist( void )
{
	Blacklist.clear();
}

bool ActorAnimationOverrider::GetBlacklisted( TESNPC* NPC ) const
{
	SME_ASSERT(NPC);
	return std::find(Blacklist.begin(), Blacklist.end(), NPC->refID) != Blacklist.end();
}

_DefineHookHdlr(TESObjectREFRRefreshAnimData, 0x004E34AB);

void __stdcall FixupAnimationOverrides(TESObjectREFR* Ref)
{
	SME_ASSERT(Ref);

	TESForm* BaseForm = Ref->baseForm;
	if (BaseForm)
	{
		TESNPC* NPC = OBLIVION_CAST(BaseForm, TESForm, TESNPC);
		if (NPC)
			ActorAnimationOverrider::Instance.Override(NPC);
	}
}

#define _hhName		TESObjectREFRRefreshAnimData
_hhBegin()
{
	_hhSetVar(Retn, 0x004E34B2);
	__asm
	{
		mov     [esp + 0x21C], eax

		pushad
		push	ecx
		call	FixupAnimationOverrides
		popad

		jmp		_hhGetVar(Retn)
	}
}

void PatchAnimationOverride( void )
{
	if (InstanceAbstraction::EditorMode == false)
	{
		_MemHdlr(TESObjectREFRRefreshAnimData).WriteJump();
	}
}

namespace AnimOverride
{
	void HandleLoadGame( void )
	{
		ActorAnimationOverrider::Instance.ClearBlacklist();
	}
}