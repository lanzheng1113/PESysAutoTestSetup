/**
 * \file Httper.h 
 * \brief 定义了CURL HTTP操作封装类CHttpRs、CMyCurl、CScopedMyCurl、CHttper
 * \author unknow
 * \date 2016/11/23
 * \Version 1.0
 */
#pragma once

#include <string>
#include <vector>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/curlbuild.h>
#include "util/interface.h"

/**
 * \defgroup CURL_HTTP_OPERATION HTTP请求
 * \{
 */

/**
 * \class CHttpRs
 * \brief http操作的结果
 */
class CHttpRs
{
public:
	CHttpRs();
	~CHttpRs();
	explicit CHttpRs(bool suc, const std::string& rs);
	CHttpRs& operator = (const CHttpRs& result)
	{
		m_bSucess = result.m_bSucess;
		m_strResult = result.m_strResult;
		return *this;
	}
public:
	/**
	 * \brief 返回一个代表操作失败的CHttpRs指针。
	 * \return 代表操作失败的CHttpRs指针
	 * \remark 不要去delete它！它返回的是一个全局的对象。
	 */
	static CHttpRs* CurlFailed();

public:
	bool m_bSucess;				///< 操作是否成功的标志
	std::string m_strResult;	///< http操作返回值。注意编码！

private:
	static CHttpRs* g_curlFailed; ///< 静态对象，代表CURL操作失败。
};

/**
 * \class CMyCurl
 * \brief 封装CURL对象，通过CMyCurl执行CURL操作。
 */
class CMyCurl
{
public:
	explicit CMyCurl(CURL *curl, struct curl_slist *chunk);

public:
	CURL *m_pObjCurl;
	struct curl_slist *m_pCurlChunkList;
};

/**
 * \class CScopedMyCurl
 * \brief CMyCurl的资源包装类
 * 
 * 用它封装一个CMyCurl的对象指针，当对象被销毁时执行curl_easy_cleanup和curl_slist_free_all
 * 资源包装类一般用于在一个作用域内自动释放资源，例如
 * \code
 * {
 *     CMyCurl ObjMyCurl = normalCurl(...); //创建一个ObjMyCurl
 *     CScopedMyCurl AutoCleanResourceHelper(&ObjMyCurl); 
 *     //...do something with ObjMyCurl.
 * }
 * //离开AutoCleanResourceHelper这个对象的作用域时，C++编译器保证AutoCleanResourceHelper的析构函数被调用。
 * //在~CScopedMyCurl()中 curl_easy_cleanup和curl_slist_free_all被调用。释放掉curl资源。
 * \endcode
 */
class CScopedMyCurl : public Interface
{
public:
	explicit CScopedMyCurl(CMyCurl *mycurl);
	virtual ~CScopedMyCurl();
private:
	CMyCurl *m_ObjCurl;
};

/**
 * \class CHttper
 * \brief 封装执行CURL http请求的操作。
 * 
 * 使用方法：
 * 在进程开始的地方执行一次CHttper::globalInit，并在进程退出前执行一次globalClean
 * \code
 * int APIENTRY _tWinMain(HINSTANCE hInstance,
 * HINSTANCE hPrevInstance,
 * LPTSTR    lpCmdLine,
 * int       nCmdShow)
 * {
 *     //程序其他初始化
 *     //...
 *     CHttper::globalInit();
 *     //...
 *     //主消息循环:
 *     while (GetMessage(&msg, NULL, 0, 0))
 *     {
 *         if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
 *         {
 *             TranslateMessage(&msg);
 *             DispatchMessage(&msg);
 *         }
 *     }
 *     CHttper::globalClean();
 * }
 * \endcode
 *
 * 当执行GET请求时：
 * \code
 * CHttpRs rs = CHttper::get("http://www.google.com.cn");
 * if (rs.m_bSucess)
 *    cout << ":-) The result of GET request is : " << endl;
 * else
 *    cout << ":-( The Http GET request was failed with error : " << endl;
 * cout << rs.m_strResult << endl;
 * \endcode
 */
class CHttper 
	: public NonCopyable
{
public:
	/**
	 * \brief 封装了CURL全局初始化函数。
	 */
	static void globalInit();
	/**
	 * \brief 封装了CURL全局清理函数。
	 */
	static void globalClean();
public:
	/**
	 * \brief 执行HTTP GET请求
	 * \param url 执行GET请求的URL
	 * \return 返回操作结果，判断CHttpRs::m_bSucess的值以确定是否成功。返回的结果放在CHttpRs::m_strResult中。
	 */
	static CHttpRs get(const std::string& url);
	/**
	 * \brief 执行HTTP POST请求
	 * \param url 执行POST请求的URL
	 * \param postFields 需要通过POST请求提交的数据
	 * \return 返回操作结果，判断CHttpRs::m_bSucess的值以确定是否成功。返回的结果放在CHttpRs::m_strResult中。
	 */
	static CHttpRs post(const std::string& url, const std::string& postFields);
	/**
	 * \brief 执行HTTP下载
	 * \param url 执行HTTP下载URL
	 * \param headers 执行下载时附加的HTTP头，一般可以为空。视WEB服务器的设定
	 * \param outfilename 指定需要下载的文件在本地的存放路径。
	 * \return 返回操作结果，判断CHttpRs::m_bSucess的值以确定是否成功。返回的结果放在CHttpRs::m_strResult中。
	 */
	static CHttpRs download(const std::string& url, const std::vector<std::string>& headers, const std::string& outfilename);
	/**
	 * \brief 执行查询远程服务器上的文件大小的操作
	 * \param url 远程服务器上文件的URL
	 * \param [out] length 返回文件大小
	 * \return 返回操作结果，判断CHttpRs::m_bSucess的值以确定是否成功。返回的结果放在CHttpRs::m_strResult中。
	 */
	static CHttpRs getDownloadFileLenth(const char *url,OUT double* length);
	/**
	 * \brief 执行查询远程服务器上的文件大小的操作
	 * \param url 远程服务器上文件的URL
	 * \param [out] length 返回文件大小
	 * \return 返回操作结果，判断CHttpRs::m_bSucess的值以确定是否成功。返回的结果放在CHttpRs::m_strResult中。
	 */
	static bool writeFile(const std::string& outfilename, const std::string& strContent, bool bClear);
private:
	/**
	 * \brief 创建一个CMyCurl，为它设置标准的选项并返回它。
	 * \param url 请求的URL
	 * \param headers GET/POST请求的HTTP头
	 * \param out 为CURL设置的标准字符串流，设置这个参数后curl_easy_perform时CURL将HTTP请求的返回结果写入到这个流中。
	 * \return 返回创建的CMyCurl对象。
	 * \remark
	 *   out 参数指定的流在执行以下语句时被设定为回调（在这里是write_data）的最后一个参数(在这里是void *stream)：
	 *   \code
	 *   size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
	 *   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	 *   \endcode
	 *   
	 */
	static CMyCurl normalCurl(const std::string& url, const std::vector<std::string>& headers, std::stringstream* out);
	/**
	 * \brief 执行一个CURL请求curl_easy_perform。返回执行结果
	 * \param mycurl normalCurl方法返回的对象
	 * \param rs @see normalCurl 的out参数
	 * \return 返回CURL的执行结果
	 */
	static CHttpRs request(CMyCurl *mycurl, std::stringstream* rs);

private:
	
	static const int g_timeout = 15;///< CURL超时设定为15秒
};

/**
 * \}
 */
