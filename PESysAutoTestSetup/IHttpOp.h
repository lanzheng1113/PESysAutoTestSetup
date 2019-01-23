/**
 * \brief 定义与WEB交互的接口，下载文件和GET请求
 */
#pragma once

#include "util/Interface.h"
#include <string>
using std::string;
/**
 * \addgroup CURL_HTTP_OPERATION
 * \{
 */

/**
 * \brief IHttpOp 接口类定义了铠甲自修复程序与WEB交互的接口。
 *
 * IHttpOp 定义了三个接口：
 * <ol>
 * <li>GetRemoteFileSize 获取远程服务器中指定文件的大小</li>
 * <li>DownloadFile 下载文件</li>
 * <li>DoGet 向WEB服务发送GET请求</li>
 * </ol>
 * <b>注1：由于自修复程序的设计中没有使用到POST方法，因此不定义相关的接口</b>
 */
class IHttpOp : Interface
{
public:
	/**
	 * \brief 获取远程服务器中指定文件的大小
	 * \param uri 指定的远程服务器文件路径
	 * \return 远程服务器文件的大小。失败返回-1，否则返回远程服务器文件的大小
	 */
	virtual double GetRemoteFileSize(const string& uri) = 0;
	/**
	 * \brief 下载一个文件到指定的目录
	 * \param uri 下载地址
	 * \param TargetFolder 下载目录
	 * \param [out] TargetName 下载文件。
	 * \return 成功返回true,否则返回false.
	 */
	virtual bool DownloadFile(const string& uri,const string& TargetFolder,string& TargetName) = 0;
	/**
	 * \brief 调用GET方法
	 * \param GET的完成URL
	 * \return 成功返回HTTP服务器返回的数据，否则返回一个空字符串。
	 */
	virtual string DoGet(const string& url) = 0;

	/**
	 * \brief POST数据到指定的URL
	 * 
	 * \param url 数据提交的URL
	 * \param data 提交的数据
	 * \return 成功后返回HTTP服务器返回的数据，否则返回一个空字符串。
	 */
	virtual string DoPost(const string& url, const string& data) = 0;
protected:
private:
};

/**
 * \}
 */
