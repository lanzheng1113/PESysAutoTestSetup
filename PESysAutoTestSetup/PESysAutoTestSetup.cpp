#include <windows.h>
#include <stdio.h>
#include "util/path.h"
#include <string>
using std::string;
using std::wstring;
#include "util/File.h"
#include "json/reader.h"
#include "json/value.h"
#include "HttpOp.h"
#include "util/OSVersion.h"
#include "util/MD5Checksum.h"
#include "util/StringEx.h"
#include <map>
using std::map;
#include "BCD.h"
#include <Shlwapi.h>
/**
 * \brief �����ļ��Ը���
 */
int configure_file_update()
{
	string text = "";
	string strConfigFile = Path::getApplicationDirPath();
	strConfigFile += "conf.json";
	if (PathFileExistsA(strConfigFile.c_str()))
	{
		CHttpOp opt;
		text = opt.DoGet("http://192.168.1.139/ATFrameWork/conf.json");
	}
	else
	{
		FileReader fr(strConfigFile);
		if (!fr.open())
		{
			printf("�������ļ������˴�����ȷ���ļ��Ƿ���ڻ��߱�ռ�á�\n");
			return 1;
		}
		text = fr.read();
		fr.close();
	}


	Json::Reader jr;
	Json::Value jv;
	if (!jr.parse(text, jv))
	{
		printf("����������ļ�\n");
		return 1;
	}
	if (jv.isMember("conf_update") && jv["conf_update"].isObject())
	{
		Json::Value jv_conf_update = jv["conf_update"];
		if (jv_conf_update.isMember("url"))
		{
			string update_url = jv_conf_update["url"].asString();
			CHttpOp opt;
			string value = opt.DoGet(update_url);
			if (value == "")
			{
				printf("%s�����˴����ֵ\n", update_url.c_str());
				return 2;
			}
			if (!jr.parse(value, jv))
			{
				printf("%s���ص�ֵ�Ǵ���ĸ�ʽ\n", update_url.c_str());
				return 3;
			}
			// д�����µ�ֵ
			FileWriter fw(strConfigFile);
			if (!fw.open())
			{
				printf("��д���µ����÷����˴���\n");
				return 12;
			}
			fw.write(value);
			return 0;
		}
		else
			return 5;
	}
	else
	{
		printf("����������ļ�\n");
		return 4;
	}
}

typedef struct tagPEAutoTestConfigure
{
	string pe_name;
	string pe_wim_url;
	string pe_md5;
	string sdi_url;
	string sdi_md5;
	string boot_entry_name;
	string boot_entry_guid;
	string boot_device_guid;
}PEAutoTestConfigure;

bool auto_test_configure_read(PEAutoTestConfigure* pConf)
{
	string strConfigFile = Path::getApplicationDirPath();
	strConfigFile += "conf.json";
	FileReader fr(strConfigFile);
	if (!fr.open())
	{
		printf("�������ļ�ʧ����\n");
		return false;
	}
	string text = fr.read();
	fr.close();
	Json::Reader jr;
	Json::Value jv;
	if (!jr.parse(text, jv))
	{
		printf("����������ļ�\n");
		return false;
	}
	pConf->boot_device_guid = jv["boot_device_guid"].asString();
	pConf->boot_entry_guid  = jv["boot_entry_guid"].asString();
	pConf->boot_entry_name  = jv["boot_entry_name"].asString();
	pConf->pe_md5     = jv["pe_wim_md5"].asString();
	pConf->pe_name    = jv["pe_name"].asString();
	pConf->pe_wim_url = jv["pe_wim_url"].asString();
	pConf->sdi_md5    = jv["sdi_md5"].asString();
	pConf->sdi_url    = jv["sdi_url"].asString();

	if (pConf->boot_device_guid.empty()
		|| pConf->boot_entry_guid.empty()
		|| pConf->boot_entry_name.empty()
		|| pConf->pe_md5.empty()
		|| pConf->pe_name.empty()
		|| pConf->pe_wim_url.empty()
		|| pConf->sdi_md5.empty()
		|| pConf->sdi_url.empty())
	{
		return false;
	}
	return true;
}

//����
BOOL ReBootComputer()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		//������ʧ��
		return FALSE;
	}
	// Get the LUID for the shutdown privilege.
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;  // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	// Get the shutdown privilege for this process.
	AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		return FALSE; //�ػ�ʧ��
	}
	// Shut down the system and force all applications to close.
	ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);

	return TRUE;
}

bool file_update(const string& file_name, const string& file_md5, const string& file_url)
{
	string pe_wim_file_path = Path::getApplicationDirPath();
	pe_wim_file_path += file_name;
	printf("���Ժ�����У��%s��MD5\n",pe_wim_file_path.c_str());
	wstring wstrMD5 = CMD5Checksum::GetMD5(String(pe_wim_file_path).toStdWString());
	string strMd5 = String::fromStdWString(wstrMD5);
	if (String(strMd5).toUpperCase() == String(file_md5).toUpperCase())
	{
		printf("�ļ���MD5У��ͨ��������Ҫ����.\n");
		return true;
	}
	else
	{
		printf("�����ļ�����\n");
		DeleteFileA(pe_wim_file_path.c_str());
		CHttpOp opt;
		string target_name;
		if (opt.DownloadFile(file_url, Path::getApplicationDirPath(), target_name))
		{
			string download_full = Path::getApplicationDirPath() + target_name;
			if (_stricmp(download_full.c_str(), pe_wim_file_path.c_str()))
			{
				MoveFileA(download_full.c_str(), pe_wim_file_path.c_str());
			}
			printf("������ɣ�����У��MD5\n");
			wstring wstrMD5 = CMD5Checksum::GetMD5(String(pe_wim_file_path).toStdWString());
			string strMd5 = String::fromStdWString(wstrMD5);
			if (String(strMd5).toUpperCase() == String(file_md5).toUpperCase())
			{
				printf("�ļ���MD5У��ͨ�����ɹ�����.\n");
				return true;
			}
			else
			{
				printf("�����ص��ļ�У��ʧ�ܣ������ԡ�\n");
				return false;
			}
		}
		else
		{
			printf("�����ļ�%sʧ����\n", file_url.c_str());
			return false;
		}
	}
}

map<wstring, BCD::BootLoaderEntry> BootEntrys;
std::map<std::wstring, BCD::DeviceAddtionOption> DeviceAddtionOptions;
int GetEntrys(map<wstring, BCD::BootLoaderEntry>&, std::map<std::wstring, BCD::DeviceAddtionOption>&);
VOID ReconstructBCD(const wstring& SDIFullPath, const wstring& WIMFullPath, const wstring& GUIDDevice, const wstring& GUIDBootLdr, const wstring& BootEntryTitle);

bool boot_entry_update(const string& pe_name, 
	const string& boot_entry_name, 
	const string& boot_entry_guid, 
	const string& boot_device_guid)
{
	GetEntrys(BootEntrys, DeviceAddtionOptions);
	bool need_del_device = false;
	bool need_del_boot_entry = false;
	for (auto i : DeviceAddtionOptions)
	{
		wprintf(L"device id: %s\n", i.first.c_str());
		string str_device_id = String::fromStdWString(i.first);
		if (_stricmp(str_device_id.c_str(),boot_device_guid.c_str()) == 0)
		{
			need_del_device = true;
			break;
		}
	}
	for (auto i : BootEntrys)
	{
		wprintf(L"BootEntrys id: %s\n", i.second.id.c_str());
		string boot_entry_id = String::fromStdWString(i.second.id);
		if (0 == _stricmp(boot_entry_id.c_str(), boot_entry_guid.c_str()))
		{
			need_del_boot_entry = true;
			break;
		}
	}

	string sdi_full = Path::getApplicationDirPath() + "BOOT.SDI";
	string wim_full = Path::getApplicationDirPath() + pe_name;

	ReconstructBCD(String(sdi_full).toStdWString(), 
		String(wim_full).toStdWString(), 
		String(boot_device_guid).toStdWString(), 
		String(boot_entry_guid).toStdWString(),
		String(boot_entry_name).toStdWString());

	return true;
}

int main(int argc, char** argv)
{
	int iRet = 0;
	CoInitialize(NULL);
	do 
	{
		if (!IsOsWindowsVistaorLater())
		{
			printf("�������֧��Vista֮���windowsϵͳ��");
			iRet = 911;
			break;
		}

		int i = 0, iMaxCount = 5;
		for (i = 0; i != iMaxCount; i++)
		{
			if (0 != configure_file_update())
			{
				printf("�����ļ��Ը���ʧ�ܣ�2�������[%d-5]����\n", i);
				Sleep(2000);
			}
			else
				break;
		}
		if (i == iMaxCount)
		{
			printf("���������ļ�����\n");
			iRet = 10;
			break;
		}

		PEAutoTestConfigure petc;
		if (!auto_test_configure_read(&petc))
		{
			printf("��ȡ�����ļ�ʧ�ܣ�\n");
			iRet = 1;
			break;
		}

		if (!file_update(petc.pe_name, petc.pe_md5, petc.pe_wim_url))
		{
			printf("����/����PE-WIMʧ��\n");
			iRet = 1;
			break;
		}

		if (!file_update("BOOT.SDI", petc.sdi_md5, petc.sdi_url))
		{
			printf("����/����SDIʧ����\n");
			iRet = 1;
			break;
		}

		printf("����ʹ��BCDEdit����BOOTMGR�����˵�\n");
		if (!boot_entry_update(petc.pe_name, petc.boot_entry_name, petc.boot_entry_guid, petc.boot_device_guid))
		{
			printf("����������ʧ����\n");
			iRet = 18;
			break;
		}
		printf("���,������������\n");
		ReBootComputer();
	} while (FALSE);
	
	CoUninitialize();
	return iRet;
}
/*
{
"conf_update":
	{
		"url" : "http://192.168.1.139/ATFrameWork/conf.json"
	},
"pe_name" : "PE64.WIM",
"pe_wim_url": "http://192.168.1.139/ATFrameWork/PE64.WIM",
"pe_wim_md5": "986206E5DE747D73254C7B435B6D3553",
"sdi_url" : "http://192.168.1.139/ATFrameWork/BOOT.SDI",
"sdi_md5" : "9106857D1B8712BA3FEE8A4BACE8B9E9"
"boot_entry_name" : "PE TEST X64",
"boot_entry_guid" : "{CF10CE22-E86A-476B-A239-143CCD43F954}"
"boot_device_guid" : "{7456BB63-5FCD-4BB6-B415-D08C10F51790}"
}
*/