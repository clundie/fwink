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
#include <shellapi.h> // for shellexecute

const UINT WebCam::uNotifyIconMessage = WM_APP + 1;
const UINT WebCam::uCaptureMessage = WM_APP + 2;
const UINT WebCam::uRemoteCaptureMessage = RegisterWindowMessage(TEXT("FwinkCapture"));
const UINT WebCam::uQuitMessage = WM_APP + 3;

void
WebCam::HandleCaptureMessage(WebCam* cam, HWND hwndDlg)
{
	long lEventCode, lParam1, lParam2;

	IMediaEventEx* pEvent = NULL;
	HRESULT hr = cam->pGraph->QueryInterface(IID_IMediaEventEx, (void**)&pEvent);
	if (FAILED(hr))
		return;

	int i=0;
	while (pEvent && SUCCEEDED(pEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0)))
	{
		if (i++ > 10) break;

		switch(lEventCode)
		{
			case EC_COMPLETE:
			{
				if (!cam || !cam->pGraph) break;

				// Get the captured frame

				cam->UploadImage();

				// Turn off further notifications
				//
				IMediaEventEx* pMediaEvent = NULL;
				cam->pGraph->QueryInterface(IID_IMediaEventEx, (void**)(&pMediaEvent));
				if (pMediaEvent)
				{
					pMediaEvent->SetNotifyFlags(AM_MEDIAEVENT_NONOTIFY);
					pMediaEvent->Release();
					pMediaEvent = NULL;
				}

				// Turn off sample grabber
				//
				ISampleGrabber *pGrabber = NULL;
				cam->pSampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)(&pGrabber));
				if (pGrabber)
				{
					pGrabber->SetBufferSamples(FALSE);
					pGrabber->SetOneShot(FALSE);
					pGrabber->Release();
					pGrabber = NULL;
				}

				// Re-start the graph if window is visible,
				// otherwise stop it
				//
				if (IsWindowVisible(hwndDlg))
				{
					cam->StopCapture();
					cam->StartCapture();
				}
				else
				{
					cam->StopCapture();
				}

				return;
			}

			default:
			{
				return;
			}

		}
	}

	if (pEvent)
		pEvent->Release();
	pEvent = NULL;
}

// Main dialog callback
//
INT_PTR CALLBACK
WebCam::DialogProcMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WebCam* cam = reinterpret_cast<WebCam*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	if (uMsg == uRemoteCaptureMessage)
	{
		cam->CaptureFrame();
		return TRUE;
	}

  switch(uMsg)
	{
		case uCaptureMessage:
		{
			cam->HandleCaptureMessage(cam, hwndDlg);
			break;
		}

		case WM_SHOWWINDOW:
		{
			if (!cam || !cam->pGraph) break;

			IMediaControl* pMediaControl = NULL;
			HRESULT hr;

			hr = cam->pGraph->QueryInterface(IID_IMediaControl, (void**)(&pMediaControl));
			if (SUCCEEDED(hr))
			{
				if (wParam)
					cam->StartCapture();
				else
					cam->StopCapture();
			}

			break;
		}

		case uQuitMessage:
		{
			PostQuitMessage(0);
			break;
		}

		case WM_TIMER:
		{
			if (wParam == cam->uCancelFtpTimerID)
			{
				::KillTimer(cam->hwndMain, cam->uCancelFtpTimerID);
				if (cam->hFTP)
				{
					::InternetCloseHandle(cam->hFTP);
					cam->hFTP = NULL;
					cam->SetStatus(STATUS_FTP_TIMEOUT);
				}
				cam->bOKToDeleteTempFile = true;
				if (cam->bOneShot)
				{
					SendMessage(cam->hwndMain, cam->uQuitMessage, 0, 0);
				}
				else
				{
					if (!cam->bDisableTimer)
					{
						cam->CaptureFrame();
					}
				}
			}
			else if ((wParam == cam->uTimerID) && !(cam->bDisableTimer))
			{
				if (!cam->hFTP && (cam->iStatus != STATUS_CONNECTING))
				{
					cam->CaptureFrame();
				}
			}
			break;
		}

		case WM_QUERYENDSESSION:
		{
			cam->SaveSettings();
			PostQuitMessage(0);
			SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
			break;
		}

		// Command message from dialog box
		//
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_BTNHOMEPAGE:
				{
					ShellExecute(hwndDlg, NULL, szHomePageURL, NULL, NULL, SW_SHOWNORMAL);
					break;
				}

				case IDC_BTNSETTINGS:
				{
					cam->ShowSettingsDialog();
					break;
				}

				case IDCANCEL:
				{
					ShowWindow(hwndDlg, SW_HIDE);
					break;
				}

				case IDC_BTNCAPTURE:
				{
					cam->CaptureFrame();
					break;
				}

				case IDC_BTNEXIT:
				{
					PostQuitMessage(0);
					break;
				}

			}
			break;
		}

		// Message from tray notification icon
		//
		case uNotifyIconMessage:
		{
			if (wParam != uNotifyIconID) break;

			if
				(
					(lParam == WM_LBUTTONDOWN) || (lParam == NIN_BALLOONUSERCLICK)
					|| (lParam == WM_LBUTTONDBLCLK)
				)
			{
				cam->ShowMainDialog();
			}
			break;
		}

		// Initialize dialog
		//
		case WM_INITDIALOG:
		{
			// Initialize common controls (up-down and trackbar controls)
			//
			INITCOMMONCONTROLSEX init;
			init.dwSize = sizeof(INITCOMMONCONTROLSEX);
			init.dwICC = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;
			if (FALSE == InitCommonControlsEx(&init))
				PostQuitMessage(0);

			// Get pointer to WebCam object that created this dialog
			//
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			// Set dialog icon
			//
			SetClassLongPtr(hwndDlg, GCLP_HICONSM,
				reinterpret_cast<LONG_PTR>(LoadImage(GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON,
				GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
				LR_DEFAULTCOLOR)));
			SetClassLongPtr(hwndDlg, GCLP_HICON,
				reinterpret_cast<LONG_PTR>(LoadImage(GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON,
				GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
				LR_DEFAULTCOLOR)));

			break;
		}

		// Close dialog
		//
		case WM_CLOSE:
		{
			ShowWindow(hwndDlg, SW_HIDE);
			break;
		}

	default:
		{
			if (uMsg == cam->uTaskbarRestart)
			{
				// Taskbar restarted, so
				// create the tray notification icon again
				//
				try
				{
					cam->CreateNotificationIcon();
				}
				catch (int)
				{
					PostQuitMessage(0);
				}
				break;
			}

			return FALSE;
		}
	}

	return TRUE;
}
