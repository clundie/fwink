/*

Copyright (c) 2001-2004 Chris Lundie
http://lundie.ca/

This file is part of Fwink.

Fwink is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Fwink is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fwink; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "WebCam.h"

const wstring WebCam::sRegKey(L"Software\\Chris Lundie\\Fwink");

// Load a string from the registry
//
bool
WebCam::LoadSetting(const WCHAR* name, wstring& value)
{
  HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey.c_str(), 0, KEY_EXECUTE, &hkey)
			!= ERROR_SUCCESS)
		return false;

	// Determine length of string
	//
	TCHAR szValue[8193];
  DWORD keysize = sizeof(szValue);

  LONG l = RegQueryValueEx(hkey, name, NULL, NULL,
														reinterpret_cast<BYTE*>(szValue),
														&keysize);
	RegCloseKey(hkey);

	if (ERROR_SUCCESS != l)
		return false;

	value.assign(szValue, keysize/sizeof(TCHAR));
	return true;
}

// Load a bool from the registry
//
bool
WebCam::LoadSetting(const WCHAR* name, bool* pValue)
{
	unsigned iValue = 0;
	bool bReturn = LoadSetting(name, &iValue);
	if (bReturn)
		*pValue = iValue ? true : false;

	return bReturn;
}

// Load an int from the registry
//
bool
WebCam::LoadSetting(const WCHAR* name, int* pValue)
{
	return LoadSetting(name, (unsigned*)pValue);
}

// Load an unsigned int from the registry
//
bool
WebCam::LoadSetting(const WCHAR* name, unsigned* pValue)
{
  HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, sRegKey.c_str(), 0, KEY_EXECUTE, &hkey)
			!= ERROR_SUCCESS)
		return false;

  DWORD keysize = sizeof(*pValue);
  LONG l = RegQueryValueEx(hkey, name, NULL, NULL,
														reinterpret_cast<BYTE*>(pValue),
														&keysize);
	RegCloseKey(hkey);

	if (ERROR_SUCCESS != l) return false;
	return true;
}

// Save a string to the registry
//
bool
WebCam::SaveSetting(const WCHAR* name, const TCHAR* value)
{
	HKEY hkey;

  if (RegCreateKeyEx(HKEY_CURRENT_USER,
                     sRegKey.c_str(), 0, NULL, 0,
                     KEY_EXECUTE | KEY_SET_VALUE, NULL,
                     &hkey, NULL) != ERROR_SUCCESS)
    return false;

  LONG l = RegSetValueEx(hkey, name, 0,
													REG_SZ, reinterpret_cast<CONST BYTE *>(value),
													(lstrlen(value) + 1) * sizeof(TCHAR));

  RegCloseKey(hkey);

	if (ERROR_SUCCESS != l) return false;
	return true;
}

// Save a bool to the registry
//
bool
WebCam::SaveSetting(const WCHAR* name, const bool value)
{
	unsigned iValue = value ? 1 : 0;
	return SaveSetting(name, iValue);
}

// Save an int to the registry
//
bool
WebCam::SaveSetting(const WCHAR* name, const int value)
{
	return SaveSetting(name, *((const unsigned*)&value));
}

// Save an unsigned int to the registry
//
bool
WebCam::SaveSetting(const WCHAR* name, const unsigned value)
{
	HKEY hkey;

  if (RegCreateKeyEx(HKEY_CURRENT_USER,
                     sRegKey.c_str(), 0, NULL, 0,
                     KEY_EXECUTE | KEY_SET_VALUE, NULL,
                     &hkey, NULL) != ERROR_SUCCESS)
    return false;

	LONG l = RegSetValueEx(hkey, name, 0,
													REG_DWORD, reinterpret_cast<CONST BYTE *>(&value),
													sizeof(value));
  RegCloseKey(hkey);

	if (ERROR_SUCCESS != l) return false;
	return true;
}
