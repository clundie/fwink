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
#include "resource.h"
#include <comdef.h>
#include <shlobj.h>

_COM_SMARTPTR_TYPEDEF(ICreateDevEnum, IID_ICreateDevEnum);

// Choose video device dialog callback
//
INT_PTR CALLBACK
WebCam::DialogProcChooseVideoDevice(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WebCam* cam = reinterpret_cast<WebCam*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch(uMsg)
	{
		// Command message from dialog box
		//
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					bool bChanged = false;

					HWND hwndComboBox = GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE);
					unsigned int newDeviceType;
					if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_DEVICE_FILE))
					{
						newDeviceType = deviceType_file;
						TCHAR filePathBuffer[MAX_PATH];
						UINT filePathLength = GetDlgItemText(hwndDlg, IDC_FILE_DEVICE_PATH, filePathBuffer, MAX_PATH);
						wstring filePath;
						if (filePathLength)
						{
							filePath = filePathBuffer;
						}
						cam->SaveSetting(L"devicePath", filePath.c_str());
						if ((cam->uDeviceType == deviceType_file)
							&& (filePath != cam->sDevicePath))
						{
							bChanged = true;
						}
					}
					else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_DEVICE_CAMERA))
					{
						newDeviceType = deviceType_camera;
						LRESULT index = SendMessage(
								hwndComboBox,
								CB_GETCURSEL,
								0, 0);
						if (index != CB_ERR)
						{
							LRESULT deviceIndex = SendMessage(
								hwndComboBox,
								CB_GETITEMDATA,
								(WPARAM)index,
								0);
							VideoDevice *device = &(cam->videoDevices[deviceIndex]);
							cam->SaveSetting(L"devicePath", device->devicePath.c_str());
							if ((cam->uDeviceType == deviceType_camera)
								&& (device->devicePath != cam->sDevicePath))
							{
								bChanged = true;
							}
						}
					}
					else
					{
						assert(false);
					}

					cam->SaveSetting(L"deviceType", newDeviceType);
					if (newDeviceType != cam->uDeviceType)
					{
						bChanged = true;
					}

					if (bChanged)
					{
						TCHAR message[256];
						LoadString(GetModuleHandle(NULL), IDS_DEVICE_CHANGED, message, 255);
						TCHAR messageTitle[256];
						LoadString(GetModuleHandle(NULL), IDS_DEVICE_CHANGED_TITLE, messageTitle, 255);
						MessageBox(hwndDlg, message, messageTitle, MB_OK | MB_ICONINFORMATION);
					}

					EndDialog(hwndDlg, (INT_PTR)1);
					break;
				}
			case IDCANCEL:
				EndDialog(hwndDlg, (INT_PTR)2);
				break;
			case IDC_RADIO_DEVICE_CAMERA:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_DEVICE_BROWSE), FALSE);
				}
				break;
			case IDC_RADIO_DEVICE_FILE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_DEVICE_BROWSE), TRUE);
				}
				break;
			case IDC_FILE_DEVICE_BROWSE:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					TCHAR szFilename[MAX_PATH];
					GetDlgItemText(hwndDlg, IDC_FILE_DEVICE_PATH, szFilename, MAX_PATH);

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

					OPENFILENAME ofn;
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFilter = TEXT("All Images\0*.bmp;*.dib;*.gif;*.jpg;*.jpe;*.jpeg;*.jif;*.png;*.tif;*.tiff;*.wmf;*.emf\0JPEG Images\0*.jpg;*.jpe;*.jpeg;*.jif\0GIF Images\0*.gif\0PNG Images\0*.png\0BMP Images\0*.bmp;*.dib\0TIFF Images\0*.tif;*.tiff\0Windows Meta File Images\0*.wmf;*.emf\0All Files\0*.*\0\0");
					ofn.lpstrCustomFilter = NULL;
					ofn.nFilterIndex = 1;
					ofn.lpstrFile = szFilename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					if (bGotFolder)
						ofn.lpstrInitialDir = szMyPictures;
					else
						ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = TEXT("Image File");
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
					ofn.lpstrDefExt = TEXT("jpg");
					ofn.pvReserved = NULL;
					ofn.dwReserved = 0;

					BOOL bGotFilename = GetOpenFileName(&ofn);

					if (bGotFilename)
					{
						SetDlgItemText(hwndDlg, IDC_FILE_DEVICE_PATH, szFilename);
					}

				}
				break;
			}

			break;
		}

		// Initialize dialog
		//
		case WM_INITDIALOG:
		{
			WebCam* cam = reinterpret_cast<WebCam*>(lParam);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cam));
			cam->EnumerateVideoDevicesForComboBox(GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE));
			if (cam->uDeviceType == deviceType_camera)
			{
				CheckDlgButton(hwndDlg, IDC_RADIO_DEVICE_CAMERA, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_DEVICE_BROWSE), FALSE);
			}
			else if (cam->uDeviceType == deviceType_file)
			{
				CheckDlgButton(hwndDlg, IDC_RADIO_DEVICE_FILE, BST_CHECKED);
				SetDlgItemText(hwndDlg, IDC_FILE_DEVICE_PATH, cam->sDevicePath.c_str());
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_DEVICE_BROWSE), TRUE);
			}
			else
			{
				assert(false);
			}
			break;
		}

	    default:
		{
			return FALSE;
		}
	}

	return TRUE;
}

void WebCam::EnumerateVideoDevicesForComboBox(HWND hwndComboBox)
{
	HRESULT hr;

	// Create the system device enumerator.
	ICreateDevEnumPtr pDevEnum;
	hr = pDevEnum.CreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC);
	if (S_OK != hr)
	{
		return;
	}

	// Create an enumerator for video capture devices.
	IEnumMonikerPtr pClassEnum;
	pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr) || (pClassEnum == NULL))
	{
		return;
	}

	ULONG cFetched;
	IMonikerPtr pMoniker;

	videoDevices.clear();

	while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		VideoDevice device;
		// Get the property bag of the moniker
		{
			IPropertyBagPtr propBag;
			hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag,
					reinterpret_cast<void**>(&propBag));
			if (FAILED(hr))
			{
				pMoniker.Release();
				continue;
			}

			_variant_t v;
			hr = propBag->Read(L"Description", &v, 0);
			if (FAILED(hr))
			{
				hr = propBag->Read(L"FriendlyName", &v, 0);
			}
			if (S_OK == hr)
			{
				device.friendlyName = v.bstrVal;
			}
			hr = propBag->Read(L"DevicePath", &v, 0);
			if (S_OK == hr)
			{
				device.devicePath = v.bstrVal;
			}

			videoDevices.push_back(device);

			LRESULT index = SendMessage(
				hwndComboBox,
				CB_ADDSTRING,
				(WPARAM)0,
				(LPARAM)device.friendlyName.c_str());
			if (index < 0)
			{
				pMoniker.Release();
				continue;
			}
			SendMessage(
				hwndComboBox,
				CB_SETITEMDATA,
				(WPARAM)index,
				(LPARAM)(DWORD)videoDevices.size() - 1);
			if ((index == 0) || ((uDeviceType == deviceType_camera) && (device.devicePath == this->sDevicePath)))
			{
				SendMessage(
					hwndComboBox,
					CB_SETCURSEL,
					(WPARAM)index,
					(LPARAM)0);
			}

		}

		pMoniker.Release();
	}
}
