/**
 * \file Httper.h 
 * \brief ������CURL HTTP������װ��CHttpRs��CMyCurl��CScopedMyCurl��CHttper
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
 * \defgroup CURL_HTTP_OPERATION HTTP����
 * \{
 */

/**
 * \class CHttpRs
 * \brief http�����Ľ��
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
	 * \brief ����һ���������ʧ�ܵ�CHttpRsָ�롣
	 * \return �������ʧ�ܵ�CHttpRsָ��
	 * \remark ��Ҫȥdelete���������ص���һ��ȫ�ֵĶ���
	 */
	static CHttpRs* CurlFailed();

public:
	bool m_bSucess;				///< �����Ƿ�ɹ��ı�־
	std::string m_strResult;	///< http��������ֵ��ע����룡

private:
	static CHttpRs* g_curlFailed; ///< ��̬���󣬴���CURL����ʧ�ܡ�
};

/**
 * \class CMyCurl
 * \brief ��װCURL����ͨ��CMyCurlִ��CURL������
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
 * \brief CMyCurl����Դ��װ��
 * 
 * ������װһ��CMyCurl�Ķ���ָ�룬����������ʱִ��curl_easy_cleanup��curl_slist_free_all
 * ��Դ��װ��һ��������һ�����������Զ��ͷ���Դ������
 * \code
 * {
 *     CMyCurl ObjMyCurl = normalCurl(...); //����һ��ObjMyCurl
 *     CScopedMyCurl AutoCleanResourceHelper(&ObjMyCurl); 
 *     //...do something with ObjMyCurl.
 * }
 * //�뿪AutoCleanResourceHelper��������������ʱ��C++��������֤AutoCleanResourceHelper���������������á�
 * //��~CScopedMyCurl()�� curl_easy_cleanup��curl_slist_free_all�����á��ͷŵ�curl��Դ��
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
 * \brief ��װִ��CURL http����Ĳ�����
 * 
 * ʹ�÷�����
 * �ڽ��̿�ʼ�ĵط�ִ��һ��CHttper::globalInit�����ڽ����˳�ǰִ��һ��globalClean
 * \code
 * int APIENTRY _tWinMain(HINSTANCE hInstance,
 * HINSTANCE hPrevInstance,
 * LPTSTR    lpCmdLine,
 * int       nCmdShow)
 * {
 *     //����������ʼ��
 *     //...
 *     CHttper::globalInit();
 *     //...
 *     //����Ϣѭ��:
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
 * ��ִ��GET����ʱ��
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
	 * \brief ��װ��CURLȫ�ֳ�ʼ��������
	 */
	static void globalInit();
	/**
	 * \brief ��װ��CURLȫ����������
	 */
	static void globalClean();
public:
	/**
	 * \brief ִ��HTTP GET����
	 * \param url ִ��GET�����URL
	 * \return ���ز���������ж�CHttpRs::m_bSucess��ֵ��ȷ���Ƿ�ɹ������صĽ������CHttpRs::m_strResult�С�
	 */
	static CHttpRs get(const std::string& url);
	/**
	 * \brief ִ��HTTP POST����
	 * \param url ִ��POST�����URL
	 * \param postFields ��Ҫͨ��POST�����ύ������
	 * \return ���ز���������ж�CHttpRs::m_bSucess��ֵ��ȷ���Ƿ�ɹ������صĽ������CHttpRs::m_strResult�С�
	 */
	static CHttpRs post(const std::string& url, const std::string& postFields);
	/**
	 * \brief ִ��HTTP����
	 * \param url ִ��HTTP����URL
	 * \param headers ִ������ʱ���ӵ�HTTPͷ��һ�����Ϊ�ա���WEB���������趨
	 * \param outfilename ָ����Ҫ���ص��ļ��ڱ��صĴ��·����
	 * \return ���ز���������ж�CHttpRs::m_bSucess��ֵ��ȷ���Ƿ�ɹ������صĽ������CHttpRs::m_strResult�С�
	 */
	static CHttpRs download(const std::string& url, const std::vector<std::string>& headers, const std::string& outfilename);
	/**
	 * \brief ִ�в�ѯԶ�̷������ϵ��ļ���С�Ĳ���
	 * \param url Զ�̷��������ļ���URL
	 * \param [out] length �����ļ���С
	 * \return ���ز���������ж�CHttpRs::m_bSucess��ֵ��ȷ���Ƿ�ɹ������صĽ������CHttpRs::m_strResult�С�
	 */
	static CHttpRs getDownloadFileLenth(const char *url,OUT double* length);
	/**
	 * \brief ִ�в�ѯԶ�̷������ϵ��ļ���С�Ĳ���
	 * \param url Զ�̷��������ļ���URL
	 * \param [out] length �����ļ���С
	 * \return ���ز���������ж�CHttpRs::m_bSucess��ֵ��ȷ���Ƿ�ɹ������صĽ������CHttpRs::m_strResult�С�
	 */
	static bool writeFile(const std::string& outfilename, const std::string& strContent, bool bClear);
private:
	/**
	 * \brief ����һ��CMyCurl��Ϊ�����ñ�׼��ѡ���������
	 * \param url �����URL
	 * \param headers GET/POST�����HTTPͷ
	 * \param out ΪCURL���õı�׼�ַ��������������������curl_easy_performʱCURL��HTTP����ķ��ؽ��д�뵽������С�
	 * \return ���ش�����CMyCurl����
	 * \remark
	 *   out ����ָ��������ִ���������ʱ���趨Ϊ�ص�����������write_data�������һ������(��������void *stream)��
	 *   \code
	 *   size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
	 *   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	 *   \endcode
	 *   
	 */
	static CMyCurl normalCurl(const std::string& url, const std::vector<std::string>& headers, std::stringstream* out);
	/**
	 * \brief ִ��һ��CURL����curl_easy_perform������ִ�н��
	 * \param mycurl normalCurl�������صĶ���
	 * \param rs @see normalCurl ��out����
	 * \return ����CURL��ִ�н��
	 */
	static CHttpRs request(CMyCurl *mycurl, std::stringstream* rs);

private:
	
	static const int g_timeout = 15;///< CURL��ʱ�趨Ϊ15��
};

/**
 * \}
 */
