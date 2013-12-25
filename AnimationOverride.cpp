#include "AnimationOverride.h"

const std::string			ActorAnimationOverrider::kOverrideFilePrefix = "BLKD_";

UInt32 ActorAnimationOverrider::GetSpecialAnims( TESNPC* NPC, const char* Prefix, AnimationFileListT& OutFiles )
{
	SME_ASSERT(NPC && Prefix);

	std::string SkeletonPath = (NPC->model.nifPath.m_data ? NPC->model.nifPath.m_data : "");
	int Count = 0;

	if (SkeletonPath.size())
	{
		SkeletonPath.erase(SkeletonPath.rfind("\\"));
		OutFiles.clear();

		std::string SourceDir = "Data\\Meshes\\" + SkeletonPath + "\\SpecialAnims\\";;
		std::string SearchFilter = kOverrideFilePrefix + std::string(Prefix) + "*.kf";

		for (IDirectoryIterator Itr(SourceDir.c_str(), SearchFilter.c_str()); Itr.Done() == false; Itr.Next())
		{
#ifndef NDEBUG
	//		_MESSAGE("Override Found - %s", Itr.Get()->cFileName);
#endif // !NDEBUG

			OutFiles.push_back(Itr.Get()->cFileName);
			Count++;
		}
	}

	return Count;
}

void ActorAnimationOverrider::ClearOverrides( TESNPC* NPC )
{
	SME_ASSERT(NPC);

	TESAnimation* AnimData = &NPC->animation;
	if (AnimData->data.animationName || AnimData->data.next)
	{
		std::vector<const char*> Delinquents;
		for (TESAnimation::AnimationNode* Itr = &AnimData->data; Itr && Itr->Info(); Itr = Itr->Next())
		{
			std::string File(Itr->Info()), Comparand(kOverrideFilePrefix);

			SME::StringHelpers::MakeLower(File);
			SME::StringHelpers::MakeLower(Comparand);

			if (File.find(Comparand) == 0)
			{
				// our override animation, queue it for removal
				Delinquents.push_back(Itr->Info());
			}
		}

		for (std::vector<const char*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
		{
#ifndef NDEBUG
			_MESSAGE("Override Disabled - %s", *Itr);
#endif // !NDEBUG

			AnimationVisitor Visitor(&AnimData->data);
			Visitor.RemoveString(*Itr);
		}
	}
}

void ActorAnimationOverrider::ApplyOverrides( TESNPC* NPC, AnimationFileListT& Files )
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

#ifndef NDEBUG
			_MESSAGE("Override Enabled - %s", Path);
#endif // !NDEBUG
		}
	}
}

void ActorAnimationOverrider::Override( TESNPC* NPC )
{
	SME_ASSERT(NPC);

#ifndef NDEBUG
	_MESSAGE("Attempting to override Animations for NPC %08X...", NPC->refID);
	gLog.Indent();
#endif // !NDEBUG

	TESRace* Race = InstanceAbstraction::GetNPCRace(NPC);
	SME_ASSERT(Race);

	const char* RaceName = InstanceAbstraction::GetFormName(Race);
	const char* GenderPath = NULL;
	if (InstanceAbstraction::GetNPCFemale(NPC))
		GenderPath = "F";
	else
		GenderPath = "M";

	UInt32 FormID = NPC->refID & 0x00FFFFFF;
	TESFile* Plugin = InstanceAbstraction::GetOverrideFile(NPC, 0);

	ClearOverrides(NPC);

	if (Settings::kAnimOverridePerRace.GetData().i)
	{
		if (RaceName && strlen(RaceName) > 2)
		{
			char Buffer[0x200] = {0};
			FORMAT_STR(Buffer, "PERRACE_%s_%s_", RaceName, GenderPath);

			AnimationFileListT Overrides;
			if (GetSpecialAnims(NPC, Buffer, Overrides))
			{
#ifndef NDEBUG
				_MESSAGE("Per-Race:");
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
		FORMAT_STR(Buffer, "PERNPC_%s_%08X_", Plugin->name, FormID);

		AnimationFileListT Overrides;
		if (GetSpecialAnims(NPC, Buffer, Overrides))
		{
#ifndef NDEBUG
			_MESSAGE("Per-NPC:");
			gLog.Indent();
#endif // !NDEBUG
			ApplyOverrides(NPC, Overrides);
#ifndef NDEBUG
			gLog.Outdent();
#endif // !NDEBUG
		}
	}

#ifndef NDEBUG
	gLog.Outdent();
#endif // !NDEBUG
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
		{
			ActorAnimationOverrider::Override(NPC);
		}
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


