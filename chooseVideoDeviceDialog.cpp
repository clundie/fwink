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
					HWND hwndComboBox = GetDlgItem(hwndDlg, IDC_COMBO_VIDEO_DEVICE);

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
						if (device->devicePath != cam->sDevicePath)
						{
							TCHAR message[256];
							LoadString(GetModuleHandle(NULL), IDS_DEVICE_CHANGED, message, 255);
							TCHAR messageTitle[256];
							LoadString(GetModuleHandle(NULL), IDS_DEVICE_CHANGED_TITLE, messageTitle, 255);
							MessageBox(hwndDlg, message, messageTitle, MB_OK | MB_ICONINFORMATION);
						}
					}

					EndDialog(hwndDlg, (INT_PTR)1);
					break;
				}
			case IDCANCEL:
				EndDialog(hwndDlg, (INT_PTR)2);
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
			if ((index == 0) || (device.devicePath == this->sDevicePath))
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
