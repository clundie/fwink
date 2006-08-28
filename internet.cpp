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

void CALLBACK
WebCam::InternetStatusCallback(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
)
{
	switch(dwInternetStatus)
	{
		case INTERNET_STATUS_REQUEST_COMPLETE:
		{
			INTERNET_ASYNC_RESULT* pResult =
				static_cast<INTERNET_ASYNC_RESULT*>(lpvStatusInformation);
			WebCam* cam = reinterpret_cast<WebCam*>(dwContext);

			switch(cam->iStatus)
			{
				case STATUS_CONNECTING:
				{
					HINTERNET hFTP = reinterpret_cast<HINTERNET>(pResult->dwResult);
					if (hFTP)
					{
						// FTP connection succeeded
						//
						cam->hFTP = hFTP;
						cam->SetStatus(STATUS_CHDIR);

						// Change directory
						//
						if ((cam->sDirectory.length() < 1) || (cam->sDirectory[0] == 0))
							FtpSetCurrentDirectory(hFTP, TEXT("."));
						else
							FtpSetCurrentDirectory(hFTP, cam->sDirectory.c_str());

					}
					else
					{
						// FTP connection failed
						//
						cam->SetStatus(ERR_CONNECT);
						if (cam->bOneShot) SendMessage(cam->hwndMain, cam->uQuitMessage, 0, 0);
					}
					break;
				}

				case STATUS_CHDIR:
				{
					BOOL bSuccess = static_cast<BOOL>(pResult->dwResult);

					if (bSuccess)
					{
						// Directory change succeeded.
						// Upload file.
						//
						cam->bOKToDeleteTempFile = false;
						cam->SetStatus(STATUS_UPLOADING);
						FtpPutFile(hInternet, cam->szwFilenameJPEG, cam->sFilename.c_str(),
							FTP_TRANSFER_TYPE_BINARY,
							reinterpret_cast<DWORD_PTR>(cam));
					}
					else
					{
						// Directory change failed.
						//
						cam->SetStatus(ERR_UPLOAD_DIRECTORY);
						InternetSetStatusCallback(hInternet, NULL);
						InternetCloseHandle(hInternet);
						cam->hFTP = NULL;
						if (cam->bOneShot) SendMessage(cam->hwndMain, cam->uQuitMessage, 0, 0);
					}

					break;
				}

				case STATUS_UPLOADING:
				{
					BOOL bSuccess = static_cast<BOOL>(pResult->dwResult);

					if (bSuccess)
					{
						// File upload succeeded.
						//
						cam->SetStatus(SUCCESS_UPLOADED_IMAGE);
						InternetSetStatusCallback(hInternet, NULL);
						InternetCloseHandle(hInternet);
						cam->hFTP = NULL;
					}
					else
					{
						// File upload failed.
						//
						cam->SetStatus(ERR_UPLOAD);
						InternetSetStatusCallback(hInternet, NULL);
						InternetCloseHandle(hInternet);
						cam->hFTP = NULL;
					}

					cam->bOKToDeleteTempFile = true;
					if (cam->bOneShot) SendMessage(cam->hwndMain, cam->uQuitMessage, 0, 0);

					break;
				}

			}

			break;
		}

	}
}

bool
WebCam::ConnectedToInternet(void)
{
	bool bReadyToCheck = true;

	if (bDialup)
	{
		bReadyToCheck = false;

		// Enumerate dial-up connections
		RASCONN* pConnData = new RASCONN[1];
		pConnData->dwSize = sizeof(RASCONN);
		DWORD dwBufferSize = sizeof(RASCONN);
		DWORD dwCount = 0;

#ifdef _DEBUG
		OutputDebugString(TEXT("Enumerating dial-up connections...\n"));
#endif

		DWORD dw = RasEnumConnections(pConnData, &dwBufferSize, &dwCount);

		if (dw == ERROR_BUFFER_TOO_SMALL)
		{
			delete[] pConnData;
			pConnData = reinterpret_cast<RASCONN*>(new char[dwBufferSize]);
			DWORD dw = RasEnumConnections(pConnData, &dwBufferSize, &dwCount);
		}

		if (dw == 0)
		{
			// Check if any connections are connected

#ifdef _DEBUG
		if (dwCount == 0)
			OutputDebugString(TEXT("There are no dial-up connections.\n"));
#endif

			for (DWORD i=0; i<dwCount; ++i)
			{
				RASCONNSTATUS status;
				status.dwSize = sizeof(RASCONNSTATUS);
				if ((RasGetConnectStatus(pConnData[i].hrasconn, &status) == 0)
							&& (status.rasconnstate == RASCS_Connected))
				{
					bReadyToCheck = true;

#ifdef _DEBUG
					OutputDebugString(TEXT("Found a connected dial-up connection\n"));
#endif

				}
			}

#ifdef _DEBUG
			if (!bReadyToCheck)
			{
				OutputDebugString(TEXT("Did not find a connected dial-up connection\n"));
			}
#endif

		}

#ifdef _DEBUG
		else
		{
			OutputDebugString(TEXT("Failed to enumerate dial-up connections\n"));
		}
#endif

		delete[] pConnData;
	}

	return bReadyToCheck;
}
