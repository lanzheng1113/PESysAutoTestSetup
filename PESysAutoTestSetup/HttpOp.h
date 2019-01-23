#pragma once
#include "ihttpop.h"

/**
 * \addgroup CURL_HTTP_OPERATION
 */

class CHttpOp :
	public IHttpOp
{
public:
	CHttpOp(void);
	~CHttpOp(void);
public:
	virtual double GetRemoteFileSize(const string& uri);
	virtual bool DownloadFile(const string& uri,const string& TargetFolder,string& TargetName);
	virtual string DoGet(const string& url);
	virtual string DoPost(const string& url, const string& data);
};

/**
 * \}
 */
