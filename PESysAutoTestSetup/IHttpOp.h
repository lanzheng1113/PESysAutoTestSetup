/**
 * \brief ������WEB�����Ľӿڣ������ļ���GET����
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
 * \brief IHttpOp �ӿ��ඨ�����������޸�������WEB�����Ľӿڡ�
 *
 * IHttpOp �����������ӿڣ�
 * <ol>
 * <li>GetRemoteFileSize ��ȡԶ�̷�������ָ���ļ��Ĵ�С</li>
 * <li>DownloadFile �����ļ�</li>
 * <li>DoGet ��WEB������GET����</li>
 * </ol>
 * <b>ע1���������޸�����������û��ʹ�õ�POST��������˲�������صĽӿ�</b>
 */
class IHttpOp : Interface
{
public:
	/**
	 * \brief ��ȡԶ�̷�������ָ���ļ��Ĵ�С
	 * \param uri ָ����Զ�̷������ļ�·��
	 * \return Զ�̷������ļ��Ĵ�С��ʧ�ܷ���-1�����򷵻�Զ�̷������ļ��Ĵ�С
	 */
	virtual double GetRemoteFileSize(const string& uri) = 0;
	/**
	 * \brief ����һ���ļ���ָ����Ŀ¼
	 * \param uri ���ص�ַ
	 * \param TargetFolder ����Ŀ¼
	 * \param [out] TargetName �����ļ���
	 * \return �ɹ�����true,���򷵻�false.
	 */
	virtual bool DownloadFile(const string& uri,const string& TargetFolder,string& TargetName) = 0;
	/**
	 * \brief ����GET����
	 * \param GET�����URL
	 * \return �ɹ�����HTTP���������ص����ݣ����򷵻�һ�����ַ�����
	 */
	virtual string DoGet(const string& url) = 0;

	/**
	 * \brief POST���ݵ�ָ����URL
	 * 
	 * \param url �����ύ��URL
	 * \param data �ύ������
	 * \return �ɹ��󷵻�HTTP���������ص����ݣ����򷵻�һ�����ַ�����
	 */
	virtual string DoPost(const string& url, const string& data) = 0;
protected:
private:
};

/**
 * \}
 */
