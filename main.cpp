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
#include <ctime>

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPSTR lpCmdLine, int nShowCmd)
{
	WebCam* cam = NULL;

	try
	{
/*
		if (time(NULL) > WebCam::_expireTime)
		{
			throw ERR_EXPIRED;
		}
*/
		cam = new WebCam();
		if (cam)
		{
			if (lpCmdLine && !lstrcmpiA(lpCmdLine, "oneshot"))
			{
				cam->StartOneShot();
			}
			else
			{
				cam->Start();
			}
		}
	}
	catch (int uID)
	{
		if (cam)
		{
			delete cam;
			cam = NULL;
		}

		TCHAR buffer[255];
		if (LoadString(GetModuleHandle(NULL), uID, buffer, 255))
		{
			MessageBox(NULL, buffer, NULL, MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);
		}
		else if (LoadString(GetModuleHandle(NULL), ERR_UNKNOWN, buffer, 255))
		{
			MessageBox(NULL, buffer, NULL, MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);
		}
		else
		{
			MessageBox(NULL, TEXT("An unexpected error occurred."), NULL, MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);
		}
	}
	catch (...)
	{
		TCHAR buffer[255];
		if (LoadString(GetModuleHandle(NULL), ERR_UNKNOWN, buffer, 255))
		{
			MessageBox(NULL, buffer, NULL, MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);
		}
		else
		{
			MessageBox(NULL, TEXT("An unexpected error occurred"), NULL, MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);
		}
	}

	if (cam) delete cam;
	return 0;
}
