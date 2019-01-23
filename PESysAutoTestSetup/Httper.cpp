#include "Httper.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <io.h>    
#include <iostream>
#include <sstream>
#include <fstream>

//#include "FunAssistant.h"
//#include "FileAssistant.h"


using namespace std;

CHttpRs* CHttpRs::g_curlFailed = nullptr;

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) 
{
	string data((const char*)ptr, (size_t)size * nmemb);
	*((stringstream*)stream) << data << endl;
	return size * nmemb;
}

void SetProgressTip(LPCWSTR Message)
{
	wprintf(L"%s\r",Message);
}

unsigned int g_DownloadLen = 0;
unsigned int g_TotalContentLen = 0;

size_t write_file(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	g_DownloadLen += size*nmemb;
	cout << "Bytes write " << g_DownloadLen << "\r";
	static DWORD lastTick = GetTickCount();
	if (GetTickCount() - lastTick > 333 ||
		g_DownloadLen == g_TotalContentLen)
	{
		lastTick = GetTickCount();
		std::wstringstream wss;
		wss << L"正在从服务器下载文件信息。" << g_DownloadLen/1024 << "KB/" << g_TotalContentLen/1024 << "KB" ;
		SetProgressTip(wss.str().c_str());
	}
	return written;
}
//////////////////////////////////////////////////////////////////////////


CHttpRs* CHttpRs::CurlFailed()
{
	if (nullptr == g_curlFailed) 
	{
		g_curlFailed = new CHttpRs(false, "");
	}

	return g_curlFailed;
}

CHttpRs::CHttpRs()
{
	m_bSucess = false;
}

CHttpRs::CHttpRs( bool suc, const std::string& rs ) 
	: m_bSucess(suc)
	, m_strResult(rs)
{

}

CHttpRs::~CHttpRs()
{

}


//////////////////////////////////////////////////////////////////////////
void CHttper::globalInit()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void CHttper::globalClean()
{
	curl_global_cleanup();
}

CHttpRs CHttper::get(const std::string& url)
{
	std::stringstream out;
	/*url域名解析部分*/
	string Name = url;
	string::size_type Site = Name.find("//");
	string::size_type Site2;
	if (Site != string::npos)
	{
		Site = Site + 2;
		Site2 = Name.find('/', Site + 1);
		if (Site2 != string::npos)
			Name = Name.substr(Site, Site2 - Site);
		else
			Name = Name.substr(Site, Name.length() - 1);
	}
	else
	{
		Site = 0;
		Site2 = Name.find('/', Site + 1);
		if (Site2 != string::npos)
			Name = Name.substr(Site, Site2);
	}

	std::string hoststring = Name;//"Host:bb.201061.com";
	std::vector<std::string> vl;
	vl.push_back(hoststring);
	CMyCurl curl = normalCurl(url, vl, &out);
	if (nullptr == curl.m_pObjCurl) 
	{
		return *CHttpRs::CurlFailed();
	}
	return request(&curl, &out);
}

CHttpRs CHttper::post(const std::string& url, const std::string& postFields)
{
	std::stringstream out;
	CMyCurl mycurl = normalCurl(url, std::vector <std::string>(), &out);
	if (nullptr == mycurl.m_pObjCurl) {
		return *CHttpRs::CurlFailed();
	}

	//curl_easy_setopt(mycurl.m_pObjCurl, CURLOPT_PROXY, "127.0.0.1:8888");//不需要抓包时，去掉这一行

	curl_easy_setopt(mycurl.m_pObjCurl, CURLOPT_POST, 1);
	curl_easy_setopt(mycurl.m_pObjCurl, CURLOPT_POSTFIELDS, postFields.c_str());

	return request(&mycurl, &out);
}

CHttpRs CHttper::request(CMyCurl *mycurl, std::stringstream* rs)
{
	CScopedMyCurl clean(mycurl);
	CURLcode res = curl_easy_perform(mycurl->m_pObjCurl);
	if (res != CURLE_OK) 
	{
		return CHttpRs(false, curl_easy_strerror(res));
	}
	return CHttpRs(true, rs->str());
}

CHttpRs CHttper::download(const std::string& url, const std::vector<std::string>& headers, const std::string& outfilename)
{
	g_DownloadLen = 0;
	FILE *fp = fopen(outfilename.c_str(), "wb");
	if (!fp)
	{
		printf("create file failed with error:%d\n",GetLastError());
		return CHttpRs(false,"create download file failed.");
	}

	CURL *curl = curl_easy_init();
	if (nullptr == curl) {
		return *CHttpRs::CurlFailed();
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");

	struct curl_slist *chunk = NULL;
	std::for_each(headers.begin(), headers.end(), [&](const std::string& header)
	{
		chunk = curl_slist_append(chunk, header.c_str());
	});

	CScopedMyCurl clean(&CMyCurl(curl, chunk));
	CURLcode res = curl_easy_perform(curl);
	fclose(fp);
	if (res != CURLE_OK) {
		return CHttpRs(false, curl_easy_strerror(res));
	}

	return CHttpRs(true, "");
}

CMyCurl CHttper::normalCurl(const std::string& url, const std::vector<std::string>& headers, std::stringstream* out)
{
	CURL *curl = curl_easy_init();
	if (nullptr == curl) {
		return CMyCurl(nullptr,nullptr);
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, g_timeout);

	struct curl_slist *chunk = NULL;
	std::for_each(headers.begin(), headers.end(), [&](const std::string& header)
	{
		chunk = curl_slist_append(chunk, header.c_str());
	});
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

	return CMyCurl(curl, chunk);
}

CScopedMyCurl::CScopedMyCurl( CMyCurl *mycurl ) 
	: m_ObjCurl(mycurl)
{

}

CScopedMyCurl::~CScopedMyCurl()
{
	if (nullptr != m_ObjCurl->m_pObjCurl) {
		curl_easy_cleanup(m_ObjCurl->m_pObjCurl);
	}
	if (nullptr != m_ObjCurl->m_pCurlChunkList) {
		curl_slist_free_all(m_ObjCurl->m_pCurlChunkList);
	}
}

static size_t save_header(void *ptr, size_t size, size_t nmemb, void *data)
{
	return (size_t)(size * nmemb);
}

CHttpRs CHttper::getDownloadFileLenth(const char *url,OUT double* length)
{
	double len = -1;
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_HEADER, 1);    //只要求header头
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1);    //不需求body
	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, save_header);
	if (curl_easy_perform(handle) == CURLE_OK) 
	{
		if(CURLE_OK == curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &len))
		{
			;
		}
		else
		{
			return CHttpRs(false,"curl_easy_getinfo failed");
		}
	}
	else
	{
		return CHttpRs(false,"curl_easy_perform failed");
	}
	*length = len;
	return CHttpRs(true, "");
}

CMyCurl::CMyCurl( CURL *curl, struct curl_slist *chunk ) 
	: m_pObjCurl(curl)
	, m_pCurlChunkList(chunk)
{

}
