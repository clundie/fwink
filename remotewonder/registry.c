#include <windows.h>
#include <stdlib.h>
static char* szRegKey = "Software\\Chris Lundie\\Fwink\\ATIRemoteWonder";

/*
	Load a string from the registry.
	This function allocates the memory for the buffer.
	The caller must free the memory.
*/
BOOL
Reg_LoadString(LPCTSTR szName, LPTSTR* buffer)
{
  HKEY hkey;
	LONG l;
	int buffersize = 0;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, szRegKey, 0, KEY_EXECUTE, &hkey)
			!= ERROR_SUCCESS)
		return FALSE;

	/*
		Get size of data.
	*/
	l = RegQueryValueEx(hkey, szName, NULL, NULL, NULL, &buffersize);
	if (ERROR_SUCCESS != l)
	{
		RegCloseKey(hkey);
		return FALSE;
	}

	/*
		Allocate memory for data.
	*/
	*buffer = malloc(buffersize);
	if (!(*buffer))
	{
		RegCloseKey(hkey);
		return FALSE;
	}

	/*
		Read data.
	*/
	l = RegQueryValueEx(hkey, szName, NULL, NULL, (BYTE*)(*buffer), &buffersize);
	RegCloseKey(hkey);
	if (ERROR_SUCCESS != l)
	{
		free(*buffer);
		return FALSE;
	}

	return TRUE;
}

/*
	Save a string to the registry
*/
BOOL
Reg_SaveString(LPCTSTR szName, LPCTSTR value)
{
	HKEY hkey;
	LONG l;

	if (RegCreateKeyEx(HKEY_CURRENT_USER,
		szRegKey, 0, NULL, 0,
		KEY_EXECUTE | KEY_SET_VALUE, NULL,
		&hkey, NULL) != ERROR_SUCCESS)
		return FALSE;

	l = RegSetValueEx(hkey, szName, 0,
		REG_SZ, (CONST BYTE*)value,
		(lstrlen(value) + 1) * sizeof(TCHAR));

	RegCloseKey(hkey);

	if (ERROR_SUCCESS != l) return FALSE;
	return TRUE;
}

/*
	Save a DWORD to the registry.
*/
BOOL
Reg_SaveDWORD(LPCTSTR szName, const DWORD value)
{
	HKEY hkey;
	LONG l;

	if (RegCreateKeyEx(HKEY_CURRENT_USER,
		szRegKey, 0, NULL, 0,
		KEY_EXECUTE | KEY_SET_VALUE, NULL,
		&hkey, NULL) != ERROR_SUCCESS)
		return FALSE;

	l = RegSetValueEx(hkey, szName, 0,
		REG_DWORD, (CONST BYTE *)&value,
		sizeof(DWORD));

	RegCloseKey(hkey);

	if (ERROR_SUCCESS != l) return FALSE;
	return TRUE;
}

/*
	Load a DWORD from the registry
*/
BOOL
Reg_LoadDWORD(LPCTSTR szName, DWORD* buffer)
{
  HKEY hkey;
	LONG l;
	int buffersize = sizeof(DWORD);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, szRegKey, 0, KEY_EXECUTE, &hkey)
			!= ERROR_SUCCESS)
		return FALSE;

	l = RegQueryValueEx(hkey, szName, NULL, NULL, (BYTE*)buffer, &buffersize);
	RegCloseKey(hkey);
	if (ERROR_SUCCESS != l)
		return FALSE;

	return TRUE;
}
