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

#include "dllversion.h"
#include <windows.h>
#include <shlwapi.h>

DWORD GetDllVersion(LPCTSTR lpszDllName)
{
	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	hinstDll = LoadLibrary(lpszDllName);
	
	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}
				
		FreeLibrary(hinstDll);
	}
	return dwVersion;
}
