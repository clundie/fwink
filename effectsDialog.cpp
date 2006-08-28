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

// Effects dialog callback
//
INT_PTR CALLBACK
WebCam::DialogProcEffects(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WebCam* cam = reinterpret_cast<WebCam*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
	static COLORREF customcolors[16];

  switch(uMsg)
	{
		case WM_CTLCOLORSTATIC:
		{
			if (((HWND)lParam) == GetDlgItem(hwndDlg, IDC_SAMPLETEXT))
			{
				if (cam->hbrSampleBackground)
					DeleteObject(cam->hbrSampleBackground);
				cam->hbrSampleBackground = CreateSolidBrush(cam->rgbBackground);

				HDC hdc = (HDC)wParam;
				SetTextColor(hdc, cam->rgbForeground);
				SetBkMode(hdc, TRANSPARENT);
				return (BOOL)cam->hbrSampleBackground;
			}

			return FALSE;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
			if (pNmhdr->code == PSN_APPLY)
			{
				TCHAR buffer[512];
				GetDlgItemText(hwndDlg, IDC_WISDOM, buffer, 512);
				cam->sWisdom = buffer;

				// save font name

				LRESULT fontindex = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, CB_GETCURSEL, 0, 0);
				if (CB_ERR != fontindex)
				{
					LRESULT fontlen = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, CB_GETLBTEXTLEN, fontindex, 0);
					if (CB_ERR != fontlen)
					{
						TCHAR* szFontname = new TCHAR[fontlen+1];
						if (CB_ERR != SendDlgItemMessage(hwndDlg, IDC_FONTLIST, CB_GETLBTEXT, fontindex, reinterpret_cast<LPARAM>(szFontname)))
						{
							cam->sFontname.assign(szFontname);
						}
						delete[] szFontname;
					}
				}

				// save text position
				LRESULT textpos = SendDlgItemMessage(hwndDlg, IDC_TEXTPOSITION, CB_GETCURSEL, 0, 0);
				if (CB_ERR != textpos)
					cam->uTextPosition = textpos;

				// save text background style
				LRESULT bgstyle = SendDlgItemMessage(hwndDlg, IDC_BGSTYLE, CB_GETCURSEL, 0, 0);
				if (CB_ERR != bgstyle)
					cam->uTextBackground = bgstyle;

				cam->bTimestamp = (IsDlgButtonChecked(hwndDlg, IDC_TIMESTAMP) == BST_CHECKED);
				cam->bDatestamp = (IsDlgButtonChecked(hwndDlg, IDC_DATESTAMP) == BST_CHECKED);
				cam->bFontBold = (IsDlgButtonChecked(hwndDlg, IDC_FONTBOLD) == BST_CHECKED);
				cam->bFontItalic = (IsDlgButtonChecked(hwndDlg, IDC_FONTITALIC) == BST_CHECKED);
				cam->bFontUnderline = (IsDlgButtonChecked(hwndDlg, IDC_FONTUNDERLINE) == BST_CHECKED);
				cam->bFontStrikeout = (IsDlgButtonChecked(hwndDlg, IDC_FONTSTRIKEOUT) == BST_CHECKED);
				cam->bDropShadow = (IsDlgButtonChecked(hwndDlg, IDC_FONTSHADOW) == BST_CHECKED);

				if (IsDlgButtonChecked(hwndDlg, IDC_SMOOTHNONE) == BST_CHECKED)
					cam->uSmoothType = smooth_none;
				else if (IsDlgButtonChecked(hwndDlg, IDC_SMOOTHCLEARTYPE) == BST_CHECKED)
					cam->uSmoothType = smooth_cleartype;
				else
					cam->uSmoothType = smooth_standard;

				BOOL bGotInt;
				cam->uFontSize = GetDlgItemInt(hwndDlg, IDC_FONTSIZE, &bGotInt, FALSE);
				cam->uFontSize = max(min(cam->uFontSize, uMaxFontSize), uMinFontSize);
				SetDlgItemInt(hwndDlg, IDC_FONTSIZE, cam->uFontSize, FALSE);

				cam->SaveEffectsSettings();
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
				// Show foreground colour selection dialog
				//
				case IDC_BTNFGCOLOUR:
				{
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = cam->rgbForeground;
					cc.lpCustColors = customcolors;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

					if(ChooseColor(&cc))
					{
						cam->rgbForeground = cc.rgbResult;
						RedrawWindow(GetDlgItem(hwndDlg, IDC_SAMPLETEXT), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
						cam->SaveEffectsSettings();
						PropSheet_CancelToClose(GetParent(hwndDlg));
					}

					break;
				}

				// Show background colour selection dialog
				//
				case IDC_BTNBGCOLOUR:
				{
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = cam->rgbBackground;
					cc.lpCustColors = customcolors;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

					if(ChooseColor(&cc))
					{
						cam->rgbBackground = cc.rgbResult;
						RedrawWindow(GetDlgItem(hwndDlg, IDC_SAMPLETEXT), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
						cam->SaveEffectsSettings();
						PropSheet_CancelToClose(GetParent(hwndDlg));
					}

					break;
				}

				case IDC_WISDOM:
				case IDC_FONTSIZE:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_FONTLIST:
				case IDC_TEXTPOSITION:
				case IDC_BGSTYLE:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_TIMESTAMP:
				case IDC_DATESTAMP:
				case IDC_FONTBOLD:
				case IDC_FONTITALIC:
				case IDC_FONTUNDERLINE:
				case IDC_FONTSTRIKEOUT:
				case IDC_FONTSHADOW:
				case IDC_SMOOTHNONE:
				case IDC_SMOOTHSTANDARD:
				case IDC_SMOOTHCLEARTYPE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
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

			if (!cam->bClearTypeAvailable)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHCLEARTYPE), FALSE);

			switch (cam->uSmoothType)
			{
				case smooth_none:
					CheckDlgButton(hwndDlg, IDC_SMOOTHNONE, BST_CHECKED);
					break;
				case smooth_cleartype:
					CheckDlgButton(hwndDlg, IDC_SMOOTHCLEARTYPE, BST_CHECKED);
					break;
				case smooth_standard:
				default:
					CheckDlgButton(hwndDlg, IDC_SMOOTHSTANDARD, BST_CHECKED);
			}

			// Set range for font size up-down control
			//
			SendDlgItemMessage(hwndDlg, IDC_SPINFONTSIZE, UDM_SETRANGE, 0,
				(LPARAM)MAKELONG((short)cam->uMaxFontSize, (short)cam->uMinFontSize));

			CheckDlgButton(hwndDlg, IDC_TIMESTAMP, cam->bTimestamp ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DATESTAMP, cam->bDatestamp ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_WISDOM, cam->sWisdom.c_str());
			SetDlgItemInt(hwndDlg, IDC_FONTSIZE, cam->uFontSize, FALSE);
			CheckDlgButton(hwndDlg, IDC_FONTBOLD, cam->bFontBold ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONTITALIC, cam->bFontItalic ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONTUNDERLINE, cam->bFontUnderline ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONTSTRIKEOUT, cam->bFontStrikeout ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONTSHADOW, cam->bDropShadow ? BST_CHECKED : BST_UNCHECKED);

			// Add options to font combo box
			//
			bool bGotDefaultFont = false;
			InstalledFontCollection collection;
			int count = collection.GetFamilyCount();
			FontFamily* pFamily = new FontFamily[count];
			int found = 0;
			collection.GetFamilies(count, pFamily, &found);
			WCHAR wszFamilyName[LF_FACESIZE];
			int i;
			for (i=0; i<count; ++i)
			{
				pFamily[i].GetFamilyName(wszFamilyName);
				LRESULT index = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, CB_ADDSTRING, 0,
					reinterpret_cast<LPARAM>(wszFamilyName));
				if (!bGotDefaultFont && (!lstrcmpi(cam->sFontname.c_str(), wszFamilyName)) && (CB_ERR != index))
				{
					SendDlgItemMessage(hwndDlg, IDC_FONTLIST, CB_SETCURSEL, index, 0);
					bGotDefaultFont = true;
				}
			}
			if (!bGotDefaultFont)
			{
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, CB_SETCURSEL, 0, 0);
			}

			delete[] pFamily;

			// Add options to text position combo box
			//
			TCHAR szPosition[255];
			for (i=0; i<9; ++i)
			{
				LoadString(GetModuleHandle(NULL), IDS_UPPERLEFT + i, szPosition, 255);
				SendDlgItemMessage(hwndDlg, IDC_TEXTPOSITION, CB_ADDSTRING, 0,
					reinterpret_cast<LPARAM>(szPosition));
			}
			SendDlgItemMessage(hwndDlg, IDC_TEXTPOSITION, CB_SETCURSEL, cam->uTextPosition, 0);

			// Add options to text background combo box
			//
			TCHAR szBackground[255];
			for (i=0; i<3; ++i)
			{
				LoadString(GetModuleHandle(NULL), IDS_BG_SOLID + i, szBackground, 255);
				SendDlgItemMessage(hwndDlg, IDC_BGSTYLE, CB_ADDSTRING, 0,
					reinterpret_cast<LPARAM>(szBackground));
			}
			SendDlgItemMessage(hwndDlg, IDC_BGSTYLE, CB_SETCURSEL, cam->uTextBackground, 0);

			break;
		}

    default:
		{
      return FALSE;
		}
  }

  return TRUE;
}
