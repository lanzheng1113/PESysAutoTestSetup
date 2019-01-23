#include <stdio.h>
#include "BCD.h"
using namespace BCD;
#include <string>
#include <sstream>
using namespace std;
#include <map>

string GetBCDObjectTypeName(int BCDObjectTypeId)
{
	static map<int, string> MapOfBCDObjectTypeName = {
		{ BCDObject::GlobalSettings,   "Windows Boot Manager" },
		{ BCDObject::WindowsLoader,    "Windows Boot Loader" },
		{ BCDObject::HibernateResumer, "Resume from Hibernate" },
		{ BCDObject::BootApplication,  "Custom boot application, such as Memory Tester" },
		{ BCDObject::LegacyOSLoader,   "Windows Legacy OS Loader" },
		{ BCDObject::ModuleSettings,   "EMS Settings, Debugger Settings, RAM defects" },
		{ BCDObject::BootLdrSettings,  "Boot Loader Settings" },
		{ BCDObject::ResumeLdrSettings,"Resume Loader Settings" },
		{ BCDObject::DeviceAddOptions, "Device addition options."},
	};
	
	map<int, string>::const_iterator it = MapOfBCDObjectTypeName.find(BCDObjectTypeId);
	if ( it != MapOfBCDObjectTypeName.end())
	{
		return it->second;
	}
	else
	{
		stringstream ss;
		ss << "TypeID-0x" << hex << BCDObjectTypeId;
		return ss.str();
	}	
}


//!NOT thread safe.It returns a static string within the function.
const char* fmt(const char* msg)
{
	if (strlen(msg) >= 25)
	{
		return msg;
	}
	static char ret_msg[26] = { 0 };
	strcpy(ret_msg, msg);
	int len = strlen(ret_msg);
	for (int i = len; i != sizeof(ret_msg); i++)
	{
		ret_msg[i] = ' ';
	}
	ret_msg[25] = 0;
	return ret_msg;
}

void DumpBootMgr(BCDObject& obj)
{
	/*
	Windows 启动管理器
	--------------------
	标识符                  {bootmgr}
	device                  partition=\Device\HarddiskVolume6
	path                    \EFI\MICROSOFT\BOOT\BOOTMGFW.EFI
	description             Windows Boot Manager
	locale                  zh-CN
	inherit                 {globalsettings}
	default                 {current}
	resumeobject            {7d7db1a5-58c5-11e8-b1a5-900dd9c5a7d3}
	displayorder            {current}
	toolsdisplayorder       {memdiag}
	timeout                 5
	*/
	printf("WWindows 启动管理器\n");
	printf("--------------------\n");
	{
		printf("%s%ls\n", fmt("标识符"), obj.GetID().c_str());
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryDevice_ApplicationDevice);
		if (ele.Valid())
		{
			BCDDeviceData data = ele.ToDeviceData();
			if (data.Valid())
			{
				printf("%s%ls\n", fmt("device"), data.GetPartitionPath().c_str());
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_ApplicationPath);
		if (ele.Valid())
			printf("%s%ls\n", fmt("path"), ele.ToString().c_str());
	}
	{
		BCDElement e1 = obj.GetElement(BCD::BcdLibraryString_Description);
		if (e1.Valid())
		{
			printf("%s%ls\n", fmt("description"), e1.ToString().c_str());
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_PreferredLocale);
		if (ele.Valid())
			printf("%s%ls\n", fmt("locale"), ele.ToString().c_str());
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdLibraryObjectList_InheritedObjects);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					if (i == 0)
					{
						printf("%s%ls\n", fmt("inherit"), oblist[i].c_str());
					}
					else
					{
						printf("%s%ls\n", fmt(""), oblist[i].c_str());
					}
				}
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdBootMgrObject_DefaultObject);
		if (ele.Valid())
		{
			printf("%s%ls\n", fmt("default"), ele.ToString().c_str());
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdBootMgrObject_ResumeObject);
		if (ele.Valid())
			printf("%s%ls\n", fmt("resumeobject"), ele.ToString().c_str());
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdBootMgrObjectList_DisplayOrder);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					if (i == 0)
					{
						printf("%s%ls\n", fmt("displayorder"), oblist[i].c_str());
					}
					else
					{
						printf("%s%ls\n", fmt(""), oblist[i].c_str());
					}
				}
			}
		}
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdBootMgrObjectList_ToolsDisplayOrder);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					if (i == 0)
					{
						printf("%s%ls\n", fmt("toolsdisplayorder"), oblist[i].c_str());
					}
					else
					{
						printf("%s%ls\n", fmt(""), oblist[i].c_str());
					}
				}
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdBootMgrInteger_Timeout);
		if (ele.Valid())
			printf("%s%I64u\n",fmt("timeout"),ele.ToInteger());
	}
}

BootLoaderEntry GetWinLoader(BCDObject& obj)
{
	BootLoaderEntry bl;
	bl.id = obj.GetID();
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryDevice_ApplicationDevice);
		if (ele.Valid())
		{
			BCDDeviceData data = ele.ToDeviceData();
			if (data.Valid())
			{
				bl.device = data.GetPartitionPath();
				bl.device_type = data.GetDeviceType();
				if (bl.device_type == BCDDeviceData::RamdiskDevice)
				{
					bl.device_option_guid = data.GetAdditionalOptions();
				}
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_ApplicationPath);
		if (ele.Valid())
		{
			bl.path = ele.ToString();
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_Description);
		if (ele.Valid())
		{
			bl.description = ele.ToString();
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_PreferredLocale);
		if (ele.Valid())
		{
			bl.locale = ele.ToString();
		}
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdLibraryObjectList_InheritedObjects);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					bl.inherit.push_back(oblist[i]);
				}
			}
		}
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdLibraryObjectList_RecoverySequence);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					bl.recoverysequence.push_back(oblist[i]);
				}
			}
		}
	}
	{
		bl.recoveryenabled = false;
		BCDElement ele = obj.GetElement(BCD::BcdLibraryBoolean_AutoRecoveryEnabled);
		if (ele.Valid())
		{
			bl.recoveryenabled = ele.ToBoolean();
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdOSLoaderDevice_OSDevice);
		if (ele.Valid())
		{
			BCDDeviceData data = ele.ToDeviceData();
			if (data.Valid())
			{
				bl.osdevice = data.GetPartitionPath();
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdOSLoaderString_SystemRoot);
		if (ele.Valid())
		{
			bl.systemroot = ele.ToString();
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdOSLoaderObject_AssociatedResumeObject);
		if (ele.Valid())
		{
			bl.resumeobject = ele.ToString();
		}
	}
	return bl;
}

void DumpWinLoader(BCDObject& obj)
{
	/*
	Windows 启动加载器
	-------------------
	标识符                  {current}
	device                  partition=C:
	path                    \WINDOWS\system32\winload.efi             12000002
	description             Windows 10                                12000004
	locale                  zh-CN                                     12000005
	inherit                 {bootloadersettings}                      14000006
	recoverysequence        {7d7db1a7-58c5-11e8-b1a5-900dd9c5a7d3}    14000008
	displaymessageoverride  Recovery
	recoveryenabled         Yes
	isolatedcontext         Yes
	allowedinmemorysettings 0x15000075
	osdevice                partition=C:
	systemroot              \WINDOWS
	resumeobject            {7d7db1a5-58c5-11e8-b1a5-900dd9c5a7d3}
	nx                      OptIn
	bootmenupolicy          Legacy
	*/


	printf("Windows 启动加载器\n");
	printf("--------------------\n");
	{
		printf("%s%ls\n", fmt("标识符"), obj.GetID().c_str());
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryDevice_ApplicationDevice);
		if (ele.Valid())
		{
			BCDDeviceData data = ele.ToDeviceData();
			if (data.Valid())
			{
				printf("%s%ls\n", fmt("device"), data.GetPartitionPath().c_str());
			}
		}
	}

	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_ApplicationPath);
		if (ele.Valid())
		{
			printf("%s%ls\n", fmt("path"), ele.ToString().c_str());
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_Description);
		if (ele.Valid())
		{
			printf("%s%ls\n", fmt("description"), ele.ToString().c_str());
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryString_PreferredLocale);
		if (ele.Valid())
		{
			printf("%s%ls\n", fmt("locale"), ele.ToString().c_str());
		}
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdLibraryObjectList_InheritedObjects);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					if (i == 0)
					{
						printf("%s%ls\n", fmt("inherit"), oblist[i].c_str());
					}
					else
					{
						printf("%s%ls\n", fmt(""), oblist[i].c_str());
					}
				}
			}
		}
	}
	{
		BCDElement e2 = obj.GetElement(BCD::BcdLibraryObjectList_RecoverySequence);
		if (e2.Valid())
		{
			BCDObjectList oblist = e2.GetObjectList();
			if (oblist.Valid())
			{
				for (unsigned i = 0; i != oblist.GetElementCount(); i++)
				{
					if (i == 0)
					{
						printf("%s%ls\n", fmt("recoverysequence"), oblist[i].c_str());
					}
					else
					{
						printf("%s%ls\n", fmt(""), oblist[i].c_str());
					}
				}
			}
		}
	}
	//15000066 displaymessageoverride (Element REG_BINARY 0x00000000 00000003)-Recovery
	{
		BCDElement ele = obj.GetElement(BCD::BcdLibraryBoolean_AutoRecoveryEnabled);
		if (ele.Valid())
		{
			printf("%s%s\n", fmt("recoveryenabled"), ele.ToBoolean()?"Yes":"No");
		}
	}
	//16000060 isolatedcontext (Element REG_BINARY 01)-Yes
	//17000077 allowedinmemorysettings (Element REG_BINARY 0x00000000 15000075)-0x15000075
	//21000001 BcdOSLoaderDevice_OSDevice
	{
		BCDElement ele = obj.GetElement(BCD::BcdOSLoaderDevice_OSDevice);
		if (ele.Valid())
		{
			BCDDeviceData data = ele.ToDeviceData();
			if (data.Valid())
			{
				printf("%s%ls\n", fmt("osdevice"), data.GetPartitionPath().c_str());
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdOSLoaderString_SystemRoot);
		if (ele.Valid())
		{
			printf("%s%ls\n", fmt("systemroot"), ele.ToString().c_str());
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdOSLoaderObject_AssociatedResumeObject);
		if (ele.Valid())
		{
			printf("%s%ls\n", fmt("resumeobject"), ele.ToString().c_str());
		}
	}
	// -- BcdOSLoaderInteger_NxPolicy                      = 0x25000020,
	//25000020 nx (Element REG_BINARY 0x00000000 00000000)-OptIn
	//250000c2 bootmenupolicy (Element REG_BINARY 0x00000000 00000000)-Legacy
	printf("\n");
}

DeviceAddtionOption GetDeviceAddtionOption(BCDObject& obj)
{
	DeviceAddtionOption x;
	x.id = obj.GetID();
	{//if(1)
		BCDElement ele = obj.GetElement(BCD::BcdDeviceInteger_SdiDevice);
		if (ele.Valid())
		{
			BCDDeviceData dt = ele.ToDeviceData();
			if (dt.Valid())
			{
				x.type = dt.GetDeviceType();
				x.partition = dt.GetPartitionPath();
			}
		}
	}
	{
		BCDElement ele = obj.GetElement(BCD::BcdDeviceInteger_SdiPath);
		if (ele.Valid())
		{
			x.sdipath = ele.ToString();
		}
	}
	return x;
}


int GetEntrys(map<wstring, BCD::BootLoaderEntry>& BootEntrys, 
	std::map<std::wstring, DeviceAddtionOption>& DeviceAddtionOptions)
{
	BootEntrys.clear();
	DeviceAddtionOptions.clear();

	BCDStore store = BCDStore::OpenStore();
	for each(BCDObject obj in store.GetObjects())
	{
		switch (obj.GetType())
		{
		case BCDObject::GlobalSettings:
			//DumpBootMgr(obj);
			break;
		case BCDObject::WindowsLoader:
		{
			BootLoaderEntry x = GetWinLoader(obj);
			BootEntrys[x.description] = x;
			break;
		}
		case BCDObject::DeviceAddOptions:
		{
			DeviceAddtionOption x = GetDeviceAddtionOption(obj);
			if (!x.id.empty())
			{
				DeviceAddtionOptions[x.id] = x;
			}
			break;
		}
		default:
			break;
		}
	}

	return 0;
}