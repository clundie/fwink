/*

Copyright (c) 2001-2006 Chris Lundie
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

// Video dialog callback
//
INT_PTR CALLBACK
WebCam::DialogProcVideo(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WebCam* cam = reinterpret_cast<WebCam*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

  switch(uMsg)
	{
		case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
			if (pNmhdr->code == PSN_APPLY)
			{
				BOOL bGotInt = FALSE;
				unsigned uValue = GetDlgItemInt(hwndDlg, IDC_CUSTOMSIZEX, &bGotInt, FALSE);
				if (bGotInt) cam->uCustomSizeX = uValue;
				uValue = GetDlgItemInt(hwndDlg, IDC_CUSTOMSIZEY, &bGotInt, FALSE);
				if (bGotInt) cam->uCustomSizeY = uValue;

				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_RADIOSTDSIZE))
					cam->bCustomImageSize = false;
				else
					cam->bCustomImageSize = true;
				
				uValue = GetDlgItemInt(hwndDlg, IDC_CROP_X, &bGotInt, FALSE);
				if (bGotInt) cam->uCropX = uValue;
				uValue = GetDlgItemInt(hwndDlg, IDC_CROP_Y, &bGotInt, FALSE);
				if (bGotInt) cam->uCropY = uValue;
				uValue = GetDlgItemInt(hwndDlg, IDC_CROP_WIDTH, &bGotInt, FALSE);
				if (bGotInt) cam->uCropWidth = uValue;
				uValue = GetDlgItemInt(hwndDlg, IDC_CROP_HEIGHT, &bGotInt, FALSE);
				if (bGotInt) cam->uCropHeight = uValue;

				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CROP_ENABLE))
					cam->bCrop = true;
				else
					cam->bCrop = false;

				cam->uImageSize = SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_GETCURSEL, 0, 0);
				cam->uImageSize = max(min(cam->uImageSize, uMaxImageSize), uMinImageSize);
				SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_SETCURSEL, cam->uImageSize, 0);
				cam->uJPEGQuality = SendDlgItemMessage(hwndDlg, IDC_JPEGQUALITY,
					TBM_GETPOS, 0, 0);

				cam->bFlipHorizontal = (IsDlgButtonChecked(hwndDlg, IDC_FLIPHORIZONTAL) == BST_CHECKED);
				cam->bFlipVertical = (IsDlgButtonChecked(hwndDlg, IDC_FLIPVERTICAL) == BST_CHECKED);

				if (cam->uDeviceType == deviceType_camera)
				{
					long lIndex = SendDlgItemMessage(hwndDlg, IDC_VIDEOSOURCE,
						CB_GETCURSEL, 0, 0);
					if (CB_ERR != lIndex)
					{
						long lSource = SendDlgItemMessage(hwndDlg, IDC_VIDEOSOURCE,
							CB_GETITEMDATA, lIndex, 0);
						if (CB_ERR != lSource)
						{
							cam->uCrossbarInput = lSource;
							IPin* pinVidcapIn = cam->GetPin(cam->pVidcap, PINDIR_INPUT, PIN_CATEGORY_ANALOGVIDEOIN);
							if (pinVidcapIn)
							{
								HRESULT hr;
								CCrossbar* pCCrossbar = new CCrossbar(pinVidcapIn, &hr);
								if (pCCrossbar)
								{
									pCCrossbar->SetInputIndex(lSource);
									delete pCCrossbar;
								}
								pinVidcapIn->Release();
							}
						}
					}
				}

				cam->GetCaptureSize();

				cam->SaveVideoSettings();
				cam->ApplyImageSize();
				cam->CropPreview();
				PropSheet_CancelToClose(GetParent(hwndDlg));
			}
			break;
		}

		case WM_HSCROLL:
		{
			PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			break;
		}

		// Command message from dialog box
		//
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CUSTOMSIZEX:
				case IDC_CUSTOMSIZEY:
				case IDC_CROP_X:
				case IDC_CROP_Y:
				case IDC_CROP_WIDTH:
				case IDC_CROP_HEIGHT:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_CROP_ENABLE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						BOOL bCrop = (IsDlgButtonChecked(hwndDlg, IDC_CROP_ENABLE) == BST_CHECKED) ? TRUE : FALSE;
						EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_X), bCrop ? TRUE : FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_Y), bCrop ? TRUE : FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_WIDTH), bCrop ? TRUE : FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_HEIGHT), bCrop ? TRUE : FALSE);
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_RADIOSTDSIZE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_IMAGESIZE), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMSIZEX), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMSIZEY), FALSE);
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_RADIOCUSTOMSIZE:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_IMAGESIZE), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMSIZEX), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMSIZEY), TRUE);
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_VIDEOSOURCE:
				case IDC_IMAGESIZE:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_FLIPHORIZONTAL:
				case IDC_FLIPVERTICAL:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
					}
					break;
				}

				case IDC_BTNTVTUNER:
				{
					cam->TVTunerProperties(hwndDlg, true);
					break;
				}

				case IDC_BTNVIDCAPPROPERTIES:
				{
					if (cam->uDeviceType == deviceType_camera)
					{
						cam->ChangeVidcapProperties(hwndDlg, true);
					}
					break;
				}

				case IDC_BTNVIDEOFORMAT:
				{
					if (cam->uDeviceType == deviceType_camera)
					{
						cam->StopTimer();
						cam->StopCapture();

						cam->ChangeVideoFormat(hwndDlg, true);

						cam->StartCapture();
						cam->StartTimer();

						PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
						PropSheet_CancelToClose(GetParent(hwndDlg));
					}

					break;
				}

				case IDC_BTN_CHANGE_VIDEO_DEVICE:
				{
					cam->ShowChooseVideoDeviceDialog(hwndDlg);
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

			// Initialize JPEG quality trackbar
			//
			SendDlgItemMessage(hwndDlg, IDC_JPEGQUALITY, TBM_SETRANGE,
				FALSE, (LPARAM) MAKELONG (0, 100));
			SendDlgItemMessage(hwndDlg, IDC_JPEGQUALITY, TBM_SETTICFREQ,
				25, 0);
			SendDlgItemMessage(hwndDlg, IDC_JPEGQUALITY, TBM_SETPOS,
				TRUE, cam->uJPEGQuality);

			// Add options to image size combo box
			//
			TCHAR buffer[256];
			LoadString(GetModuleHandle(NULL), IDS_IMAGESIZESMALL, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));
			LoadString(GetModuleHandle(NULL), IDS_IMAGESIZEMEDIUM, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));
			LoadString(GetModuleHandle(NULL), IDS_IMAGESIZELARGE, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));
			LoadString(GetModuleHandle(NULL), IDS_IMAGESIZEXLARGE, buffer, 256);
			SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>(buffer));

			// Set image size
			SendDlgItemMessage(hwndDlg, IDC_IMAGESIZE, CB_SETCURSEL, cam->uImageSize, 0);

			SetDlgItemInt(hwndDlg, IDC_CUSTOMSIZEX, cam->uCustomSizeX, FALSE);
			SetDlgItemInt(hwndDlg, IDC_CUSTOMSIZEY, cam->uCustomSizeY, FALSE);

			if(cam->bCustomImageSize)
			{
				CheckDlgButton(hwndDlg, IDC_RADIOCUSTOMSIZE, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_IMAGESIZE), FALSE);
			}
			else
			{
				CheckDlgButton(hwndDlg, IDC_RADIOSTDSIZE, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMSIZEX), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMSIZEY), FALSE);
			}
			
			// Set crop settings
			SetDlgItemInt(hwndDlg, IDC_CROP_X, cam->uCropX, FALSE);
			SetDlgItemInt(hwndDlg, IDC_CROP_Y, cam->uCropY, FALSE);
			SetDlgItemInt(hwndDlg, IDC_CROP_WIDTH, cam->uCropWidth, FALSE);
			SetDlgItemInt(hwndDlg, IDC_CROP_HEIGHT, cam->uCropHeight, FALSE);
			CheckDlgButton(hwndDlg, IDC_CROP_ENABLE, cam->bCrop ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_X), cam->bCrop ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_Y), cam->bCrop ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_WIDTH), cam->bCrop ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CROP_HEIGHT), cam->bCrop ? TRUE : FALSE);

			// Load settings into dialog
			CheckDlgButton(hwndDlg, IDC_FLIPHORIZONTAL, cam->bFlipHorizontal ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FLIPVERTICAL, cam->bFlipVertical ? BST_CHECKED : BST_UNCHECKED);

			// set up crossbar

			if (cam->uDeviceType == deviceType_camera)
			{


				EnableWindow(GetDlgItem(hwndDlg, IDC_VIDEOSOURCE), FALSE);

				HRESULT hr;

				IPin* pinVidcapIn = cam->GetPin(cam->pVidcap, PINDIR_INPUT, PIN_CATEGORY_ANALOGVIDEOIN);
				if (pinVidcapIn)
				{
					HRESULT hrCrossbar;
					CCrossbar* pCCrossbar = new CCrossbar(pinVidcapIn, &hrCrossbar);
					if (pCCrossbar)
					{
						LONG lCount = 0;
						hr = pCCrossbar->GetInputCount(&lCount);
				
						for (LONG l=0; l<lCount; ++l)
						{
							LONG lType;
							hr = pCCrossbar->GetInputType(l, &lType);
							if (SUCCEEDED(hr))
							{
								TCHAR buffer[256];
								hr = pCCrossbar->GetInputName(l, buffer, 256);
								if (SUCCEEDED(hr))
								{
									long lIndex = SendDlgItemMessage(hwndDlg, IDC_VIDEOSOURCE,
										CB_ADDSTRING,
										0, (LPARAM)buffer);
									if (CB_ERR != lIndex)
									{
										EnableWindow(GetDlgItem(hwndDlg, IDC_VIDEOSOURCE), TRUE);
										SendDlgItemMessage(hwndDlg, IDC_VIDEOSOURCE,
											CB_SETITEMDATA,
											lIndex,
											(LPARAM)l);

										if (l == cam->uCrossbarInput)
										{
											SendDlgItemMessage(hwndDlg, IDC_VIDEOSOURCE,
												CB_SETCURSEL, lIndex, 0);
											SendDlgItemMessage(hwndDlg, IDC_VIDEOSOURCE,
												CB_SETTOPINDEX, lIndex, 0);
										}
									}
								}

								if (l == cam->uCrossbarInput)
								{
									pCCrossbar->SetInputIndex(l);
									//break;
								}
							}
						}
						delete pCCrossbar;
					}
					pinVidcapIn->Release();
				}

				// Check if there is a TV tuner
				//
				if (cam->pBuilder)
				{
					IAMTVTuner* pTuner = NULL;
					hr = cam->pBuilder->FindInterface(
						&LOOK_UPSTREAM_ONLY,
						NULL,
						cam->pVidcap,
						IID_IAMTVTuner,
						(void**)&pTuner);
					if (FAILED(hr))
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTNTVTUNER), FALSE);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTNTVTUNER), TRUE);
						pTuner->Release();
					}
				}
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_VIDEOSOURCE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNTVTUNER), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNVIDCAPPROPERTIES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNVIDEOFORMAT), FALSE);
			}

			// Set video device name
			SetDlgItemText(hwndDlg, IDC_VIDEO_DEVICE_NAME, cam->sDeviceName.c_str());

			break;
		}

    default:
		{
      return FALSE;
		}
  }

  return TRUE;
}
