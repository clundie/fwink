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

#ifndef WEBCAM_H
#define WEBCAM_H

#pragma warning(disable : 4995)

class WebCam;

#include <windows.h>
#include <streams.h>
#include <objbase.h>
#include <prsht.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <wininet.h>
#include <commctrl.h>
#include <ras.h>
#include <raserror.h>
#include <dshow.h>
#include <qedit.h>
#include <exception>
#include <string>
#include <dvdmedia.h>
#include <commdlg.h>
#include <vector>
#include "resource.h"
#include "dllversion.h"
#include "crossbar.h"
#include "VideoDevice.h"

using std::wstring;
using std::vector;
using namespace Gdiplus;

class WebCam
{
public:
	void CropPreview();

	static const int _expireTime;

	enum
	timeunit_t
	{
		timeunit_seconds = 0,
		timeunit_minutes = 1,
		timeunit_hours   = 2
	};

	enum
	smooth_t
	{
		smooth_none = 0,
		smooth_standard = 1,
		smooth_cleartype = 2
	};

	enum
	ovlscale_t
	{
		ovlscale_original = 0,
		ovlscale_bestfit = 1,
		ovlscale_percentage = 2
	};

	enum
	textposition_t
	{
		textposition_upperleft = 0,
		textposition_top = 1,
		textposition_upperright = 2,
		textposition_left = 3,
		textposition_centre = 4,
		textposition_right = 5,
		textposition_lowerleft = 6,
		textposition_bottom = 7,
		textposition_lowerright = 8
	};

	enum textbackground_t
	{
		textbackground_solid = 0,
		textbackground_translucent = 1,
		textbackground_none = 2
	};

	WebCam();
	~WebCam();

	void Start(void);
	void StartOneShot(void);

	void HandleCaptureMessage(WebCam* cam, HWND hwndDlg);
	static INT_PTR CALLBACK DialogProcMain(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DialogProcVideo(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DialogProcFTP(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DialogProcEffects(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DialogProcOverlay(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK DialogProcChooseVideoDevice(HWND, UINT, WPARAM, LPARAM);
	void EnumerateVideoDevicesForComboBox(HWND hwndComboBox);
	void ShowChooseVideoDeviceDialog(HWND hwndParent);
	void ShowMainDialog(void);
	void CreateNotificationIcon(void);
	void CreateMainDialog(void);
	void ChangeVideoFormat(const HWND, const bool);
	void ChangeVidcapProperties(const HWND, const bool);
	void CaptureFrame(void);
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	void SetStatus(int);
	static void CALLBACK InternetStatusCallback(HINTERNET, DWORD_PTR, DWORD, LPVOID, DWORD);
	void SaveSettings(void);
	void SaveEffectsSettings(void);
	void SaveVideoSettings(void);
	void SaveFTPSettings(void);
	void SaveOverlaySettings(void);
	void StartTimer(void);
	void StopTimer(void);
	bool ConnectedToInternet(void);
	HRESULT GetPinCategory(IPin* pPin, GUID* pPinCategory);
	IPin* GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, const GUID& pPinCategory);
	IPin* GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, unsigned iIndex);
	void UploadImage(void);
	void InitDirectShow(void);
	void ShutdownDirectShow(void);
	void AddText(Image*);
	void DeleteTempFile(void);
	void ApplyImageSize(void);
	void ShowSettingsDialog(void);
	void NukeDownstream(IBaseFilter*);
	void UnNuke(void);
	void StartCapture(void);
	void StopCapture(void);
	void GetCaptureSize(void);
	void SetCaptureSize(void);
	void TVTunerProperties(const HWND, const bool);
	void InitFTPControls(HWND hwndDlg, BOOL bEnable);
	void InitTimerControls(HWND hwndDlg, BOOL bEnable);
	void InitFileControls(HWND hwndDlg, BOOL bEnable);
	void AddOverlay(
		Image* imBase, Image* imOverlay = NULL,
		bool bLowQuality = false,
		unsigned uOpacity = 100,
		unsigned uScale = ovlscale_original,
		float fScaleX = 100.0f, float fScaleY = 100.0f,
		bool bColourKey = false,
		unsigned uKeyR = 255, unsigned uKeyG = 255, unsigned uKeyB = 255,
		unsigned pos = textposition_lowerleft
		);

	static bool LoadSetting(const WCHAR* name, wstring& value);
	static bool LoadSetting(const WCHAR* name, unsigned* pValue);
	static bool LoadSetting(const WCHAR* name, int* pValue);
	static bool LoadSetting(const WCHAR* name, bool* pValue);

	static bool SaveSetting(const WCHAR* name, const TCHAR* value);
	static bool SaveSetting(const WCHAR* name, const unsigned value);
	static bool SaveSetting(const WCHAR* name, const int value);
	static bool SaveSetting(const WCHAR* name, const bool value);

	HWND hwndMain;
	HICON hIconIdle;
	static const UINT uNotifyIconMessage;
	static const UINT uNotifyIconID;
	static const UINT uTaskbarRestart;
	ULONG_PTR uGDIPlusToken;
	unsigned uImageWidth, uImageHeight;
	HINTERNET hInternet;
	HINTERNET hFTP;
	int iStatus;
	WCHAR szwFilenameJPEG[MAX_PATH];
	bool bTempFile;
	bool bReadyToUpload;
	static const wstring sRegKey;
	wstring sServer,sUsername,sPassword,sDirectory,sFilename,sWisdom;
	unsigned uTimeValue, uTimeUnit;
	bool bDialup;
	bool bTimestamp, bDatestamp;
	bool bPassiveFTP;
	static const unsigned uMaxTimeValue, uMinTimeValue;
	static const unsigned uMinTimeUnit, uMaxTimeUnit;
	const UINT_PTR uTimerID;
	bool bBalloonTips;
	bool bRunAtStartup;
	static const UINT uCaptureMessage;
	static const UINT uRemoteCaptureMessage;
	static void FreeMediaType(AM_MEDIA_TYPE&);
	bool bOKToDeleteTempFile;
	unsigned uImageSize;
	static const unsigned uMaxImageSize, uMinImageSize;
	bool bCustomImageSize;
	unsigned uCustomSizeX, uCustomSizeY;
	TCHAR szAppname[512];
	static const unsigned uMaxFontSize, uMinFontSize;
	unsigned uFontSize;
	wstring sFontname;
	unsigned uSmoothType;
	static const unsigned uMinSmooth, uMaxSmooth;
	bool bClearTypeAvailable;
	bool bFontBold, bFontItalic, bFontUnderline, bFontStrikeout;
	bool bFlipHorizontal, bFlipVertical;
	unsigned uCrossbarInput;
	int iCaptureWidth, iCaptureHeight;
	HBRUSH hbrSampleBackground;
	COLORREF rgbForeground, rgbBackground;
	unsigned uTextPosition;
	bool bDropShadow;
	unsigned uTextBackground;
	unsigned uJPEGQuality;
	bool bOneShot;
	static const UINT uQuitMessage;
	static const TCHAR szHomePageURL[];
	bool bDisableFTP;
	bool bDisableTimer;
	bool bSaveToFile;
	wstring sLocalFilename;
	wstring sOverlayImageFile;
	unsigned uOverlayScale;
	unsigned uOverlayScalePercent;
	unsigned uOverlayPosition;
	unsigned uOverlayOpacity;
	bool bOverlayColourKey;
	COLORREF rgbColourKey;
	bool bCrop;
	unsigned uCropWidth, uCropHeight,
		uCropX, uCropY;

	wstring sDeviceName;
	wstring sDevicePath;

	vector<VideoDevice> videoDevices;

	// DirectShow stuff
	//
	IGraphBuilder* pGraph;
	ICaptureGraphBuilder2* pBuilder;
	IBaseFilter* pVidcap;
	IBaseFilter* pSampleGrabber;
	IBaseFilter* pNullRenderer;
};

#endif // WEBCAM_H
