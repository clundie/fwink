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

void
WebCam::InitFTPControls(HWND hwndDlg, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_FTPSERVER), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_USERNAME), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTORY), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DIALUP), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PASSIVEFTP), bEnable);
}

void
WebCam::InitTimerControls(HWND hwndDlg, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEVALUE), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SPINTIME), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEUNIT), bEnable);
}

void
WebCam::InitFileControls(HWND hwndDlg, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOCALFILENAME), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_FILEBROWSE), bEnable);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OPENFILE), bEnable);
}

// FTP dialog callback
//
INT_PTR CALLBACK
WebCam::DialogProcFTP(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WebCam* cam = reinterpret_cast<WebCam*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

  switch(uMsg)
	{
		case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
			if (pNmhdr->code == PSN_APPLY)
			{
				TCHAR buffer[512];

				GetDlgItemText(hwndDlg, IDC_FTPSERVER, buffer, 512);
				cam->sServer = buffer;

				GetDlgItemText(hwndDlg, IDC_USERNAME, buffer, 512);
				cam->sUsername = buffer;

				GetDlgItemText(hwndDlg, IDC_PASSWORD, buffer, 512);
				cam->sPassword = buffer;

				GetDlgItemText(hwndDlg, IDC_DIRECTORY, buffer, 512);
				cam->sDirectory = buffer;

				GetDlgItemText(hwndDlg, IDC_FILENAME, buffer, 512);
				cam->sFilename = buffer;

				GetDlgItemText(hwndDlg, IDC_LOCALFILENAME, buffer, 512);
				cam->sLocalFilename = buffer;

				cam->bDialup = (IsDlgButtonChecked(hwndDlg, IDC_DIALUP) == BST_CHECKED);
				cam->bPassiveFTP = (IsDlgButtonChecked(hwndDlg, IDC_PASSIVEFTP) == BST_CHECKED);
				cam->bRunAtStartup = (IsDlgButtonChecked(hwndDlg, IDC_RUNATSTARTUP) == BST_CHECKED);
				cam->bDisableFTP = (IsDlgButtonChecked(hwndDlg, IDC_DISABLEFTP) == BST_CHECKED);
				cam->bDisableTimer = (IsDlgButtonChecked(hwndDlg, IDC_DISABLETIMER) == BST_CHECKED);
				cam->bSaveToFile = (IsDlgButtonChecked(hwndDlg, IDC_SAVETOFILE) == BST_CHECKED);

				BOOL bGotInt;
				cam->uTimeValue = GetDlgItemInt(hwndDlg, IDC_TIMEVALUE, &bGotInt, FALSE);
				cam->uTimeUnit = SendDlgItemMessage(hwndDlg, IDC_TIMEUNIT, CB_GETCURSEL, 0, 0);
				cam->uTimeValue = max(min(cam->uTimeValue, uMaxTimeValue), uMinTimeValue);
				cam->uTimeUnit = max(min(cam->uTimeUnit, uMaxTimeUnit), uMinTimeUnit);
				SetDlgItemInt(hwndDlg, IDC_TIMEVALUE, cam->uTimeValue, FALSE);
				SendDlgItemMessage(hwndDlg, IDC_TIMEUNIT, CB_SETCURSEL, cam->uTimeUnit, 0);

				cam->SaveFTPSettings();
				cam->StartTimer();

				PropSheet_CancelToClose(GetParent(hwndDlg));
			}
			break;
		}

		// Command message from dialog box
		//
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_OPENFILE:
				{
					TCHAR szFilename[512];
					if (GetDlgItemText(hwndDlg, IDC_LOCALFILENAME, szFilename, 512) && lstrlen(szFilename))
					{
						ShellExecute(hwndDlg, NULL, szFilename, NULL, NULL, SW_SHOWNORMAL);
					}
					break;
				}

				case IDC_FILEBROWSE:
				{
					TCHAR szFilename[512];
					GetDlgItemText(hwndDlg, IDC_LOCALFILENAME, szFilename, 512);

					OPENFILENAME ofn;
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFilter = TEXT("JPEG Images\0*.jpg;*.jpe;*.jpeg;*.jif\0All Files\0*.*\0\0");
					ofn.lpstrCustomFilter = NULL;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = szFilename;
					ofn.nMaxFile = 512;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = NULL;
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
					ofn.lpstrDefExt = TEXT("jpg");
					ofn.pvReserved = NULL;
					ofn.dwReserved = 0;

					BOOL bGotFilename = GetSaveFileName(&ofn);

					if (bGotFilename)
					{
						SetDlgItemText(hwndDlg, IDC_LOCALFILENAME, szFilename);
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}

					break;
				}

				case IDC_FTPSERVER:
				case IDC_USERNAME:
				case IDC_PASSWORD:
				case IDC_DIRECTORY:
				case IDC_FILENAME:
				case IDC_TIMEVALUE:
				case IDC_LOCALFILENAME:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_TIMEUNIT:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_RUNATSTARTUP:
				case IDC_DIALUP:
				case IDC_PASSIVEFTP:
				case IDC_DISABLEFTP:
				case IDC_DISABLETIMER:
				case IDC_SAVETOFILE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					if (LOWORD(wParam) == IDC_DISABLEFTP)
					{
						cam->InitFTPControls(hwndDlg, !IsDlgButtonChecked(hwndDlg, IDC_DISABLEFTP));
					}
					else if (LOWORD(wParam) == IDC_DISABLETIMER)
					{
						cam->InitTimerControls(hwndDlg, !IsDlgButtonChecked(hwndDlg, IDC_DISABLETIMER));
					}
					else if (LOWORD(wParam) == IDC_SAVETOFILE)
					{
						cam->InitFileControls(hwndDlg, IsDlgButtonChecked(hwndDlg, IDC_SAVETOFILE));
					}
					break;
				}

			}

			break;
		}

		// Initialize dialog
		//
		case WM_INITDIALOG:
		{
			PROPSHEETPAGE* psp = reinterpret_cast<PROPSHEETPAGE*>(lParam);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, psp->lParam);
			WebCam* cam = reinterpret_cast<WebCam*>(psp->lParam);

			// Add options to time unit combo box (minutes, hours, seconds)
			//
			TCHAR buffer[256];
			LoadString(GetModuleHandle(NULL), IDS_SECONDS, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_TIMEUNIT, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));
			LoadString(GetModuleHandle(NULL), IDS_MINUTES, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_TIMEUNIT, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));
			LoadString(GetModuleHandle(NULL), IDS_HOURS, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_TIMEUNIT, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));

			// Set range for up-down control
			//
			SendDlgItemMessage(hwndDlg, IDC_SPINTIME, UDM_SETRANGE, 0,
				(LPARAM)MAKELONG((short)cam->uMaxTimeValue, (short)cam->uMinTimeValue));

			// Load settings into controls
			SetDlgItemInt(hwndDlg, IDC_TIMEVALUE, cam->uTimeValue, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_TIMEUNIT, CB_SETCURSEL, cam->uTimeUnit, 0);
			CheckDlgButton(hwndDlg, IDC_DIALUP, cam->bDialup ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PASSIVEFTP, cam->bPassiveFTP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RUNATSTARTUP, cam->bRunAtStartup ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_FTPSERVER, cam->sServer.c_str());
			SetDlgItemText(hwndDlg, IDC_USERNAME, cam->sUsername.c_str());
			SetDlgItemText(hwndDlg, IDC_PASSWORD, cam->sPassword.c_str());
			SetDlgItemText(hwndDlg, IDC_DIRECTORY, cam->sDirectory.c_str());
			SetDlgItemText(hwndDlg, IDC_FILENAME, cam->sFilename.c_str());
			CheckDlgButton(hwndDlg, IDC_DISABLEFTP, cam->bDisableFTP ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DISABLETIMER, cam->bDisableTimer ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_FTPSERVER, cam->sServer.c_str());
			CheckDlgButton(hwndDlg, IDC_SAVETOFILE, cam->bSaveToFile ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_LOCALFILENAME, cam->sLocalFilename.c_str());

			cam->InitFTPControls(hwndDlg, !cam->bDisableFTP);
			cam->InitTimerControls(hwndDlg, !cam->bDisableTimer);
			cam->InitFileControls(hwndDlg, cam->bSaveToFile);

			break;
		}

    default:
		{
      return FALSE;
		}
  }

  return TRUE;
}
