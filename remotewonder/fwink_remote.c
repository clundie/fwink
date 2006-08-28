/*

This file was derived from "Winamp.c"
in the ATI REMOTE WONDER Plug-In SDK.

Last modified by Chris Lundie 2003-08-09.
http://lundie.ca/

*/

#include "ammo.h"
#include "resource.h"
#include <stdlib.h>
#include <commdlg.h>
#include <mmsystem.h>

UINT uRemoteCaptureMessage;
BOOL bGotRemoteCaptureMessage = FALSE;

#define kNumFunctions (1)
#define kTakePicture (0)
static char* functions[kNumFunctions] = 
{ 
	"Take a Picture"
};

/*
	Function prototypes
*/
INT_PTR CALLBACK DialogProc_Configure(HWND, UINT, WPARAM,	LPARAM);
BOOL Reg_LoadString(LPCTSTR szName, LPTSTR* buffer);
BOOL Reg_SaveString(LPCTSTR szName, LPCTSTR value);
BOOL Reg_LoadDWORD(LPCTSTR szName, DWORD* buffer);
BOOL Reg_SaveDWORD(LPCTSTR szName, const DWORD value);
void SetSoundOption(HWND hwndDlg, DWORD dwSound, BOOL bCheckButton);
void TakePicture(HWND hwndFwink);

#define FWINK_SOUND_NONE (0)
#define FWINK_SOUND_DEFAULT (1)
#define FWINK_SOUND_CUSTOM (2)

DWORD
WhatKeysDoYouWant(void)
{
	return (RECORD | CUSTOM_MAPPED);
}

char*
EnumerateProgrammableFunction(WORD wIndex)
{
	if (wIndex >= kNumFunctions)
		return NULL;

	return functions[wIndex];
}

void
Configure(HANDLE hWnd)
{
	DialogBox(GetModuleHandle("fwink_remote"), MAKEINTRESOURCE(IDD_CONFIGURE),
		hWnd, DialogProc_Configure);
}

int
AreYouInFocus(void)
{
	HWND hwndFwink = FindWindow("#32770", "Fwink");
	HWND hForeground = GetForegroundWindow();

	if (hwndFwink == NULL)
		return FALSE;

	if (hwndFwink == hForeground)
		return TRUE;
	
	if (hwndFwink == GetWindow(hForeground, GW_OWNER))
		return TRUE;

	return FALSE;
}

BOOL
HandleKey(BOOL bCustom, WORD wKeyEvent, WORD wState)
{
	HWND hwndFwink = FindWindow("#32770", "Fwink");

	if (hwndFwink == NULL)
		return FALSE;
	
	if (wState != RMCTRL_KEY_ON)
		return FALSE;

	if (bCustom)
	{
		switch (wKeyEvent)
		{
			case kTakePicture:
			{
				TakePicture(hwndFwink);
				return TRUE;
			}
		}
	}
	else
	{
		switch (wKeyEvent)
		{
			case RMCTRL_RECORD:
			{
				TakePicture(hwndFwink);
				return TRUE;
			}
		}
	}

	return FALSE;
}

INT_PTR CALLBACK
DialogProc_Configure(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			LPTSTR buffer;
			DWORD dwSound = FWINK_SOUND_DEFAULT;

			if (Reg_LoadString("soundfile", &buffer))
			{
				SetDlgItemText(hwndDlg, IDC_EDIT_SOUNDFILE, buffer);
				free(buffer);
			}

			Reg_LoadDWORD("sound", &dwSound);
			SetSoundOption(hwndDlg, dwSound, TRUE);

			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_SOUND_NONE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						SetSoundOption(hwndDlg, FWINK_SOUND_NONE, FALSE);
					break;
				}

				case IDC_SOUND_DEFAULT:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						SetSoundOption(hwndDlg, FWINK_SOUND_DEFAULT, FALSE);
					break;

				}
				case IDC_SOUND_CUSTOM:
				{
					if (HIWORD(wParam) == BN_CLICKED)
						SetSoundOption(hwndDlg, FWINK_SOUND_CUSTOM, FALSE);
					break;
				}

				case IDC_BTN_PLAYSOUND:
				{
					int buffersize = 512;
					LPTSTR buffer = malloc(buffersize * sizeof(TCHAR));
					if (0 != GetDlgItemText(hwndDlg, IDC_EDIT_SOUNDFILE, buffer, buffersize))
					{
						Reg_SaveString("soundfile", buffer);
						PlaySound(NULL, GetModuleHandle("fwink_remote"), SND_PURGE);
						PlaySound(buffer, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
					}
					free(buffer);
					return TRUE;
				}

				case IDCANCEL:
				{
					PlaySound(NULL, GetModuleHandle("fwink_remote"), SND_PURGE);
					EndDialog(hwndDlg, 0);
					return TRUE;
				}

				case IDOK:
				{
					DWORD dwSound = FWINK_SOUND_DEFAULT;
					int buffersize = 512;
					LPTSTR buffer = malloc(buffersize * sizeof(TCHAR));

					if (0 != GetDlgItemText(hwndDlg, IDC_EDIT_SOUNDFILE, buffer, buffersize))
						Reg_SaveString("soundfile", buffer);
					else
					{
						buffer[0] = 0;
						Reg_SaveString("soundfile", buffer);
					}
					free(buffer);

					if (BST_CHECKED == IsDlgButtonChecked(hwndDlg,  IDC_SOUND_NONE))
						dwSound = FWINK_SOUND_NONE;
					else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg,  IDC_SOUND_CUSTOM))
						dwSound = FWINK_SOUND_CUSTOM;

					Reg_SaveDWORD("sound", dwSound);

					PlaySound(NULL, GetModuleHandle("fwink_remote"), SND_PURGE);
					EndDialog(hwndDlg, 0);
					return TRUE;
				}

				case IDC_BTN_BROWSE_SOUND:
				{
					OPENFILENAME ofn;
					BOOL bGotFilename;
					int buffersize = 512;
					LPTSTR szFilename = malloc(buffersize * sizeof(TCHAR));

					GetDlgItemText(hwndDlg, IDC_EDIT_SOUNDFILE, szFilename, buffersize);

					#if (_WIN32_WINNT >= 0x0500)
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					#else
					ofn.lStructSize = sizeof(OPENFILENAME);
					#endif
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFilter = TEXT("Sound Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0\0");
					ofn.lpstrCustomFilter = NULL;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = szFilename;
					ofn.nMaxFile = buffersize;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = NULL;
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					ofn.lpstrDefExt = TEXT("jpg");

					bGotFilename = GetOpenFileName(&ofn);
					if (bGotFilename)
						SetDlgItemText(hwndDlg, IDC_EDIT_SOUNDFILE, szFilename);

					free(szFilename);

					return TRUE;
				}

			}
			break;
		}
	}

	return FALSE;
}

void
SetSoundOption(HWND hwndDlg, DWORD dwSound, BOOL bCheckButton)
{
	switch (dwSound)
	{
		case FWINK_SOUND_NONE:
			if (bCheckButton)
				CheckDlgButton(hwndDlg, IDC_SOUND_NONE, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_SOUNDFILE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PLAYSOUND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_BROWSE_SOUND), FALSE);
			break;

		case FWINK_SOUND_CUSTOM:
			if (bCheckButton)
				CheckDlgButton(hwndDlg, IDC_SOUND_CUSTOM, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_SOUNDFILE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PLAYSOUND), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_BROWSE_SOUND), TRUE);
			break;

		case FWINK_SOUND_DEFAULT:
		default:
			if (bCheckButton)
				CheckDlgButton(hwndDlg, IDC_SOUND_DEFAULT, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_SOUNDFILE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PLAYSOUND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_BROWSE_SOUND), FALSE);
	}
}

void
TakePicture(HWND hwndFwink)
{
	DWORD dwSound = FWINK_SOUND_DEFAULT;
	LPTSTR buffer;

	if (!bGotRemoteCaptureMessage)
	{
		uRemoteCaptureMessage = RegisterWindowMessage(TEXT("FwinkCapture"));
		bGotRemoteCaptureMessage = TRUE;
	}

	PostMessage(hwndFwink, uRemoteCaptureMessage, 0, 0);

	Reg_LoadDWORD("sound", &dwSound);
	if (FWINK_SOUND_CUSTOM == dwSound)
	{
		if (Reg_LoadString("soundfile", &buffer))
		{
			PlaySound(NULL, GetModuleHandle("fwink_remote"), SND_PURGE);
			PlaySound(buffer, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
			free(buffer);
		}
	}
	else if (FWINK_SOUND_DEFAULT == dwSound)
	{
		MessageBeep(MB_OK);
	}
}
