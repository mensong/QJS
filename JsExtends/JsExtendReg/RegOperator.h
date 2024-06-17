#ifndef REGOP_H
#define REGOP_H

#include <windows.h>
#include <assert.h>
#include <vector>
#include <string>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

class CRegOperator
{
private:
	/* ���浱ǰ���������ľ��						*/
	HKEY m_hKey;															

public:
	/* �Ƿ���Ч										*/
	BOOL IsValid() const;													
	
	/* ���캯����Ĭ�ϲ���Ϊ��HKEY_LOCAL_MACHINE		*/
	CRegOperator(HKEY hKey = NULL);											
	
	/* ���õ�ǰ�����ĸ���							*/
	CRegOperator(const char* strKey, bool bX64 = false);

	/* ��д����ʽ��ע���							*/
	CRegOperator OpenKey(const char* lpSubKey, bool bX64 = false) const;	
	
	/* �رռ����									*/
	void Close();															
	
	/* ��������ע���		    					*/
	CRegOperator CreateKey(const char* lpSubKey, bool bX64 = false) const;	
	
	/* ɾ����Ӧ���Ӽ����ӽ��±���Ϊ�գ�				*/
	BOOL DeleteKey(const char* lpSubKey, bool bX64 = false) const;			
	
	/* ɾ����Ӧ���Ӽ�								*/
	BOOL DeleteTree(const char* lpSubKey) const;							
	
	/* ɾ���Ӽ�������Ӧ�ļ�ֵ						*/
	BOOL DeleteValue(const char* lpValueName) const;						
	
	/* ��ָ��ע����ļ��лָ�						*/
	BOOL SaveKey(const char* lpFileName) const;						

	/* �ѵ�ǰ��ֵ���浽ָ���ļ�						*/
	BOOL RestoreKey(const char* lpFileName) const;							
	
	/* ����REG_SZ���͵�ֵ							*/
	BOOL ReadValue(const char* lpValueName, std::string& outVal) const;		
	
	/* ����DWORD���͵�ֵ							*/
	BOOL ReadValue(const char* lpValueName, DWORD& dwVal) const;			
	
	/* д��REG_SZ����ֵ								*/
	BOOL WriteValue(const char* lpValueName, const char* lpVal) const;			
	
	/* д��DWORD����ֵ								*/
	BOOL WriteValue(const char* lpValueName, DWORD dwVal) const;				
	
	/* д��ָ������ֵ								*/
	BOOL WriteValue(const char* lpValueName, DWORD dwType, const BYTE* lpData, DWORD dwDataSize) const;	
	
	/* д��REG_EXPAND_SZ����ֵ						*/
	BOOL WriteExpandString(const char* lpValueName, const char* lpVal) const;	
	
	/* ö��											*/
	BOOL EnumSub(
		OUT std::vector<std::string>* subTreeNames = NULL,
		OUT std::vector<std::string>* subValueNames = NULL) const;			
		
	/* ����							                 */
	BOOL CopyFrom(const CRegOperator& from, bool bX64 = false) const;

	~CRegOperator();

	CRegOperator& operator=(const CRegOperator& o);
	CRegOperator(const CRegOperator& o);

protected:
	void _deRef();

private:
	int* m_pRefCount;
};

#endif