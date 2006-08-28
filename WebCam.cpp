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

const UINT WebCam::uNotifyIconID = 0;
const UINT WebCam::uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
const unsigned WebCam::uMaxTimeValue = 999, WebCam::uMinTimeValue = 1;
const unsigned WebCam::uMinTimeUnit = 0, WebCam::uMaxTimeUnit = 2;
const unsigned WebCam::uMinImageSize = 0, WebCam::uMaxImageSize = 3;
const unsigned WebCam::uMinFontSize = 1, WebCam::uMaxFontSize = 999;
const unsigned WebCam::uMinSmooth = 0, WebCam::uMaxSmooth = 2;
const TCHAR WebCam::szHomePageURL[] = TEXT("http://lundie.ca/fwink/version.php?version=0.9.9.3");
const int WebCam::_expireTime =  1082174400;

WebCam::WebCam()
: hwndMain(NULL),
	uImageWidth(320), uImageHeight(240),
	hInternet(NULL), hFTP(NULL),
	iStatus(IDS_IDLE),
	bTempFile(false), bReadyToUpload(true),
	uTimeValue(1), uTimeUnit(timeunit_minutes),
	sServer(L""),sUsername(L""),sPassword(L""),sDirectory(L""),sFilename(L"webcam.jpg"),
	sWisdom(L""),
	uTimerID(1),
	bRunAtStartup(false), bTimestamp(true), bDatestamp(true),
	pGraph(NULL), pVidcap(NULL), pSampleGrabber(NULL),
	pNullRenderer(NULL), pBuilder(NULL),
	bOKToDeleteTempFile(true),
	uImageSize(1), uFontSize(14),
	sFontname(L"Arial"),
	bFontBold(false), bFontItalic(false), bFontUnderline(false), bFontStrikeout(false),
	bFlipHorizontal(false), bFlipVertical(false),
	uSmoothType(smooth_standard),
	uCrossbarInput(0),
	iCaptureWidth(320), iCaptureHeight(240),
	hbrSampleBackground(NULL),
	bPassiveFTP(false),
	uTextPosition(textposition_bottom),
	bDropShadow(true),
	uTextBackground(textbackground_translucent),
	uJPEGQuality(75),
	bOneShot(false),
	bDisableFTP(false),
	bDisableTimer(false),
	bSaveToFile(false),
	sLocalFilename(L""),
	uOverlayScale(ovlscale_original),
	uOverlayScalePercent(100),
	uOverlayPosition(textposition_lowerleft),
	uOverlayOpacity(100),
	bOverlayColourKey(false),
	rgbColourKey(RGB(255, 255, 255)),
	bCustomImageSize(false),
	uCustomSizeX(320), uCustomSizeY(240),
	bCrop(false),
	uCropX(0), uCropY(0), uCropWidth(320), uCropHeight(240)
{
	rgbForeground = RGB(255, 255, 128);
	rgbBackground = RGB(96, 16, 16);

	UINT uSmoothingType;
	if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &uSmoothingType, 0))
		bClearTypeAvailable = true;
	else
		bClearTypeAvailable = false;

	LoadString(GetModuleHandle(NULL), IDS_APPNAME, szAppname, 512);

	if (FAILED(CoInitialize(NULL)))
		throw ERR_COINIT;

	try
	{
		DWORD dwFlags;
		if (InternetGetConnectedState(&dwFlags, 0))
		{
			if (dwFlags & INTERNET_CONNECTION_MODEM)
				bDialup = true;
			else
				bDialup = false;
		}
		else
		{
			bDialup = false;
		}

		if (GetDllVersion(TEXT("shell32.dll")) >= PACKVERSION(5, 0))
			bBalloonTips = true;
		else
			bBalloonTips = false;

		// Load notification icon
		//
		hIconIdle = static_cast<HICON>(LoadImage(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDI_APPICON),
			IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR));

		// Initialize GDI+
		//
		GdiplusStartupInput startup;
		if (Ok != GdiplusStartup(&uGDIPlusToken, &startup, NULL))
			throw ERR_GDIPLUSSTARTUP;

		// Open a Win32 Internet session
		//
		try
		{
			hInternet = InternetOpen(NULL,
				INTERNET_OPEN_TYPE_PRECONFIG,
				NULL,
				NULL,
				INTERNET_FLAG_ASYNC);

			if (hInternet)
			{
				// Set Internet status callback
				//
				if (INTERNET_INVALID_STATUS_CALLBACK == InternetSetStatusCallback(
					hInternet, InternetStatusCallback))
				{
					InternetCloseHandle(hInternet);
					throw ERR_INTERNETOPEN;
				}


				// Set timeout values to prevent FTP
				// transfer from getting stuck.

				unsigned long optionValue = 30000;

				if (
					FALSE == InternetSetOption(
						hInternet,
						INTERNET_OPTION_CONNECT_TIMEOUT,
						&optionValue,
						sizeof(optionValue)
					)
				)
				{
					InternetCloseHandle(hInternet);
					throw ERR_INTERNETOPEN;
				}

				if (
					FALSE == InternetSetOption(
						hInternet,
						INTERNET_OPTION_RECEIVE_TIMEOUT,
						&optionValue,
						sizeof(optionValue)
					)
				)
				{
					InternetCloseHandle(hInternet);
					throw ERR_INTERNETOPEN;
				}

				if (
					FALSE == InternetSetOption(
						hInternet,
						INTERNET_OPTION_SEND_TIMEOUT,
						&optionValue,
						sizeof(optionValue)
					)
				)
				{
					InternetCloseHandle(hInternet);
					throw ERR_INTERNETOPEN;
				}
			}
			else
			{
				throw ERR_INTERNETOPEN;
			}
		}
		catch (int)
		{
			GdiplusShutdown(uGDIPlusToken);
			throw;
		}
	}
	catch (int)
	{
		CoUninitialize();
		throw;
	}
}

WebCam::~WebCam()
{
	if (IsWindow(hwndMain))
	{
		NOTIFYICONDATA data;
		data.cbSize = NOTIFYICONDATA_V1_SIZE;
		data.hWnd = hwndMain;
		data.uID = uNotifyIconID;
		data.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &data);

		DestroyWindow(hwndMain);
		hwndMain = NULL;
	}

	if (hFTP)
	{
//		InternetSetStatusCallback(hFTP, NULL);
		InternetCloseHandle(hFTP);
		hFTP = NULL;
	}
	if (hInternet)
	{
//		InternetSetStatusCallback(hInternet, NULL);
		InternetCloseHandle(hInternet);
		hInternet = NULL;
	}

	DeleteTempFile();
	GdiplusShutdown(uGDIPlusToken);
	ShutdownDirectShow();
	CoUninitialize();

	if (hbrSampleBackground)
		DeleteObject(hbrSampleBackground);
	hbrSampleBackground = NULL;
}

void
WebCam::StartOneShot(void)
{
	bOneShot = true;
	Start();
}

void
WebCam::Start(void)
{
	// Load settings
	//
	LoadSetting(L"server", sServer);
	LoadSetting(L"user", sUsername);
	LoadSetting(L"password", sPassword);
	LoadSetting(L"directory", sDirectory);
	LoadSetting(L"filename", sFilename);
	LoadSetting(L"timevalue", &(uTimeValue));
	LoadSetting(L"timeunit", &(uTimeUnit));
	LoadSetting(L"dialup", &(bDialup));
	LoadSetting(L"passiveftp", &(bPassiveFTP));
	LoadSetting(L"runatstartup", &(bRunAtStartup));
	LoadSetting(L"timestamp", &(bTimestamp));
	LoadSetting(L"datestamp", &(bDatestamp));
	LoadSetting(L"wisdom", sWisdom);
	LoadSetting(L"imagesize", &(uImageSize));
	LoadSetting(L"fontsize", &(uFontSize));
	LoadSetting(L"fontname", sFontname);
	LoadSetting(L"fontbold", &(bFontBold));
	LoadSetting(L"fontitalic", &(bFontItalic));
	LoadSetting(L"fontunderline", &(bFontUnderline));
	LoadSetting(L"fontstrikeout", &(bFontStrikeout));
	LoadSetting(L"smoothtype", &(uSmoothType));
	LoadSetting(L"crossbarinput", &(uCrossbarInput));
	LoadSetting(L"capturewidth", &(iCaptureWidth));
	LoadSetting(L"captureheight", &(iCaptureHeight));
	LoadSetting(L"fliphorizontal", &(bFlipHorizontal));
	LoadSetting(L"flipvertical", &(bFlipVertical));
	LoadSetting(L"textposition", &(uTextPosition));
	LoadSetting(L"dropshadow", &(bDropShadow));
	LoadSetting(L"textbackground", &(uTextBackground));
	LoadSetting(L"JPEGquality", &(uJPEGQuality));
	LoadSetting(L"disableFTP", &(bDisableFTP));
	LoadSetting(L"disableTimer", &(bDisableTimer));
	LoadSetting(L"savetofile", &(bSaveToFile));
	LoadSetting(L"localfilename", sLocalFilename);
	LoadSetting(L"overlayfilename", sOverlayImageFile);
	LoadSetting(L"overlayscale", &(uOverlayScale));
	LoadSetting(L"overlayscalepercent", &(uOverlayScalePercent)),
	LoadSetting(L"overlayposition", &(uOverlayPosition));
	LoadSetting(L"overlayopacity", &(uOverlayOpacity));
	LoadSetting(L"overlaycolourkey", &(bOverlayColourKey));
	LoadSetting(L"UseCustomImageSize", &bCustomImageSize);
	LoadSetting(L"CustomSizeX", &uCustomSizeX);
	LoadSetting(L"CustomSizeY", &uCustomSizeY);
	LoadSetting(L"CropX", &uCropX);
	LoadSetting(L"CropY", &uCropY);
	LoadSetting(L"CropWidth", &uCropWidth);
	LoadSetting(L"CropHeight", &uCropHeight);
	LoadSetting(L"CropEnable", &bCrop);
	
	unsigned r, g, b;

	r = GetRValue(rgbBackground);
	g = GetGValue(rgbBackground);
	b = GetBValue(rgbBackground);

	LoadSetting(L"BackgroundR", &r);
	LoadSetting(L"BackgroundG", &g);
	LoadSetting(L"BackgroundB", &b);
	rgbBackground = RGB(min(r, 255), min(g, 255), min(b, 255));

	r = GetRValue(rgbForeground);
	g = GetGValue(rgbForeground);
	b = GetBValue(rgbForeground);

	LoadSetting(L"ForegroundR", &r);
	LoadSetting(L"ForegroundG", &g);
	LoadSetting(L"ForegroundB", &b);
	rgbForeground = RGB(min(r, 255), min(g, 255), min(b, 255));

	r = GetRValue(rgbColourKey);
	g = GetGValue(rgbColourKey);
	b = GetBValue(rgbColourKey);

	LoadSetting(L"ColourKeyR", &r);
	LoadSetting(L"ColourKeyG", &g);
	LoadSetting(L"ColourKeyB", &b);
	rgbColourKey = RGB(min(r, 255), min(g, 255), min(b, 255));

	uTimeValue = max(min(uTimeValue, uMaxTimeValue), uMinTimeValue);
	uTimeUnit = max(min(uTimeUnit, uMaxTimeUnit), uMinTimeUnit);
	uImageSize = max(min(uImageSize, uMaxImageSize), uMinImageSize);
	uFontSize = max(min(uFontSize, uMaxFontSize), uMinFontSize);
	uSmoothType = max(min(uSmoothType, uMaxSmooth), uMinSmooth);

	if ((uSmoothType == smooth_cleartype) && (!bClearTypeAvailable))
		uSmoothType = smooth_standard;

	ApplyImageSize();
	CreateMainDialog();
	CreateNotificationIcon();
	InitDirectShow();

	// Check if this is the first time the app has run.
	{
		unsigned uFirstRun = 1;
		LoadSetting(L"FirstRun", &uFirstRun);
		if (uFirstRun)
		{
			uFirstRun = 0;
			SaveSetting(L"FirstRun", uFirstRun);
			ShowWindow(hwndMain, SW_SHOWNORMAL);
			ShowSettingsDialog();
		}
	}

	CaptureFrame();
	StartTimer();

	// Windows message loop
	MSG msg;
	BOOL bGotMessage;

	while ((bGotMessage = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bGotMessage == -1)
			break;

		//TranslateAccelerator(hwndDlg, hAccelerators, &msg);
		if (!IsDialogMessage(hwndMain, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	StopTimer();
	SaveSettings();
}

void
WebCam::ShowMainDialog(void)
{
	if (!IsWindow(hwndMain)) return;

	ShowWindow(hwndMain, SW_SHOW);
	SetForegroundWindow(hwndMain);
	SetActiveWindow(hwndMain);
	SetFocus(hwndMain);
}

void
WebCam::CreateNotificationIcon(void)
{
	if (!IsWindow(hwndMain)) return;

	NOTIFYICONDATA data;
	data.cbSize = NOTIFYICONDATA_V1_SIZE;
	data.hWnd = hwndMain;
	data.uID = uNotifyIconID;
	data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	data.hIcon = hIconIdle;
	data.uCallbackMessage = uNotifyIconMessage;
	LoadString(GetModuleHandle(NULL), IDS_IDLE, data.szTip, 64);

	if (Shell_NotifyIcon(NIM_ADD, &data) == FALSE)
		throw ERR_CREATEICON;
}

void
WebCam::CreateMainDialog(void)
{
	// Create the main dialog
	//
	hwndMain = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN),
																NULL, DialogProcMain, reinterpret_cast<LPARAM>(this));
	if (!hwndMain)
		throw ERR_CREATEMAINWINDOW;

	ShowWindow(hwndMain, SW_SHOWNORMAL);
	ShowWindow(hwndMain, SW_HIDE);
}

void
WebCam::SetStatus(int iStatus)
{
	this->iStatus = iStatus;
	TCHAR buffer[512];
	LoadString(GetModuleHandle(NULL), iStatus, buffer, 512);
	SetDlgItemText(hwndMain, IDC_STATUS, buffer);

	if (iStatus == STATUS_CONNECTING)
	{
		EnableWindow(GetDlgItem(hwndMain, IDC_BTNCAPTURE), FALSE);
		bReadyToUpload = false;
	}
	else if ((iStatus == ERR_UPLOAD) || (iStatus == SUCCESS_UPLOADED_IMAGE)
			|| (iStatus == ERR_CONNECT) || (iStatus == ERR_UPLOAD_DIRECTORY))
	{
		EnableWindow(GetDlgItem(hwndMain, IDC_BTNCAPTURE), TRUE);
		bReadyToUpload = true;
	}

	// Display balloon tooltip, if applicable
	//
	switch(iStatus)
	{
		case ERR_UPLOAD_DIRECTORY:
		case ERR_INTERNETOPEN:
		case ERR_UPLOAD:
		case ERR_CONNECT:
		case ERR_CAPTURE:
		{
			if (bBalloonTips && !IsWindowVisible(hwndMain))
			{
				NOTIFYICONDATA data;
				data.cbSize = sizeof(NOTIFYICONDATA);
				data.hWnd = hwndMain;
				data.uID = uNotifyIconID;
				data.uFlags = NIF_INFO;

				TCHAR titlebuffer[64];
				LoadString(GetModuleHandle(NULL), ERR_BALLOONTITLE, titlebuffer, 64);

				lstrcpyn(data.szInfo, buffer, 255);
				data.uTimeout = 10000;
				lstrcpyn(data.szInfoTitle, titlebuffer, 63);
				data.dwInfoFlags = NIIF_ERROR;

				Shell_NotifyIcon(NIM_MODIFY, &data);
			}
			break;
		}
	}

}

void
WebCam::SaveEffectsSettings(void)
{
	SaveSetting(L"wisdom", sWisdom.c_str());
	SaveSetting(L"timestamp", bTimestamp);
	SaveSetting(L"datestamp", bDatestamp);
	SaveSetting(L"fontsize", uFontSize);
	SaveSetting(L"fontname", sFontname.c_str());
	SaveSetting(L"fontbold", bFontBold);
	SaveSetting(L"fontitalic", bFontItalic);
	SaveSetting(L"fontunderline", bFontUnderline);
	SaveSetting(L"fontstrikeout", bFontStrikeout);
	SaveSetting(L"smoothtype", uSmoothType);
	SaveSetting(L"textposition", uTextPosition);
	SaveSetting(L"dropshadow", bDropShadow);
	SaveSetting(L"textbackground", uTextBackground);

	unsigned u = GetRValue(rgbBackground);
	SaveSetting(L"BackgroundR", u);
	u = GetGValue(rgbBackground);
	SaveSetting(L"BackgroundG", u);
	u = GetBValue(rgbBackground);
	SaveSetting(L"BackgroundB", u);

	u = GetRValue(rgbForeground);
	SaveSetting(L"ForegroundR", u);
	u = GetGValue(rgbForeground);
	SaveSetting(L"ForegroundG", u);
	u = GetBValue(rgbForeground);
	SaveSetting(L"ForegroundB", u);
}

void
WebCam::SaveVideoSettings(void)
{
	SaveSetting(L"imagesize", uImageSize);
	SaveSetting(L"crossbarinput", uCrossbarInput);
	SaveSetting(L"capturewidth", iCaptureWidth);
	SaveSetting(L"captureheight", iCaptureHeight);
	SaveSetting(L"fliphorizontal", bFlipHorizontal);
	SaveSetting(L"flipvertical", bFlipVertical);
	SaveSetting(L"JPEGquality", uJPEGQuality);
	SaveSetting(L"UseCustomImageSize", bCustomImageSize);
	SaveSetting(L"CustomSizeX", uCustomSizeX);
	SaveSetting(L"CustomSizeY", uCustomSizeY);
	SaveSetting(L"CropX", uCropX);
	SaveSetting(L"CropY", uCropY);
	SaveSetting(L"CropWidth", uCropWidth);
	SaveSetting(L"CropHeight", uCropHeight);
	SaveSetting(L"CropEnable", bCrop);
}

void
WebCam::SaveFTPSettings(void)
{
	SaveSetting(L"server", sServer.c_str());
	SaveSetting(L"user", sUsername.c_str());
	SaveSetting(L"password", sPassword.c_str());
	SaveSetting(L"directory", sDirectory.c_str());
	SaveSetting(L"filename", sFilename.c_str());
	SaveSetting(L"timevalue", uTimeValue);
	SaveSetting(L"timeunit", uTimeUnit);
	SaveSetting(L"dialup", bDialup);
	SaveSetting(L"passiveftp", bPassiveFTP);
	SaveSetting(L"runatstartup", bRunAtStartup);
	SaveSetting(L"disableFTP", bDisableFTP);
	SaveSetting(L"disableTimer", bDisableTimer);
	SaveSetting(L"savetofile", bSaveToFile);
	SaveSetting(L"localfilename", sLocalFilename.c_str());

	HKEY hkey;
  if (RegCreateKeyEx(HKEY_CURRENT_USER,
			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL, 0,
			KEY_SET_VALUE, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		if (bRunAtStartup)
		{
			TCHAR buffer[512];
			if (GetModuleFileName(GetModuleHandle(NULL), buffer, 512))
			{
				RegSetValueEx(hkey, TEXT("ChrisLundieFwink"), 0, REG_SZ,
											reinterpret_cast<CONST BYTE *>(buffer),
											(lstrlen(buffer) + 1) * sizeof(TCHAR));
			}
		}
		else
		{
			RegDeleteValue(hkey, TEXT("ChrisLundieFwink"));
		}
		RegCloseKey(hkey);
	}
}

void
WebCam::SaveOverlaySettings(void)
{
	SaveSetting(L"overlayfilename", sOverlayImageFile.c_str());
	SaveSetting(L"overlayscale", uOverlayScale);
	SaveSetting(L"overlayscalepercent", uOverlayScalePercent);
	SaveSetting(L"overlayposition", uOverlayPosition);
	SaveSetting(L"overlayopacity", uOverlayOpacity);
	SaveSetting(L"overlaycolourkey", bOverlayColourKey);

	unsigned u = GetRValue(rgbColourKey);
	SaveSetting(L"ColourKeyR", u);
	u = GetGValue(rgbColourKey);
	SaveSetting(L"ColourKeyG", u);
	u = GetBValue(rgbColourKey);
	SaveSetting(L"ColourKeyB", u);
}

void
WebCam::SaveSettings(void)
{
	SaveEffectsSettings();
	SaveVideoSettings();
	SaveFTPSettings();
}

void
WebCam::StartTimer(void)
{
	unsigned multiplier = 1000;
	if (uTimeUnit == timeunit_minutes) multiplier = 60000;
	else if (uTimeUnit == timeunit_hours) multiplier = 3600000;

	SetTimer(hwndMain, uTimerID, uTimeValue * multiplier, NULL);
}

void
WebCam::StopTimer(void)
{
	KillTimer(hwndMain, uTimerID);
}

void
WebCam::DeleteTempFile(void)
{
	if (bTempFile)
	{
		DWORD dwStartTime = GetTickCount();
		while (!bOKToDeleteTempFile)
		{
			if (((GetTickCount() - dwStartTime) > 10000) || (GetTickCount() < dwStartTime))
				break;
		}
		DeleteFile(szwFilenameJPEG);
	}
}

void
WebCam::ApplyImageSize(void)
{
	if (bCustomImageSize)
	{
		uImageWidth = uCustomSizeX;
		uImageHeight = uCustomSizeY;
	}
	else
	{
		switch(uImageSize)
		{
			case 0:
				uImageWidth = 160;
				uImageHeight = 120;
				break;
			case 2:
				uImageWidth = 480;
				uImageHeight = 360;
				break;
			case 3:
				uImageWidth = 640;
				uImageHeight = 480;
				break;
			default:
				uImageWidth = 320;
				uImageHeight = 240;
				break;
		}
	}
}

void
WebCam::ShowSettingsDialog(void)
{
	HPROPSHEETPAGE hPsp[4];
	PROPSHEETPAGE psp;

	psp.dwSize = PROPSHEETPAGE_V1_SIZE;
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_FTP);
	psp.pfnDlgProc = DialogProcFTP;
	psp.lParam = reinterpret_cast<LPARAM>(this);
	psp.pfnCallback = NULL;
	hPsp[0] = CreatePropertySheetPage(&psp);

	psp.dwSize = PROPSHEETPAGE_V1_SIZE;
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_VIDEO);
	psp.pfnDlgProc = DialogProcVideo;
	psp.lParam = reinterpret_cast<LPARAM>(this);
	hPsp[1] = CreatePropertySheetPage(&psp);

	psp.dwSize = PROPSHEETPAGE_V1_SIZE;
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_EFFECTS);
	psp.pfnDlgProc = DialogProcEffects;
	psp.lParam = reinterpret_cast<LPARAM>(this);
	hPsp[2] = CreatePropertySheetPage(&psp);

	psp.dwSize = PROPSHEETPAGE_V1_SIZE;
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_OVERLAY);
	psp.pfnDlgProc = DialogProcOverlay;
	psp.lParam = reinterpret_cast<LPARAM>(this);
	hPsp[3] = CreatePropertySheetPage(&psp);

	if (!hPsp[0] || !hPsp[1] || !hPsp[2] || !hPsp[3]) return;

	PROPSHEETHEADER psh;
	psh.dwSize = PROPSHEETHEADER_V1_SIZE;
	psh.dwFlags =	PSH_DEFAULT;
	psh.hwndParent = hwndMain;
	psh.hInstance = GetModuleHandle(NULL);
	psh.pszCaption = MAKEINTRESOURCE(IDS_SETTINGS);
	psh.nPages = 4;
	psh.nStartPage = 0;
	psh.phpage = hPsp;

	PropertySheet(&psh);
}
