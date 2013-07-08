#include "BlockheadInternals.h"
#include "VersionInfo.h"

extern "C"
{
	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		_MESSAGE("Blockhead Initializing...");
		
		info->infoVersion =	PluginInfo::kInfoVersion;
		info->name =		"Blockhead";
		info->version =		PACKED_SME_VERSION;

		g_pluginHandle = obse->GetPluginHandle();

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
		else if (obse->oblivionVersion != OBLIVION_VERSION)
		{
			_MESSAGE("Unsupported runtime version %08X", obse->oblivionVersion);
			return false;
		}
		else if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		_MESSAGE("Initializing INI Manager");
		BlockheadINIManager::Instance.Initialize("Data\\OBSE\\Plugins\\Blockhead.ini", NULL);

		_MESSAGE("Pah! There's no pleasing some horses!\n\n");
		gLog.Indent();

		BlockHeads();

		return true;
	}

	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
	{
		return TRUE;
	}
};

