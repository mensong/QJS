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
	/* 保存当前操作根键的句柄						*/
	HKEY m_hKey;															

public:
	/* 是否有效										*/
	BOOL IsValid() const;													
	
	/* 构造函数，默认参数为：HKEY_LOCAL_MACHINE		*/
	CRegOperator(HKEY hKey = NULL);											
	
	/* 设置当前操作的根键							*/
	CRegOperator(const char* strKey, bool bX64 = false);

	/* 读写的形式打开注册表							*/
	CRegOperator OpenKey(const char* lpSubKey, bool bX64 = false) const;	
	
	/* 关闭键句柄									*/
	void Close();															
	
	/* 创建并打开注册表		    					*/
	CRegOperator CreateKey(const char* lpSubKey, bool bX64 = false) const;	
	
	/* 删除相应的子键（子健下必须为空）				*/
	BOOL DeleteKey(const char* lpSubKey, bool bX64 = false) const;			
	
	/* 删除相应的子键								*/
	BOOL DeleteTree(const char* lpSubKey) const;							
	
	/* 删除子键处的相应的键值						*/
	BOOL DeleteValue(const char* lpValueName) const;						
	
	/* 从指定注册表文件中恢复						*/
	BOOL SaveKey(const char* lpFileName) const;						

	/* 把当前键值保存到指定文件						*/
	BOOL RestoreKey(const char* lpFileName) const;							
	
	/* 读出REG_SZ类型的值							*/
	BOOL ReadValue(const char* lpValueName, std::string& outVal) const;		
	
	/* 读出DWORD类型的值							*/
	BOOL ReadValue(const char* lpValueName, DWORD& dwVal) const;			
	
	/* 写入REG_SZ类型值								*/
	BOOL WriteValue(const char* lpValueName, const char* lpVal) const;			
	
	/* 写入DWORD类型值								*/
	BOOL WriteValue(const char* lpValueName, DWORD dwVal) const;				
	
	/* 写入指定类型值								*/
	BOOL WriteValue(const char* lpValueName, DWORD dwType, const BYTE* lpData, DWORD dwDataSize) const;	
	
	/* 写入REG_EXPAND_SZ类型值						*/
	BOOL WriteExpandString(const char* lpValueName, const char* lpVal) const;	
	
	/* 枚举											*/
	BOOL EnumSub(
		OUT std::vector<std::string>* subTreeNames = NULL,
		OUT std::vector<std::string>* subValueNames = NULL) const;			
		
	/* 拷贝							                 */
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