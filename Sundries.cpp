#include "Sundries.h"
#include "BodyOverride.h"

_DefineHookHdlr(RaceSexMenuPoser, 0x0040D658);
_DefineJumpHdlr(RaceSexMenuRender, 0x005CE629, 0x005CE650);
_DefineHookHdlr(RaceSexMenuBodyFix, 0x005C8D57);
_DefineHookHdlr(PlayerInventory3DAnimSequenceQueue, 0x0066951A);

void NiMatrix33_Multiply(NiMatrix33* LHS, NiMatrix33* RHS, NiMatrix33* OutResult = NULL)
{
	NiMatrix33 Buffer = {0};
	thisCall<NiMatrix33*>(0x007100A0, LHS, &Buffer, RHS);

	if (OutResult == NULL)
		memcpy(LHS, &Buffer, sizeof(NiMatrix33));
	else
		memcpy(OutResult, &Buffer, sizeof(NiMatrix33));
}

void NiMatrix33_DebugDump(NiMatrix33* Matrix, const char* Name)
{
	_MESSAGE("NiMatrix33 %s Dump:", Name);
	gLog.Indent();

	char Buffer[0x100] = {0};

	for (int i = 0; i < 9; i += 3)
	{
		FORMAT_STR(Buffer, "(%0.3f       %0.3f       %0.3f)", Matrix->data[i], Matrix->data[i + 1], Matrix->data[i + 2]);
		_MESSAGE(Buffer);
	}

	gLog.Outdent();
	_MESSAGE("\n");
}

void __stdcall PoseFace(void)
{
	if (InterfaceManager::GetSingleton())
	{
		if (InterfaceManager::GetSingleton()->MenuModeHasFocus(kMenuType_RaceSex) == false)
			return;
		else if (IsConsoleOpen())
			return;

		bool UpArrowDown = Interfaces::kOBSEIO->IsKeyPressed(0xC8) || Interfaces::kOBSEIO->IsKeyPressed(0x11);
		bool DownArrowDown = Interfaces::kOBSEIO->IsKeyPressed(0xD0) || Interfaces::kOBSEIO->IsKeyPressed(0x1F);
		bool LeftArrowDown = Interfaces::kOBSEIO->IsKeyPressed(0xCB) || Interfaces::kOBSEIO->IsKeyPressed(0x1E);
		bool RightArrowDown = Interfaces::kOBSEIO->IsKeyPressed(0xCD) || Interfaces::kOBSEIO->IsKeyPressed(0x20);
		bool ShiftKeyDown = Interfaces::kOBSEIO->IsKeyPressed(0x2A) || Interfaces::kOBSEIO->IsKeyPressed(0x36);
		bool TabKeyDown = Interfaces::kOBSEIO->IsKeyPressed(0x0F);
		
		if (UpArrowDown == false && DownArrowDown == false &&
			ShiftKeyDown == false && TabKeyDown == false &&
			LeftArrowDown == false && RightArrowDown == false)
		{
			return;
		}

		OSInputGlobals* InputManager = (*g_osGlobals)->input;

		SME_ASSERT((*g_worldSceneGraph)->m_children.numObjs > 0);

		NiNode* WorldCameraRoot = (NiNode*)((SceneGraph*)(*g_worldSceneGraph))->m_children.data[0];

		NiMatrix33* CameraRootWorldRotate = &WorldCameraRoot->m_worldRotate;
		NiMatrix33* CameraRootLocalRotate = &WorldCameraRoot->m_localRotate;

		Vector3* CameraRootWorldTranslate = (Vector3*)&WorldCameraRoot->m_worldTranslate;
		Vector3* CameraRootLocalTranslate = (Vector3*)&WorldCameraRoot->m_localTranslate;

		if (UpArrowDown || DownArrowDown)
		{
			float MovementMultiplier = Settings::kRaceMenuPoserMovementSpeed.GetData().f;
			if (DownArrowDown)
				MovementMultiplier *= -1;

			Vector3 Offset(CameraRootWorldRotate->data[1], CameraRootWorldRotate->data[4], CameraRootWorldRotate->data[7]);
			Offset.Scale(MovementMultiplier);

			*CameraRootWorldTranslate += Offset;
			*CameraRootLocalTranslate += Offset;
		}
		
		if (LeftArrowDown || RightArrowDown)
		{
			float MovementMultiplier = Settings::kRaceMenuPoserMovementSpeed.GetData().f;
			if (LeftArrowDown)
				MovementMultiplier *= -1;

			Vector3 Offset(CameraRootWorldRotate->data[0], CameraRootWorldRotate->data[3], CameraRootWorldRotate->data[6]);
			Offset.Scale(MovementMultiplier);

			*CameraRootWorldTranslate += Offset;
			*CameraRootLocalTranslate += Offset;
		}

		if (ShiftKeyDown)
		{
			float RotationMultiplier = Settings::kRaceMenuPoserRotationSpeed.GetData().f;
			DIMOUSESTATE2* MouseState = &InputManager->unk1B20.mouseState;
			
			if (MouseState->lX || MouseState->lY)
			{
				NiMatrix33 Buffer = {0}, MulResult = {0};

				float XAlpha = (MouseState->lX / 100.0f * RotationMultiplier * 0.5f) * 1.0f;
				float YAlpha = (MouseState->lY / 100.0f * RotationMultiplier * 0.5f) * 1.0f;

				thisCall<void>(0x0070FDD0, &Buffer, XAlpha);		// initialize rotation transform matrices
				NiMatrix33_Multiply(&Buffer, CameraRootWorldRotate, &MulResult);

				thisCall<void>(0x0070FD30, &Buffer, YAlpha);
				NiMatrix33_Multiply(&MulResult, &Buffer);

				memcpy(CameraRootLocalRotate, &MulResult, sizeof(NiMatrix33));
			}
		}	

		thisCall<void>(0x00707370, WorldCameraRoot, 0.0f, 1);		// traverse and update
	}
}

#define _hhName		RaceSexMenuPoser
_hhBegin()
{
	_hhSetVar(Retn, 0x0040D65D);
	_hhSetVar(Call, 0x0040C830);
	__asm
	{
		pushad
		call	PoseFace
		popad

		call	_hhGetVar(Call)
		jmp		_hhGetVar(Retn)
	}
}

void __stdcall DoRaceSexMenuBodyFixHook(RaceSexMenu* Menu)
{
	BodyOverride::FixPlayerBodyModel();

	float Unk8A0 = *(float*)((UInt32)Menu + 0x8A0);
	thisCall<void>(0x005C2BF0, Menu, 1.0, Unk8A0 / 5.0);	// update the camera
}

#define _hhName		RaceSexMenuBodyFix
_hhBegin()
{
	_hhSetVar(Retn, 0x005C8D5C);
	_hhSetVar(Call, 0x00519D20);
	__asm
	{
		pushad
		push	esi
		call	DoRaceSexMenuBodyFixHook
		popad

		call	_hhGetVar(Call)
		jmp		_hhGetVar(Retn)
	}
}


enum
{
	kInventoryIdle_Idle		= 0,
	kInventoryIdle_H2H,
	kInventoryIdle_H2HTorch,
	kInventoryIdle_1H,
	kInventoryIdle_1HTorch,
	kInventoryIdle_2H,
	kInventoryIdle_2HTorch,
	kInventoryIdle_Staff,
	kInventoryIdle_StaffTorch,
	kInventoryIdle_Bow,
	kInventoryIdle_BowTorch,

	kInventoryIdle__MAX
};

void __stdcall OverrideInventoryIdles(void* AnimationSeqHolder, tList<char>* Idles, NiNode* AnimNode, TESObjectREFR* AnimRef)
{
	static const SME::INI::INISetting*			kOverrideNames[kInventoryIdle__MAX] =
	{
		NULL,
		&Settings::kInventoryIdleOverridePath_HandToHandIdle,
		&Settings::kInventoryIdleOverridePath_HandToHandTorchIdle,
		&Settings::kInventoryIdleOverridePath_OneHandIdle,
		&Settings::kInventoryIdleOverridePath_OneHandTorchIdle,
		&Settings::kInventoryIdleOverridePath_TwoHandIdle,
		NULL,
		&Settings::kInventoryIdleOverridePath_StaffIdle,
		NULL,
		&Settings::kInventoryIdleOverridePath_BowIdle,
		NULL
	};

	static const char*		kInventoryIdleNames[kInventoryIdle__MAX] = 
	{
		"Idle.kf",
		"HandToHandIdle.kf",
		"HandToHandTorchIdle.kf",
		"OneHandIdle.kf",
		"OneHandTorchIdle.kf",
		"TwoHandIdle.kf",
		"TwoHandTorchIdle.kf",
		"StaffIdle.kf",
		"StaffTorchIdle.kf",
		"BowIdle.kf",
		"BowTorchIdle.kf"
	};

#ifndef NDEBUG
	_MESSAGE("Overriding inventory idles...");
	gLog.Indent();
#endif

	int IdleNameCounter = 0;
	for (tList<char>::Iterator Itr = Idles->Begin(); Itr.End() == false && Itr.Get(); ++Itr)
	{
		std::string FileName = strrchr(Itr.Get(), '\\') + 1;
		
		bool ValidIdle = false;
		while (IdleNameCounter < kInventoryIdle__MAX)
		{
			const char* Current = kInventoryIdleNames[IdleNameCounter];
			IdleNameCounter++;
			
			if (!_stricmp(FileName.c_str(), Current))
			{
				ValidIdle = true;
				break;
			}
		}

		if (ValidIdle == false)
		{
			SME_ASSERT((++Itr).End() == true);		// the current idle node has to be the last
			break;									// since they are added in sequence
		}

		const SME::INI::INISetting* OverrideName = kOverrideNames[IdleNameCounter - 1];
		if (OverrideName == NULL || strlen(OverrideName->GetData().s) < 4)
			continue;								// skip to the next idle path for those without overrides

#ifndef NDEBUG
		_MESSAGE("Attempting override for idle %s...", kInventoryIdleNames[IdleNameCounter - 1]);
		gLog.Indent();
#endif // !NDEBUG

		std::string Path = "Meshes\\" + std::string(Itr.Get());
		Path.erase(Path.rfind("\\") + 1);
		Path += std::string(OverrideName->GetData().s);

		if (InstanceAbstraction::FileFinder::GetFileExists(Path.c_str()))
		{
#ifndef NDEBUG
			_MESSAGE("Idle path %s switched to %s", Itr.Get(), Path.c_str());
#endif // !NDEBUG

			sprintf_s(Itr.Get(), 0x104, "%s", Path.c_str());
		}
		else
		{
#ifndef NDEBUG
			_MESSAGE("Override path %s invalid", Path.c_str());
#endif // !NEDEBUG
		}

#ifndef NDEBUG
		gLog.Outdent();
#endif // !NDEBUG
	}

#ifndef NDEBUG
	gLog.Outdent();
#endif // !NDEBUG


	thisCall<void>(0x00475D80, AnimationSeqHolder, Idles, AnimNode, AnimRef);
}

#define _hhName		PlayerInventory3DAnimSequenceQueue
_hhBegin()
{
	_hhSetVar(Retn, 0x0066951F);
	__asm
	{
		push	ecx
		call	OverrideInventoryIdles

		jmp		_hhGetVar(Retn)
	}
}


void PatchSundries( void )
{
	if (InstanceAbstraction::EditorMode == false && Settings::kRaceMenuPoserEnabled.GetData().i)
	{
		_MemHdlr(RaceSexMenuPoser).WriteJump();
		_MemHdlr(RaceSexMenuRender).WriteJump();
		_MemHdlr(RaceSexMenuBodyFix).WriteJump();
	}

	if (InstanceAbstraction::EditorMode == false && Settings::kInventoryIdleOverrideEnabled.GetData().i)
	{
		_MemHdlr(PlayerInventory3DAnimSequenceQueue).WriteJump();
	}
}
