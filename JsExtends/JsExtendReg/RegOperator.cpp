/********************************************************************
*　 文件名：RegOperator.cpp
*　 文件描述：对头文件中定义类的成员函数进行了实现
*　 修改记录：
*********************************************************************/
#include "RegOperator.h"

/*
提升权限
privilegeName可取有：
	SE_CREATE_TOKEN_NAME
	SE_ASSIGNPRIMARYTOKEN_NAME
	SE_LOCK_MEMORY_NAME
	SE_INCREASE_QUOTA_NAME
	SE_UNSOLICITED_INPUT_NAME
	SE_MACHINE_ACCOUNT_NAME
	SE_TCB_NAME
	SE_SECURITY_NAME
	SE_TAKE_OWNERSHIP_NAME
	SE_LOAD_DRIVER_NAME
	SE_SYSTEM_PROFILE_NAME
	SE_SYSTEMTIME_NAME
	SE_PROF_SINGLE_PROCESS_NAME
	SE_INC_BASE_PRIORITY_NAME
	SE_CREATE_PAGEFILE_NAME
	SE_CREATE_PERMANENT_NAME
	SE_BACKUP_NAME
	SE_RESTORE_NAME
	SE_SHUTDOWN_NAME
	SE_DEBUG_NAME
	SE_AUDIT_NAME
	SE_SYSTEM_ENVIRONMENT_NAME
	SE_CHANGE_NOTIFY_NAME
	SE_REMOTE_SHUTDOWN_NAME
	SE_UNDOCK_NAME
	SE_SYNC_AGENT_NAME
	SE_ENABLE_DELEGATION_NAME
	SE_MANAGE_VOLUME_NAME
	SE_IMPERSONATE_NAME
	SE_CREATE_GLOBAL_NAME
	SE_TRUSTED_CREDMAN_ACCESS_NAME
	SE_RELABEL_NAME
	SE_INC_WORKING_SET_NAME
	SE_TIME_ZONE_NAME
	SE_CREATE_SYMBOLIC_LINK_NAME
*/
static BOOL EnablePrivilege(LPCTSTR privilegeName)
{
	HANDLE TokenHandle = NULL;
	TOKEN_PRIVILEGES TokenPrivilege;

	LUID uID;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle)) {
		if (LookupPrivilegeValue(NULL, privilegeName, &uID)) {
			TokenPrivilege.PrivilegeCount = 1;
			TokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			TokenPrivilege.Privileges[0].Luid = uID;
			if (AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPrivilege, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
				CloseHandle(TokenHandle);
				TokenHandle = INVALID_HANDLE_VALUE;
				return TRUE;
			}
			else
				goto Fail;

		}
		else
			goto Fail;
	}
	else
		goto Fail;

Fail:
	CloseHandle(TokenHandle);
	TokenHandle = INVALID_HANDLE_VALUE;
	return FALSE;
}

/* helper macro to fixup start/end slice values */
#define ADJUST_INDICES(start, end, len)         \
    if (end > len)                          \
        end = len;                          \
    else if (end < 0) {                     \
        end += len;                         \
        if (end < 0)                        \
        end = 0;                        \
    }                                       \
    if (start < 0) {                        \
        start += len;                       \
        if (start < 0)                      \
        start = 0;                      \
    }

#define MAX_32BIT_INT 2147483647

static int strFind(const std::string & str, const std::string & sub, int start = 0, int end = MAX_32BIT_INT)
{
	ADJUST_INDICES(start, end, (int)str.size());

	std::string::size_type result = str.find(sub, start);

	// If we cannot find the string, or if the end-point of our found substring is past
	// the allowed end limit, return that it can't be found.
	if (result == std::string::npos ||
		(result + sub.size() > (std::string::size_type)end))
	{
		return -1;
	}

	return (int)result;
}

static std::string strReplace(const std::string & str, const std::string & oldstr, const std::string & newstr, int count = -1)
{
	int sofar = 0;
	int cursor = 0;
	std::string s(str);

	std::string::size_type oldlen = oldstr.size(), newlen = newstr.size();

	cursor = strFind(s, oldstr, cursor);

	while (cursor != -1 && cursor <= (int)s.size())
	{
		if (count > -1 && sofar >= count)
		{
			break;
		}

		s.replace(cursor, oldlen, newstr);
		cursor += (int)newlen;

		if (oldlen != 0)
		{
			cursor = strFind(s, oldstr, cursor);
		}
		else
		{
			++cursor;
		}

		++sofar;
	}

	return s;

}

/*============================================================
* 函 数 名：IsValid
* 参　　数：无
* 功能描述：构造函数，初始化根键
* 返 回 值：有效返回TRUE,无效返回FALSE
*============================================================*/
BOOL CRegOperator::IsValid() const
{
	return m_hKey != NULL;
}

/*============================================================
* 函 数 名：CRegOperator
* 参　　数：HKEY [IN] : 默认是HKEY_LOCAL_MACHINE
* 功能描述：构造函数，初始化根键
* 返 回 值：无
*============================================================*/
CRegOperator::CRegOperator(HKEY hKey)
	: m_pRefCount(NULL)
{
	m_hKey = hKey;

	if (m_hKey)
	{
		m_pRefCount = new int;
		*m_pRefCount = 1;
	}
}


void CRegOperator::_deRef()
{
	if (m_pRefCount)
	{
		--(*m_pRefCount);

		if ((*m_pRefCount) <= 0)
		{
			*m_pRefCount = 0;
			delete m_pRefCount;
			m_pRefCount = NULL;
			Close();
		}
	}
}

/*============================================================
* 函 数 名：~CRegOperator
* 参　　数：NULL [IN]
* 功能描述：析构函数，关闭打开的注册表句柄
* 返 回 值：无
* 抛出异常：
*============================================================*/
CRegOperator::~CRegOperator()
{
	_deRef();
}

/*============================================================
* 函 数 名：operator=
* 参　　数：另外一个对象 [IN]
* 功能描述：拷贝函数
* 返 回 值：无
* 抛出异常：
*============================================================*/
CRegOperator& CRegOperator::operator=(const CRegOperator& o)
{
	_deRef();

	m_pRefCount = o.m_pRefCount;
	if (m_pRefCount)
		++(*m_pRefCount);

	m_hKey = o.m_hKey;
	return *this;
}
/*============================================================
* 函 数 名：CRegOperator
* 参　　数：NULL [IN]
* 功能描述：拷贝构造函数
* 返 回 值：无
* 抛出异常：
*============================================================*/
CRegOperator::CRegOperator(const CRegOperator& o)
{
	*this = o;
}

/*============================================================
* 函 数 名：Close
* 参　　数：NULL [IN]
* 功能描述：关闭打开键的句柄
* 返 回 值：void
* 抛出异常：
*============================================================*/
void CRegOperator::Close()
{
	if (m_hKey)
	{
		/* 句柄非空进行关闭 */
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
}

/*============================================================
* 函 数 名：SetHKEY
* 参　　数：LPCTSTR [IN] : 根键值
* 功能描述：由传入的根键字符串设置当前操作的根键的值
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/

CRegOperator::CRegOperator(const char* strKey, bool bX64/* = false*/)
	: m_pRefCount(NULL)
{
	assert(strKey);
	
	std::string sPath = strKey;
	sPath = strReplace(sPath, "/", "\\");
	
	do
	{
		size_t oldLen = sPath.size();
		sPath = strReplace(sPath, "\\\\", "\\");
		if (oldLen == sPath.size())
			break;
	} while (true);

	int idxRoot = strFind(sPath, "\\");
	if (idxRoot <= 0)
	{
		/* 逐个进行比较 */
		if (0 == _stricmp(strKey, ("HKEY_CLASSES_ROOT")))
		{
			m_hKey = HKEY_CLASSES_ROOT;
		}
		else if (0 == _stricmp(strKey, ("HKEY_CURRENT_USER")))
		{
			m_hKey = HKEY_CURRENT_USER;
		}
		else if (0 == _stricmp(strKey, ("HKEY_LOCAL_MACHINE")))
		{
			m_hKey = HKEY_LOCAL_MACHINE;
		}
		else if (0 == _stricmp(strKey, ("HKEY_USERS")))
		{
			m_hKey = HKEY_USERS;
		}
		else if (0 == _stricmp(strKey, ("HKEY_PERFORMANCE_DATA")))
		{
			m_hKey = HKEY_PERFORMANCE_DATA;
		}
		else if (0 == _stricmp(strKey, ("HKEY_PERFORMANCE_TEXT")))
		{
			m_hKey = HKEY_PERFORMANCE_TEXT;
		}
		else if (0 == _stricmp(strKey, ("HKEY_PERFORMANCE_NLSTEXT")))
		{
			m_hKey = HKEY_PERFORMANCE_NLSTEXT;
		}
		/* 对操作系统版本进行测试 */
#if(WINVER >= 0x0400)
		else if (0 == _stricmp(strKey, ("HKEY_CURRENT_CONFIG")))
		{
			m_hKey = HKEY_CURRENT_CONFIG;
		}
		else if (0 == _stricmp(strKey, ("HKEY_DYN_DATA")))
		{
			m_hKey = HKEY_DYN_DATA;
		}
#endif
	}	
	else
	{//打开全路径		
		std::string root = sPath.substr(0, idxRoot);
		new(this) CRegOperator(root.c_str());
		if (!m_hKey)
			return;
		std::string sub = sPath.substr(idxRoot + 1);
		*this = OpenKey(sub.c_str(), bX64);
	}
}

/*============================================================
* 函 数 名：OpenKey
* 参　　数：LPCTSTR [IN] : 子键字符串
* 功能描述：通过传入子键的字符串打开注册表相应的位置
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
CRegOperator CRegOperator::OpenKey(const char* lpSubKey, bool bX64/*=false*/) const
{
	assert(m_hKey);
	assert(lpSubKey);

	if (!m_hKey || !lpSubKey)
		return CRegOperator();

	DWORD opt = KEY_ALL_ACCESS;
	if (bX64)
		opt |= KEY_WOW64_64KEY;

	HKEY hKey;
	long lReturn = RegOpenKeyExA(m_hKey, lpSubKey, 0L, opt, &hKey);

	if (ERROR_SUCCESS == lReturn)
	{
		return CRegOperator(hKey);
	}
	/* 打开失败 */
	return CRegOperator();
}

/*============================================================
* 函 数 名：CreateKey
* 参　　数：LPCTSTR [IN] : 子键字符串
* 功能描述：通过传入子键的字符串打开（存在子键）或者创建（不存在子键）相应的子键
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
CRegOperator CRegOperator::CreateKey(const char* lpSubKey, bool bX64/*=false*/) const
{
	assert(m_hKey);
	assert(lpSubKey);

	if (!m_hKey || !lpSubKey)
		return CRegOperator();

	DWORD opt = KEY_ALL_ACCESS;
	if (bX64)
		opt |= KEY_WOW64_64KEY;
	
	HKEY hKey;
	DWORD dw;
	long lReturn = RegCreateKeyExA(m_hKey, lpSubKey, 0L, NULL, REG_OPTION_NON_VOLATILE, opt, NULL, &hKey, &dw);

	if (ERROR_SUCCESS == lReturn)
	{
		return CRegOperator(hKey);
	}
	/* 打开或者创建失败 */
	return CRegOperator();
}

/*============================================================
* 函 数 名：DeleteKey
* 参　　数：LPCTSTR LPCTSTR [IN] : 根键值 子键值
* 功能描述：通过传入的根键和子键，将子键删除删除，不能包含子键
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::DeleteKey(const char* lpSubKey, bool bX64/*=false*/) const
{
	assert(lpSubKey);
	assert(m_hKey);

	if (!m_hKey || !lpSubKey)
		return FALSE;

	DWORD opt = KEY_ALL_ACCESS;
	if (bX64)
		opt |= KEY_WOW64_64KEY;

	long lReturn = RegDeleteKeyExA(m_hKey, lpSubKey, opt, 0);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 删除成功 */
		return TRUE;
	}
	/* 删除失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：DeleteTree
* 参　　数：LPCTSTR [IN] : 要删除键值的名称
* 功能描述：通过传入键值名称，删除对应的键值
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::DeleteTree(const char* lpSubKey) const
{
	assert(lpSubKey);
	assert(m_hKey);

	if (!m_hKey || !lpSubKey)
		return FALSE;
	
	long lReturn = RegDeleteTreeA(m_hKey, lpSubKey);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 删除成功 */
		return TRUE;
	}
	/* 删除失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：DeleteValue
* 参　　数：LPCTSTR [IN] : 要删除键值的名称
* 功能描述：通过传入键值名称，删除对应的键值
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::DeleteValue(const char* lpValueName) const
{
	assert(m_hKey);
	assert(lpValueName);

	if (!m_hKey || !lpValueName)
		return FALSE;

	long lReturn = RegDeleteValueA(m_hKey, lpValueName);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 删除成功 */
		return TRUE;
	}
	/* 删除失败 */
	return FALSE;
}


/*============================================================
* 函 数 名：SaveKey
* 参　　数：LPCTSTR [IN] : 待保存的文件名
* 功能描述：通过保存的文件名称，保存对应的子键
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::SaveKey(const char* lpFileName) const
{
	assert(m_hKey);
	assert(lpFileName);

	if (!m_hKey || !lpFileName)
		return FALSE;

	EnablePrivilege(SE_BACKUP_NAME);
	
	long lReturn = RegSaveKeyA(m_hKey, lpFileName, NULL);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 保存成功 */
		return TRUE;
	}
	/* 保存失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：RestoreKey
* 参　　数：LPCTSTR [IN] : 待恢复的文件名
* 功能描述：通过文件名称，从其中导入注册表中
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::RestoreKey(const char* lpFileName) const
{
	assert(m_hKey);
	assert(lpFileName);

	if (!m_hKey || !lpFileName)
		return FALSE;

	EnablePrivilege(SE_RESTORE_NAME);

	long lReturn = RegRestoreKeyA(m_hKey, lpFileName, 0);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 导入成功 */
		return TRUE;
	}
	/* 导入失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：Read
* 参　　数：LPCTSTR CString [IN] : 键值 读取值的key名，为空字符串时读取默认值
* 功能描述：将指定位置的CString类型的值读出
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::ReadValue(const char* lpValueName, std::string& outVal) const
{
	assert(m_hKey);
	assert(lpValueName);

	if (!m_hKey || !lpValueName)
		return FALSE;

	DWORD dwType = 0;
	DWORD dwSize = 2047;
	char szString[2048];
	memset(szString, 0, 2048);

	long lReturn = RegQueryValueExA(m_hKey, lpValueName, NULL, &dwType, (BYTE *)szString, &dwSize);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 查询成功 */
		outVal = szString;

		return TRUE;
	}
	/* 查询失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：Read
* 参　　数：LPCTSTR DWORD [IN] : 键值 读取值的key名，为空字符串时读取默认值
* 功能描述：将指定位置DWORD类型的值读出
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::ReadValue(const char* lpValueName, DWORD& dwVal) const
{
	assert(m_hKey);
	assert(lpValueName);

	if (!m_hKey || !lpValueName)
		return FALSE;

	DWORD dwType;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwDest;
	long lReturn = RegQueryValueExA(m_hKey, lpValueName, NULL, &dwType, (BYTE *)&dwDest, &dwSize);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 查询成功 */
		dwVal = dwDest;

		return TRUE;
	}
	DWORD err = GetLastError();
	/* 查询失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：Write
* 参　　数：LPCTSTR LPCTSTR [IN] : 键值 读取值的key名，为空字符串时读取默认值
* 功能描述：将LPCTSTR类型的值写入指定位置
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::WriteValue(const char* lpValueName, const char* lpValue) const
{
	assert(m_hKey);
	assert(lpValueName);
	assert(lpValue);

	if (!m_hKey || !lpValueName || !lpValue)
		return FALSE;

	long lReturn = RegSetValueExA(m_hKey, lpValueName, 0L, REG_SZ, (const BYTE *)lpValue, (DWORD)(strlen(lpValue) * sizeof(lpValue[0])) + 1);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 成功写入 */
		return TRUE;
	}
	/* 写入失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：Write
* 参　　数：LPCTSTR DWORD [IN] : 键值 写入值的key名，为空字符串时读取默认值
* 功能描述：将DWORD类型的值写入指定位置
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::WriteValue(const char* lpValueName, DWORD dwVal) const
{
	assert(m_hKey);
	assert(lpValueName);

	if (!m_hKey || !lpValueName)
		return FALSE;

	long lReturn = RegSetValueExA(m_hKey, lpValueName, 0L, REG_DWORD, (const BYTE *)&dwVal, sizeof(DWORD));

	if (ERROR_SUCCESS == lReturn)
	{
		/* 成功写入 */
		return TRUE;
	}
	/* 写入失败 */
	return FALSE;
}


BOOL CRegOperator::WriteValue(const char* lpValueName, DWORD dwType, const BYTE* lpData, DWORD dwDataSize) const
{
	assert(m_hKey);
	assert(lpValueName);

	if (!m_hKey || !lpValueName)
		return FALSE;

	long lReturn = RegSetValueExA(m_hKey, lpValueName, 0L, dwType, lpData, dwDataSize);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 成功写入 */
		return TRUE;
	}
	/* 写入失败 */
	return FALSE;
}

BOOL CRegOperator::WriteExpandString(const char* lpValueName, const char* lpVal) const
{
	assert(m_hKey);
	assert(lpValueName);

	if (!m_hKey || !lpValueName)
		return FALSE;

	long lReturn = RegSetValueExA(m_hKey, lpValueName, 0L, REG_EXPAND_SZ, (const BYTE *)lpVal, (DWORD)(strlen(lpVal) * sizeof(lpVal[0])) + 1);

	if (ERROR_SUCCESS == lReturn)
	{
		/* 成功写入 */
		return TRUE;
	}
	/* 写入失败 */
	return FALSE;
}

/*============================================================
* 函 数 名：Write
* 参　　数：subTreeNames [OUT] 子树列表
* 参　　数：subValueNames [OUT] 键列表
* 功能描述：将整型数写入指定位置
* 返 回 值：BOOL
* 抛出异常：
*============================================================*/
BOOL CRegOperator::EnumSub(OUT std::vector<std::string>* subTreeNames/*=NULL*/, OUT std::vector<std::string>* subValueNames/*=NULL*/) const
{
	if (!m_hKey)
		return FALSE;

	char     achKey[MAX_KEY_LENGTH];   // buffer for subkey name  
	char	 achValue[MAX_VALUE_NAME];
	DWORD	 cchValue = MAX_VALUE_NAME;
	DWORD    cbName;                   // size of name string   
	char     achClass[MAX_PATH] = ("");  // buffer for class name   
	DWORD    cchClassName = MAX_PATH;  // size of class string   
	DWORD    cSubKeys = 0;               // number of subkeys
	DWORD    cValues;              // number of values for key   
	DWORD    cbMaxSubKey;              // longest subkey size   
	DWORD    cchMaxClass;              // longest class string
	DWORD    cchMaxValue;          // longest value name   
	DWORD    cbMaxValueData;       // longest value data   
	DWORD    cbSecurityDescriptor; // size of security descriptor   
	FILETIME ftLastWriteTime;      // last write time   

	DWORD i, retCode;
	
	// Get the class name and the value count.   
	retCode = RegQueryInfoKeyA(
		m_hKey,                    // key handle   
		achClass,                // buffer for class name   
		&cchClassName,           // size of class string   
		NULL,                    // reserved   
		&cSubKeys,               // number of subkeys   
		&cbMaxSubKey,            // longest subkey size   
		&cchMaxClass,            // longest class string   
		&cValues,                // number of values for this key   
		&cchMaxValue,            // longest value name   
		&cbMaxValueData,         // longest value data   
		&cbSecurityDescriptor,   // security descriptor   
		&ftLastWriteTime);       // last write time   

	if (retCode != ERROR_SUCCESS)
		return FALSE;

	// Enumerate the subkeys, until RegEnumKeyEx fails.  
	if (cSubKeys && subTreeNames)
	{
		//printf( "\nNumber of subkeys: %d\n", cSubKeys);  

		for (i = 0; i < cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			achKey[0] = '\0';
			retCode = RegEnumKeyExA(m_hKey, i,
				achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS)
			{
				//_tprintf(TEXT("(%d) %s\n"), i+1, achKey);  
				subTreeNames->push_back(achKey);
			}
		}
	}

	// Enumerate the key values.   
	if (cValues && subValueNames)
	{
		printf("\nNumber of values: %d\n", cValues);

		for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
		{
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValueA(m_hKey, i,
				achValue,
				&cchValue,
				NULL,
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS)
			{
				//_tprintf(TEXT("(%d) %s\n"), i+1, achValue);   
				subValueNames->push_back(achValue);
			}
		}
	}

	return TRUE;
}

BOOL CRegOperator::CopyFrom(const CRegOperator& from, bool bX64/* = false*/) const
{
	if (!m_hKey || !from.IsValid())
		return FALSE;

	return (ERROR_SUCCESS == RegCopyTree(from.m_hKey, NULL, m_hKey));
}
