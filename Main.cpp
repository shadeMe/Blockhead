#include "BlockheadInternals.h"
#include "Commands.h"
#include "HeadOverride.h"
#include "Sundries.h"
#include "BodyOverride.h"
#include "VersionInfo.h"

IDebugLog gLog("Blockhead.log");


static void LoadCallbackHandler(void * reserved)
{
	BodyOverride::HandleLoadGame(true);
	HeadOverride::HandleLoadGame();
}

static void SaveCallbackHandler(void * reserved)
{
	;//
}

static void NewGameCallbackHandler(void * reserved)
{
	BodyOverride::HandleLoadGame(false);
	HeadOverride::HandleLoadGame();
}

void BlockheadMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	if (Msg->type == 'CSEI')
	{
		CSEInterface* Interface = (CSEInterface*)Msg->data;

		Interfaces::kCSEConsole = (CSEConsoleInterface*)Interface->InitializeInterface(CSEInterface::kCSEInterface_Console);
		Interfaces::kCSEIntelliSense = (CSEIntelliSenseInterface*)Interface->InitializeInterface(CSEInterface::kCSEInterface_IntelliSense);

		_MESSAGE("Received interface from CSE");

		Interfaces::kCSEConsole->PrintToConsole("Blockhead", "Registering command URLs ...");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("SetBodyAssetOverride", "http://cs.elderscrolls.com/index.php?title=SetBodyAssetOverride");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("GetBodyAssetOverride", "http://cs.elderscrolls.com/index.php?title=GetBodyAssetOverride");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("ResetBodyAssetOverride", "http://cs.elderscrolls.com/index.php?title=ResetBodyAssetOverride");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("GetFaceGenAge", "http://cs.elderscrolls.com/index.php?title=GetFaceGenAge");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("SetFaceGenAge", "http://cs.elderscrolls.com/index.php?title=SetFaceGenAge");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("SetHeadAssetOverride", "http://cs.elderscrolls.com/index.php?title=SetHeadAssetOverride");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("GetHeadAssetOverride", "http://cs.elderscrolls.com/index.php?title=GetHeadAssetOverride");
		Interfaces::kCSEIntelliSense->RegisterCommandURL("ResetHeadAssetOverride", "http://cs.elderscrolls.com/index.php?title=ResetHeadAssetOverride");
	}
}

void OBSEMessageHandler(OBSEMessagingInterface::Message* Msg)
{
	switch (Msg->type)
	{
	case OBSEMessagingInterface::kMessage_PostLoad:
		Interfaces::kOBSEMessaging->RegisterListener(Interfaces::kOBSEPluginHandle, "CSE", BlockheadMessageHandler);
		_MESSAGE("Registered to receive messages from CSE");

		break;
	case OBSEMessagingInterface::kMessage_PostPostLoad:
		_MESSAGE("Requesting an interface from CSE");
		Interfaces::kOBSEMessaging->Dispatch(Interfaces::kOBSEPluginHandle, 'CSEI', NULL, 0, "CSE");	

		break;
	}
}

extern "C"
{
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		_MESSAGE("Blockhead Initializing...");
		
		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"Blockhead";
		info->version =		PACKED_SME_VERSION;

		Interfaces::kOBSEPluginHandle = obse->GetPluginHandle();
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}	

		InstanceAbstraction::EditorMode = false;

		if (obse->isEditor)
		{
			InstanceAbstraction::EditorMode = true;

			if (obse->editorVersion != CS_VERSION_1_2)
			{
				_MESSAGE("Unsupported editor version %08X", obse->oblivionVersion);
				return false;
			}
		}
		else
		{
			if (obse->oblivionVersion != OBLIVION_VERSION)
			{
				_MESSAGE("Unsupported runtime version %08X", obse->oblivionVersion);
				return false;
			}

			Interfaces::kOBSESerialization = (OBSESerializationInterface *)obse->QueryInterface(kInterface_Serialization);
			if (!Interfaces::kOBSESerialization)
			{
				_MESSAGE("Serialization interface not found");
				return false;
			}

			if (Interfaces::kOBSESerialization->version < OBSESerializationInterface::kVersion)
			{
				_MESSAGE("Incorrect serialization version found (got %08X need %08X)", Interfaces::kOBSESerialization->version, OBSESerializationInterface::kVersion);
				return false;
			}

			Interfaces::kOBSEArrayVar = (OBSEArrayVarInterface*)obse->QueryInterface(kInterface_ArrayVar);
			if (!Interfaces::kOBSEArrayVar)
			{
				_MESSAGE("Array interface not found");
				return false;
			}

			Interfaces::kOBSEScript = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);
			if (!Interfaces::kOBSEScript)
			{
				_MESSAGE("Script interface not found");
				return false;
			}

			Interfaces::kOBSEIO = (OBSEIOInterface*)obse->QueryInterface(kInterface_IO);
			if (Interfaces::kOBSEIO == NULL)
			{
				_MESSAGE("IO interface not found");
				return false;
			}

			Interfaces::kOBSEStringVar = (OBSEStringVarInterface*)obse->QueryInterface(kInterface_StringVar);
			if (Interfaces::kOBSEStringVar == NULL)
			{
				_MESSAGE("String var interface not found");
				return false;
			}
		}

		Interfaces::kOBSEMessaging = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
		if (Interfaces::kOBSEMessaging == NULL)
		{
			_MESSAGE("Messaging interface not found");
			return false;
		}

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		_MESSAGE("Initializing INI Manager");
		BlockheadINIManager::Instance.Initialize("Data\\OBSE\\Plugins\\Blockhead.ini", NULL);

		if (InstanceAbstraction::EditorMode == false)
		{
			Interfaces::kOBSESerialization->SetSaveCallback(Interfaces::kOBSEPluginHandle, SaveCallbackHandler);
			Interfaces::kOBSESerialization->SetLoadCallback(Interfaces::kOBSEPluginHandle, LoadCallbackHandler);
			Interfaces::kOBSESerialization->SetNewGameCallback(Interfaces::kOBSEPluginHandle, NewGameCallbackHandler);
						
			RegisterStringVarInterface(Interfaces::kOBSEStringVar);
		}
		else
		{			
			Interfaces::kOBSEMessaging->RegisterListener(Interfaces::kOBSEPluginHandle, "OBSE", OBSEMessageHandler);
		}

		_MESSAGE("Pah! There's no pleasing some horses!\n\n");
		gLog.Indent();

		
		RegisterCommands(obse);
		PatchHeadOverride();
		PatchBodyOverride();
		PatchSundries();

		return true;
	}

	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
	{
		return TRUE;
	}
};

