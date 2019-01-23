#include <map>
#include <string>
#include <sstream>
#include "BCD.h"
#include <strsafe.h>
#include <fstream>
#include "resource.h"
#include <Shlwapi.h>
#include <shellapi.h>

#pragma comment(lib,"shlwapi.lib")

using std::wstring;
using std::map;

BOOL GetVolumePaths(LPCWSTR VolumeName, wstring &strPath);

extern map<wstring, BCD::BootLoaderEntry> BootEntrys;
extern std::map<std::wstring, BCD::DeviceAddtionOption> DeviceAddtionOptions;

std::wstring BootEntryToString(const BCD::BootLoaderEntry& bble)
{
	std::wstring ret;
	ret = std::wstring(L"id: ") + bble.id + L"\r\n";
	ret += std::wstring(L"device: ") + bble.device + L"\r\n";
	if (bble.device_type == BCD::BCDDeviceData::RamdiskDevice
		&& !DeviceAddtionOptions[bble.device_option_guid].partition.empty())
	{
		wstring wstrDosPath;
		if (GetVolumePaths(DeviceAddtionOptions[bble.device_option_guid].partition.c_str(), wstrDosPath)
			&& !wstrDosPath.empty())
		{
			ret += L"    partition=[" + wstrDosPath + L"]\r\n";
		}
		else
		{
			ret += L"    " + DeviceAddtionOptions[bble.device_option_guid].partition + L"\r\n";
		}


		ret += L"    " + DeviceAddtionOptions[bble.device_option_guid].sdipath + L"\r\n";
	}
	ret += std::wstring(L"path: ") + bble.path + L"\r\n";
	ret += std::wstring(L"description: ") + bble.description + L"\r\n";
	ret += std::wstring(L"locale: ") + bble.locale + L"\r\n";
	for (std::wstring i : bble.inherit)
	{
		ret += std::wstring(L"inherit: ") + i + L"\r\n";
	}
	for (std::wstring i : bble.recoverysequence)
	{
		ret += std::wstring(L"recoverysequence: ") + i + L"\r\n";
	}
	ret += std::wstring(L"recoveryenabled: ") + (bble.recoveryenabled ? L"True" : L"False") + L"\r\n";
	ret += std::wstring(L"osdevice: ") + bble.osdevice + L"\r\n";
	ret += std::wstring(L"systemroot: ") + bble.systemroot + L"\r\n";
	if (!bble.resumeobject.empty())
	{
		ret += std::wstring(L"resumeobject: ") + bble.resumeobject;
	}
	return ret;
}

std::wstring GetSDIFullPath(const BCD::BootLoaderEntry& bble)
{
	ATLASSERT(bble.device_type == BCD::BCDDeviceData::RamdiskDevice);
	ATLASSERT(!DeviceAddtionOptions[bble.device_option_guid].partition.empty());
	wstring wstrDosPath;
	if (!GetVolumePaths(DeviceAddtionOptions[bble.device_option_guid].partition.c_str(), wstrDosPath) 
		|| wstrDosPath.empty())
	{
		return L"";
	}
	//wstrDosPath格式如同C:
	wstrDosPath += DeviceAddtionOptions[bble.device_option_guid].sdipath;
	return wstrDosPath;
}

std::wstring GetWMIFullPath(const BCD::BootLoaderEntry& bble)
{
	ATLASSERT(bble.device_type == BCD::BCDDeviceData::RamdiskDevice);
	ATLASSERT(!DeviceAddtionOptions[bble.device_option_guid].partition.empty());
	wstring wstrDosPath;
	if (!GetVolumePaths(DeviceAddtionOptions[bble.device_option_guid].partition.c_str(), wstrDosPath)
		|| wstrDosPath.empty())
	{
		return L"";
	}
	wstrDosPath += bble.osdevice;
	return wstrDosPath;
}

std::wstring GetDeviceID(const BCD::BootLoaderEntry& bble)
{
	ATLASSERT(bble.device_type == BCD::BCDDeviceData::RamdiskDevice);
	return bble.device_option_guid;
}

std::wstring GetBootLdrID(const BCD::BootLoaderEntry& bble)
{
	ATLASSERT(bble.device_type == BCD::BCDDeviceData::RamdiskDevice);
	return bble.id;
}



static BOOL GetVolumePaths(LPCWSTR VolumeName, wstring &strPath)
{
	WCHAR szVolumeName[MAX_PATH] = { 0 };
	WCHAR DosDeviceName[3] = { 0,L':',0 };
	for (WCHAR i = L'C'; i != L'Z'; i++)
	{
		DosDeviceName[0] = i;
		memset(szVolumeName, 0, sizeof(szVolumeName));
		QueryDosDevice(DosDeviceName, szVolumeName, _countof(szVolumeName));
		if (0 == _wcsicmp(VolumeName, szVolumeName))
		{
			strPath = DosDeviceName;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CallCmd(const std::wstring& strExeFile, const std::wstring& strExeParam, BOOL bShow);
BOOL Bcdedit(const std::wstring& strExeParam);

bool ExportResFile(const wchar_t * szPathOut, UINT nID, const wchar_t * szResType, HMODULE hModule/*=NULL*/)	//释放资源文件
{
	//ATLTRACE("ExportResFile [%s]", CS2SC(szPathOut));
	bool bRet = false;
	HRSRC hResInfo = NULL;
	HGLOBAL hResLoad = NULL;
	LPVOID lpRes = NULL;
	const wchar_t * szResName = MAKEINTRESOURCE(nID);
	try
	{
		hResInfo = FindResourceW(hModule, szResName, szResType);
		if (!hResInfo)
		{
			ATLTRACE("ExtractResource FindResourceW");
			return false;
		}

		hResLoad = LoadResource(hModule, hResInfo);
		if (!hResLoad)
		{
			ATLTRACE("ExtractResource LoadResource");
			return false;
		}

		lpRes = LockResource(hResLoad);
		if (!lpRes)
		{
			ATLTRACE("ExtractResource LockResource");
			return false;
		}

		//-----------------------------------
		//创建文件，并将资源数据写入
		std::ofstream out_file(szPathOut, std::ios::out | std::ios::binary);
		if (!out_file)
		{
			ATLTRACE("ExtractResource 文件创建失败");
			return FALSE;
		}

		DWORD dwResSize = SizeofResource(hModule, hResInfo);
		if (out_file.write(static_cast<const char *>(lpRes), dwResSize))
		{
			bRet = true;
		}
		else
		{
			bRet = false;
		}
	}
	catch (std::exception & e)
	{
		ATLTRACE(e.what());
		return false;
	}
	catch (...)
	{
		ATLTRACE("ExtractResource中捕捉到一个异常");
		return false;
	}

	return bRet;
}

static BOOL Bcdedit(const std::wstring& strExeParam)
{
	WCHAR szTempPath[MAX_PATH] = { 0 };
	GetTempPath(_countof(szTempPath), szTempPath);
	PathAddBackslash(szTempPath);
	wcscat_s(szTempPath, L"bcdedit.exe");
	ExportResFile(szTempPath, IDR_EXE_BCDEDIT, L"EXE", NULL);
	return CallCmd(szTempPath, strExeParam, FALSE);
}

static BOOL CallCmd(const std::wstring& strExeFile, const std::wstring& strExeParam, BOOL bShow)
{
	if (strExeFile.size() < 3)
		return FALSE;

	int iShow;
	if (!bShow)
	{
		iShow = SW_HIDE;
	}
	else
	{
		iShow = SW_SHOW;
	}
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = strExeFile.c_str();
	ShExecInfo.lpParameters = strExeParam.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = iShow;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteExW(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	DWORD dwExitCode = -1;
	if (GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode))
	{
		if (dwExitCode == 0)
			return TRUE;
	}
	return FALSE;
}

static std::wstring ErrorMessageW(LPCWSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code
	std::wstring ret;
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(wcslen((LPCWSTR)lpMsgBuf) + wcslen((LPCWSTR)lpszFunction) + 40) * sizeof(WCHAR));

	StringCchPrintfW((LPWSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(WCHAR),
		L"%s发生错误%d:%s",
		lpszFunction, dw, lpMsgBuf);

	//MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
	ret = (LPCWSTR)lpDisplayBuf;

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);

	return ret;
}

// 从一个根目录移动到另一个根目录
static BOOL MoveFileWithSameFolder(const std::wstring& from, WCHAR to)
{
	if (from.empty() || 0 == to)
	{
		return FALSE;
	}
	//必须是DOS格式
	if (from.length() < 2
		|| from[1] != L':')
	{
		return FALSE;
	}
	if (from[0] == to)
	{
		//来源和目的是同一个目录，直接返回TRUE。这种情况下我们只重做一次bcd。
		return TRUE;
	}
	wstring toFull = from;
	toFull[0] = to;

	//
	// (1) Create directory "toFull" recursively.
	//
	size_t off = toFull.find(L'\\');
	if (off == wstring::npos)
	{
		return FALSE;
	}
	// 跳过根目录，再执行一次find'\\'
	do
	{
		off = toFull.find(L"\\", off + 1);
		if (off != wstring::npos)
		{
			wstring str = toFull.substr(0, off);
			if (!PathFileExists(str.c_str()))
			{
				if (!CreateDirectory(str.c_str(), NULL))
				{
					std::wstringstream wss;
					wss << L"创建目录[" << str << L"]时";
					MessageBox(NULL, ErrorMessageW(wss.str().c_str()).c_str(), NULL, MB_OK);
					return FALSE;
				}
			}
		}
	} while (off != wstring::npos && off < toFull.length());

	//
	// (2) Move file, from ==> to
	//
	if (!MoveFile(from.c_str(), toFull.c_str()))
	{
		std::wstringstream wss;
		wss << L"移动文件[" << from << L"]时";
		MessageBox(NULL, ErrorMessageW(wss.str().c_str()).c_str(), NULL, MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL MoveRamdiskFiles(const wstring& SDIFullPath, const wstring& WIMFullPath, WCHAR PartitionMoveTo)
{
	return MoveFileWithSameFolder(SDIFullPath, PartitionMoveTo) 
		&& MoveFileWithSameFolder(WIMFullPath, PartitionMoveTo);
}

VOID ReconstructBCD(const wstring& SDIFullPath, 
	const wstring& WIMFullPath,
	const wstring& GUIDDevice,
	const wstring& GUIDBootLdr,
	const wstring& BootEntryTitle)
{
	// 创建“设备选项”
	Bcdedit(L"/set {default} bootmenupolicy legacy");
	Bcdedit(L"/delete " + GUIDDevice);
	Bcdedit(L"/create " + GUIDDevice + L" /device");
	Bcdedit(L"/set " + GUIDDevice + L" ramdisksdidevice partition=" + SDIFullPath.substr(0, 2));
	Bcdedit(L"/set " + GUIDDevice + L" ramdisksdipath " + SDIFullPath.substr(2));

	// 创建“启动加载器”
	Bcdedit(L"/delete " + GUIDBootLdr);
	Bcdedit(L"/create " + GUIDBootLdr + L" /d \"" + BootEntryTitle + L"\" /application OSLOADER");
	Bcdedit(L"/set " + GUIDBootLdr + L" device ramdisk=[" + WIMFullPath.substr(0, 2) + L"]" + WIMFullPath.substr(2) + L"," + GUIDDevice);
	Bcdedit(L"/set " + GUIDBootLdr + L" osdevice ramdisk=[" + WIMFullPath.substr(0, 2) + L"]" + WIMFullPath.substr(2) + L"," + GUIDDevice);
	Bcdedit(L"/set " + GUIDBootLdr + L" systemroot \\windows");
	Bcdedit(L"/set " + GUIDBootLdr + L" detecthal yes");
	Bcdedit(L"/set " + GUIDBootLdr + L" winpe yes");
	Bcdedit(L"/ems " + GUIDBootLdr + L" off");
	Bcdedit(L"/displayorder " + GUIDBootLdr + L" /addlast");
	Bcdedit(L"/default " + GUIDBootLdr);
	return;
}
