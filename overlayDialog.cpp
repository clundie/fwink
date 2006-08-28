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
#include "overlayDialog.h"
#include <shlobj.h>

void
WebCam::AddOverlay(
	Image* imBase, Image* imOverlay,
	bool bLowQuality,
	unsigned uOpacity,
	unsigned uScaleType,
	float fScaleX, float fScaleY,
	bool bColourKey,
	unsigned uKeyR, unsigned uKeyG, unsigned uKeyB,
	unsigned pos
	)
{
	bool bGotOverlay = true;
	if (!imOverlay)
	{
		bGotOverlay = false;
		imOverlay = new Image(sOverlayImageFile.c_str(), TRUE);
		if (imOverlay->GetLastStatus() != Ok)
		{
			delete imOverlay;
			return;
		}
	}
	Graphics g(imBase);

	float fWidthScaled = imOverlay->GetWidth();
	float fHeightScaled = imOverlay->GetHeight();

	if (uScaleType == WebCam::ovlscale_percentage)
	{
		fWidthScaled *= (fScaleX*0.01f);
		fHeightScaled *= (fScaleY*0.01f);
	}
	else if (uScaleType == WebCam::ovlscale_bestfit)
	{
		float fAspect = fWidthScaled / fHeightScaled;
		fHeightScaled = imBase->GetHeight();
		fWidthScaled = fHeightScaled * fAspect;
		if (fWidthScaled > imBase->GetWidth())
		{
			fWidthScaled = imBase->GetWidth();
			fHeightScaled = fWidthScaled / fAspect;
		}
	}

	float fX = 0.0f, fY = 0.0f;
	switch(pos)
	{
		case textposition_upperleft:
		case textposition_left:
		case textposition_lowerleft:
			fX = 0.0f;
			break;
		case textposition_upperright:
		case textposition_right:
		case textposition_lowerright:
			fX = imBase->GetWidth() - fWidthScaled;
			break;
		case textposition_top:
		case textposition_bottom:
			fX = (imBase->GetWidth() - fWidthScaled) * 0.5f;
	}

	switch(pos)
	{
		case textposition_upperleft:
		case textposition_top:
		case textposition_upperright:
			fY = 0.0f;
			break;
		case textposition_lowerleft:
		case textposition_bottom:
		case textposition_lowerright:
			fY = imBase->GetHeight() - fHeightScaled;
			break;
		case textposition_left:
		case textposition_right:
			fY = (imBase->GetHeight() - fHeightScaled) * 0.5f;
			break;
		case textposition_centre:
			fX = (imBase->GetWidth() - fWidthScaled) * 0.5f;
			fY = (imBase->GetHeight() - fHeightScaled) * 0.5f;
	}

	g.SetCompositingMode(CompositingModeSourceOver);
	if (bLowQuality)
	{
		g.SetCompositingQuality(CompositingQualityHighSpeed);
		g.SetPixelOffsetMode(PixelOffsetModeHighSpeed);
	}
	else
	{
		g.SetCompositingQuality(CompositingQualityGammaCorrected);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);
	}

	if (bLowQuality)
		g.SetInterpolationMode(InterpolationModeLowQuality);
	else
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	if (uOpacity > 0)
	{
		// Alpha blending
		//
		ImageAttributes imageAtt;
		if (uOpacity < 100)
		{
			ColorMatrix colorMatrix =
				{1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, (float)uOpacity * 0.01f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			imageAtt.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault,
				ColorAdjustTypeBitmap);
		}

		// Colour key
		//
		if (bColourKey)
		{
			imageAtt.SetColorKey(Color(0, uKeyR, uKeyG, uKeyB), Color(255, uKeyR, uKeyG, uKeyB), ColorAdjustTypeBitmap);
		}

		// Draw the overlay
		//
		RectF r(fX, fY, fWidthScaled, fHeightScaled);
		g.DrawImage(
			imOverlay, r,
			0.5f, 0.5f,
			(float)imOverlay->GetWidth() - 1.0f,
			(float)imOverlay->GetHeight() - 1.0f,
			UnitPixel, &imageAtt, NULL, NULL);
	}

	if (!bGotOverlay) delete imOverlay;
}

void
OverlayDialog::InitKeyControls()
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_OVLKEY_CHOOSE), (IsDlgButtonChecked(hwndDlg, IDC_OVLKEY_CHK) == BST_CHECKED));
}

void
OverlayDialog::InitScaleControls()
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_OVLSCALE_SLIDER), (IsDlgButtonChecked(hwndDlg, IDC_OVLSCALE_PERCENT) == BST_CHECKED));
	EnableWindow(GetDlgItem(hwndDlg, IDC_OVLSCALE_EDIT), (IsDlgButtonChecked(hwndDlg, IDC_OVLSCALE_PERCENT) == BST_CHECKED));
}

void
OverlayDialog::SetOverlayFile(LPCTSTR szFile)
{
	if (imageOverlay) delete imageOverlay;
	imageOverlay = new Image(szFile, TRUE);
	UpdateComposite();
}

void
OverlayDialog::UpdateComposite()
{
	if (!imageBase) return;

	if (imageComposite) delete imageComposite;
	imageComposite = new Bitmap(uPreviewWidth, uPreviewHeight, PixelFormat32bppRGB);

	Graphics g(imageComposite);
	g.SetCompositingMode(CompositingModeSourceCopy);
	g.SetCompositingQuality(CompositingQualityHighSpeed);
	g.SetInterpolationMode(InterpolationModeLowQuality);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	RectF r(0, 0, uPreviewWidth, uPreviewHeight);
	g.DrawImage(imageBase, r, 0.5f, 0.5f, (float)imageBase->GetWidth() - 1.0f, (float)imageBase->GetHeight() - 1.0f, UnitPixel, NULL, NULL, NULL);

	unsigned uOpacity = SendDlgItemMessage(hwndDlg, IDC_OVLOPACITY_SLIDER, TBM_GETPOS, 0, 0);
	float fScale = SendDlgItemMessage(hwndDlg, IDC_OVLSCALE_SLIDER, TBM_GETPOS, 0, 0);
	float fScaleX = fScale * (float)uPreviewWidth / (float)cam->uImageWidth;
	float fScaleY = fScale * (float)uPreviewHeight / (float)cam->uImageHeight;

	bool bKey = (IsDlgButtonChecked(hwndDlg, IDC_OVLKEY_CHK) == BST_CHECKED);

	LRESULT ovlpos = SendDlgItemMessage(hwndDlg, IDC_OVLPOS_LIST, CB_GETCURSEL, 0, 0);

	unsigned uScaletype;
	if (IsDlgButtonChecked(hwndDlg, IDC_OVLSCALE_PERCENT) == BST_CHECKED)
		uScaletype = WebCam::ovlscale_percentage;
	else if (IsDlgButtonChecked(hwndDlg, IDC_OVLSCALE_BESTFIT) == BST_CHECKED)
		uScaletype = WebCam::ovlscale_bestfit;
	else
	{
		uScaletype = WebCam::ovlscale_percentage;
		fScaleX = 100.0f * (float)uPreviewWidth / (float)cam->uImageWidth;
		fScaleY = 100.0f * (float)uPreviewHeight / (float)cam->uImageHeight;
	}

	cam->AddOverlay(imageComposite, imageOverlay, true, uOpacity,
		uScaletype, fScaleX, fScaleY,
		bKey, GetRValue(clrKeyLast), GetGValue(clrKeyLast), GetBValue(clrKeyLast),
		ovlpos
		);

	InvalidateRect(GetDlgItem(hwndDlg, IDC_OVERLAY_PREVIEW_FRAME), NULL, FALSE);
}

// Overlay dialog callback
//
INT_PTR CALLBACK
WebCam::DialogProcOverlay(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	OverlayDialog* od = reinterpret_cast<OverlayDialog*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
	static COLORREF customcolors[16];

  switch(uMsg)
	{
		case WM_CTLCOLORSTATIC:
		{
			if (((HWND)lParam) == GetDlgItem(hwndDlg, IDC_OVLKEY_STATIC))
			{
				if (od->hbrSampleBackground) DeleteObject(od->hbrSampleBackground);
				od->hbrSampleBackground = CreateSolidBrush(od->clrKeyLast);
				return (BOOL)(od->hbrSampleBackground);
			}
			return FALSE;
		}

		case WM_DRAWITEM:
		{
			if (!od) break;

			LPDRAWITEMSTRUCT pDrawItem = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
			if (wParam == IDC_OVERLAY_PREVIEW_FRAME)
			{
				// Draw the overlay preview
				//
				if (od->imageComposite)
				{
					Graphics g(pDrawItem->hDC);
					g.DrawImage(od->imageComposite, 0, 0);
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
			if (pNmhdr->code == PSN_APPLY)
			{
				TCHAR buffer[512];

				GetDlgItemText(hwndDlg, IDC_OVERLAYFILENAME, buffer, 512);
				od->cam->sOverlayImageFile = buffer;

				// save overlay position
				LRESULT ovlpos = SendDlgItemMessage(hwndDlg, IDC_OVLPOS_LIST, CB_GETCURSEL, 0, 0);
				if (CB_ERR != ovlpos)
					od->cam->uOverlayPosition = ovlpos;

				// save opacity & scale
				od->cam->uOverlayOpacity = SendDlgItemMessage(hwndDlg, IDC_OVLOPACITY_SLIDER, TBM_GETPOS, 0, 0);
				od->cam->uOverlayScalePercent = SendDlgItemMessage(hwndDlg, IDC_OVLSCALE_SLIDER, TBM_GETPOS, 0, 0);

				// save scale type
				if (IsDlgButtonChecked(hwndDlg, IDC_OVLSCALE_PERCENT) == BST_CHECKED)
					od->cam->uOverlayScale = ovlscale_percentage;
				else if (IsDlgButtonChecked(hwndDlg, IDC_OVLSCALE_BESTFIT) == BST_CHECKED)
					od->cam->uOverlayScale = ovlscale_bestfit;
				else
					od->cam->uOverlayScale = ovlscale_original;

				// save colour key
				od->cam->bOverlayColourKey = (IsDlgButtonChecked(hwndDlg, IDC_OVLKEY_CHK) == BST_CHECKED);
				od->cam->rgbColourKey = od->clrKeyLast;

				od->cam->SaveOverlaySettings();
				PropSheet_CancelToClose(GetParent(hwndDlg));
				od->SetOverlayFile(od->cam->sOverlayImageFile.c_str());
			}
			break;
		}

		case WM_HSCROLL:
		{
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_OVLOPACITY_SLIDER))
			{
				UINT uPos = SendDlgItemMessage(hwndDlg, IDC_OVLOPACITY_SLIDER, TBM_GETPOS, 0, 0);
				SetDlgItemInt(hwndDlg, IDC_OVLOPACITY_EDIT, uPos, FALSE);
				PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
				od->UpdateComposite();
			}
			else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_OVLSCALE_SLIDER))
			{
				UINT uScale = SendDlgItemMessage(hwndDlg, IDC_OVLSCALE_SLIDER, TBM_GETPOS, 0, 0);
				SetDlgItemInt(hwndDlg, IDC_OVLSCALE_EDIT, uScale, FALSE);
				PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
				od->UpdateComposite();
			}
			break;
		}

		// Command message from dialog box
		//
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_OVLSCALE_ORIGINAL:
				case IDC_OVLSCALE_BESTFIT:
				case IDC_OVLSCALE_PERCENT:
				{
					od->InitScaleControls();
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					od->UpdateComposite();
					break;
				}

				case IDC_OVLKEY_CHOOSE:
				{

					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = od->clrKeyLast;
					cc.lpCustColors = customcolors;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

					if(ChooseColor(&cc))
					{
						od->clrKeyLast = cc.rgbResult;
						RedrawWindow(GetDlgItem(hwndDlg, IDC_OVLKEY_STATIC), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
						od->UpdateComposite();
					}
					break;

				}

				case IDC_OVERLAYFILE_CLEAR:
				{
					SetDlgItemText(hwndDlg, IDC_OVERLAYFILENAME, TEXT(""));
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					od->SetOverlayFile(TEXT(""));
					break;
				}

				case IDC_FILEBROWSE_OVERLAY:
				{
					TCHAR szMyPictures[MAX_PATH];
					BOOL bGotFolder;
					if (GetDllVersion(TEXT("shell32.dll")) >= PACKVERSION(5, 0))
					{
						bGotFolder = SHGetSpecialFolderPath(hwndDlg, szMyPictures, CSIDL_MYPICTURES, FALSE);
					}
					else
					{
						bGotFolder = SHGetSpecialFolderPath(hwndDlg, szMyPictures, CSIDL_PERSONAL, FALSE);
					}

					TCHAR szFilename[512];
					GetDlgItemText(hwndDlg, IDC_OVERLAYFILENAME, szFilename, 512);

					OPENFILENAME ofn;
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFilter = TEXT("All Images\0*.bmp;*.dib;*.gif;*.jpg;*.jpe;*.jpeg;*.jif;*.png;*.tif;*.tiff;*.wmf;*.emf\0JPEG Images\0*.jpg;*.jpe;*.jpeg;*.jif\0GIF Images\0*.gif\0PNG Images\0*.png\0BMP Images\0*.bmp;*.dib\0TIFF Images\0*.tif;*.tiff\0Windows Meta File Images\0*.wmf;*.emf\0All Files\0*.*\0\0");
					ofn.lpstrCustomFilter = NULL;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = szFilename;
					ofn.nMaxFile = 512;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					if (bGotFolder)
						ofn.lpstrInitialDir = szMyPictures;
					else
						ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = TEXT("Overlay Image");
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					ofn.lpstrDefExt = TEXT("jpg");
					ofn.pvReserved = NULL;
					ofn.dwReserved = 0;

					BOOL bGotFilename = GetOpenFileName(&ofn);

					if (bGotFilename)
					{
						SetDlgItemText(hwndDlg, IDC_OVERLAYFILENAME, szFilename);
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
						od->SetOverlayFile(szFilename);
					}

					break;
				}

				case IDC_OVERLAYFILENAME:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_OVLPOS_LIST:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
						od->UpdateComposite();
					}
					break;
				}

				case IDC_OVLKEY_CHK:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
						od->InitKeyControls();
						od->InitScaleControls();
						od->UpdateComposite();
					}
					break;
				}
			}

			break;
		}

		case WM_DESTROY:
		{
			if (od)
			{
				delete od;
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, NULL);
			}
			break;
		}

		// Initialize dialog
		//
		case WM_INITDIALOG:
		{
			PROPSHEETPAGE* psp = reinterpret_cast<PROPSHEETPAGE*>(lParam);

			RECT rClient;
			GetClientRect(GetDlgItem(hwndDlg, IDC_OVERLAY_PREVIEW_FRAME), &rClient);

			WebCam* cam = reinterpret_cast<WebCam*>(psp->lParam);
			OverlayDialog* od = new OverlayDialog(hwndDlg, rClient.right, rClient.bottom, cam->rgbColourKey);
			od->cam = cam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(od));

			Bitmap* imageBase = new Bitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_OVERLAYPREVIEW));

			od->imageBase = imageBase;

			SetDlgItemText(hwndDlg, IDC_OVERLAYFILENAME, cam->sOverlayImageFile.c_str());

			// Add options to overlay position combo box
			//
			TCHAR szPosition[255];
			for (unsigned i=0; i<9; ++i)
			{
				LoadString(GetModuleHandle(NULL), IDS_UPPERLEFT + i, szPosition, 255);
				SendDlgItemMessage(hwndDlg, IDC_OVLPOS_LIST, CB_ADDSTRING, 0,
					reinterpret_cast<LPARAM>(szPosition));
			}
			SendDlgItemMessage(hwndDlg, IDC_OVLPOS_LIST, CB_SETCURSEL, od->cam->uOverlayPosition, 0);

			switch (od->cam->uOverlayScale)
			{
				case ovlscale_percentage:
					CheckDlgButton(hwndDlg, IDC_OVLSCALE_PERCENT, BST_CHECKED);
					break;
				case ovlscale_bestfit:
					CheckDlgButton(hwndDlg, IDC_OVLSCALE_BESTFIT, BST_CHECKED);
					break;
				case ovlscale_original:
				default:
					CheckDlgButton(hwndDlg, IDC_OVLSCALE_ORIGINAL, BST_CHECKED);
			}

			if (od->cam->bOverlayColourKey)
			{
				CheckDlgButton(hwndDlg, IDC_OVLKEY_CHK, BST_CHECKED);
			}

			// Initialize scale trackbar
			//
			SendDlgItemMessage(hwndDlg, IDC_OVLSCALE_SLIDER, TBM_SETRANGE,
				FALSE, (LPARAM) MAKELONG (0, 400));
			SendDlgItemMessage(hwndDlg, IDC_OVLSCALE_SLIDER, TBM_SETTICFREQ,
				50, 0);
			SendDlgItemMessage(hwndDlg, IDC_OVLSCALE_SLIDER, TBM_SETPOS,
				TRUE, od->cam->uOverlayScalePercent);

			SetDlgItemInt(hwndDlg, IDC_OVLOPACITY_EDIT, od->cam->uOverlayOpacity, FALSE);
			SetDlgItemInt(hwndDlg, IDC_OVLSCALE_EDIT, od->cam->uOverlayScalePercent, FALSE);

			// Initialize opacity trackbar
			//
			SendDlgItemMessage(hwndDlg, IDC_OVLOPACITY_SLIDER, TBM_SETRANGE,
				FALSE, (LPARAM) MAKELONG (0, 100));
			SendDlgItemMessage(hwndDlg, IDC_OVLOPACITY_SLIDER, TBM_SETTICFREQ,
				20, 0);
			SendDlgItemMessage(hwndDlg, IDC_OVLOPACITY_SLIDER, TBM_SETPOS,
				TRUE, od->cam->uOverlayOpacity);

			od->InitKeyControls();
			od->InitScaleControls();

			od->SetOverlayFile(cam->sOverlayImageFile.c_str());

			break;
		}

    default:
		{
      return FALSE;
		}
  }

  return TRUE;
}
