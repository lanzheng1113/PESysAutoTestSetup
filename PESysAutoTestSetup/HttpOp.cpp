#include "HttpOp.h"
#include "Httper.h"

CHttpOp::CHttpOp(void)
{

}

CHttpOp::~CHttpOp(void)
{

}

static string getFileNameFromURL( const string& strUrl )
{
	string x(strUrl);
	string::size_type xPos = x.find_last_of('/');
	if(xPos == std::string::npos || xPos == x.length()-1)
	{
		return "";
	}
	else
	{
		x = x.substr(xPos+1,x.length());
	}
	return x;
}

static void AddBackSlash( string& strPath)
{
	string x(strPath);
	char EndOne = x[x.length()-1];
	if (!(EndOne == '\\' || EndOne == '/'))
	{
		strPath += "\\";
	}
}

bool CHttpOp::DownloadFile( const string& uri,const string& TargetFolder,string& TargetName )
{
	std::vector<std::string> httpHead;
	string name = getFileNameFromURL(uri);
	if (name.empty())
	{
		return false;
	}
	TargetName = name;

	string strFullPath = TargetFolder;
	AddBackSlash(strFullPath);
	strFullPath += name;

	CHttpRs rs = CHttper::download(uri,httpHead,strFullPath);
	if (rs.m_bSucess)
		return true;
	else
		return false;
}

string CHttpOp::DoGet( const string& url )
{
	CHttpRs rs = CHttper::get(url);
	if (rs.m_bSucess)
	{
		return rs.m_strResult;
	}
	else
		return "";
}

string CHttpOp::DoPost(const string& url, const string& data)
{
	CHttpRs rs = CHttper::post(url, data);
	if (rs.m_bSucess)
	{
		return rs.m_strResult;
	}
	else
		return "";
}

extern unsigned int g_TotalContentLen;
double CHttpOp::GetRemoteFileSize( const string& uri )
{
	double dlen = -1;
	CHttpRs rs = CHttper::getDownloadFileLenth(uri.c_str(),&dlen);
	g_TotalContentLen = dlen;
	return dlen;
}
