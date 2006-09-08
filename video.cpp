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
#include <ctime>
#include <comdef.h>

// Open TV tuner properties
//
void
WebCam::TVTunerProperties(const HWND hwndParent, const bool bShowErrors)
{
	if (!pBuilder) return;

	HRESULT hr;

	IAMTVTuner* pTuner = NULL;

	hr = pBuilder->FindInterface(
		&LOOK_UPSTREAM_ONLY,
		NULL,
		pVidcap,
		IID_IAMTVTuner,
		(void**)&pTuner);
	if (FAILED(hr))
	{
		if (bShowErrors)
		{
			TCHAR buffer[512];
			LoadString(GetModuleHandle(NULL), ERR_TVTUNERPROPERTIES, buffer, 512);
			MessageBox(hwndParent, buffer, szAppname, MB_ICONEXCLAMATION);
		}
		return;
	}

	ISpecifyPropertyPages* pPropTuner = NULL;
	pTuner->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pPropTuner);
	if (!pPropTuner)
	{
		pTuner->Release();
		if (bShowErrors)
		{
			TCHAR buffer[512];
			LoadString(GetModuleHandle(NULL), ERR_TVTUNERPROPERTIES, buffer, 512);
			MessageBox(hwndParent, buffer, szAppname, MB_ICONEXCLAMATION);
		}
		return;
	}

	IUnknown* pFilterUnk;
	CAUUID caGUID;

	pTuner->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);
	pPropTuner->GetPages(&caGUID);
	pPropTuner->Release();
	pPropTuner = NULL;

	// Show the page. 

	OleCreatePropertyFrame(
			hwndParent, 									// Parent window
			0, 0, 									// (Reserved)
			OLESTR("TV Tuner"), 		// Caption for the dialog box
			1,											// Number of objects (just the filter)
			&pFilterUnk,						// Array of object pointers. 
			caGUID.cElems,					// Number of property pages
			caGUID.pElems,					// Array of property page CLSIDs
			0,											// Locale identifier
			0, NULL 								// Reserved
	);

	pFilterUnk->Release();
	CoTaskMemFree(caGUID.pElems);

	pTuner->Release();
}

// Open video format properties
//
void
WebCam::ChangeVideoFormat(const HWND hwndParent, const bool bShowErrors)
{
	HRESULT hr;

	IMediaControl* pMediaControl = NULL;
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)(&pMediaControl));
	if (SUCCEEDED(hr))
	{
		pMediaControl->Stop();
		pMediaControl->Release();
	}

	NukeDownstream(pVidcap);

	IPin* pinVidcapOut = GetPin(pVidcap, PINDIR_OUTPUT, PIN_CATEGORY_CAPTURE);
	if (!pinVidcapOut)
	{
		if (bShowErrors)
		{
			TCHAR buffer[512];
			LoadString(GetModuleHandle(NULL), ERR_MODIFYVIDEOFORMAT, buffer, 512);
			MessageBox(hwndParent, buffer, szAppname, MB_ICONEXCLAMATION);
		}
		return;
	}

	ISpecifyPropertyPages* pPropVidcapOut = NULL;
	pinVidcapOut->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pPropVidcapOut);
	if (!pPropVidcapOut)
	{
		pinVidcapOut->Release();
		if (bShowErrors)
		{
			TCHAR buffer[512];
			LoadString(GetModuleHandle(NULL), ERR_MODIFYVIDEOFORMAT, buffer, 512);
			MessageBox(hwndParent, buffer, szAppname, MB_ICONEXCLAMATION);
		}
		return;
	}

	IUnknown* pFilterUnk;
	CAUUID caGUID;

	pinVidcapOut->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);
	pPropVidcapOut->GetPages(&caGUID);
	pPropVidcapOut->Release();
	pPropVidcapOut = NULL;

	// Show the page. 

	OleCreatePropertyFrame(
			hwndParent, 									// Parent window
			0, 0, 									// (Reserved)
			OLESTR("Video Format"), 		// Caption for the dialog box
			1,											// Number of objects (just the filter)
			&pFilterUnk,						// Array of object pointers. 
			caGUID.cElems,					// Number of property pages
			caGUID.pElems,					// Array of property page CLSIDs
			0,											// Locale identifier
			0, NULL 								// Reserved
	);

	pFilterUnk->Release();
	CoTaskMemFree(caGUID.pElems);
	pinVidcapOut->Release();

	UnNuke();

}

// Open video capture properties
//
void
WebCam::ChangeVidcapProperties(const HWND hwndParent, const bool bShowErrors)
{
	if (uDeviceType != deviceType_camera)
	{
		return;
	}
	ISpecifyPropertyPages* pPropVidcap = NULL;
	pVidcap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pPropVidcap);
	if (!pPropVidcap)
	{
		if (bShowErrors)
		{
			TCHAR buffer[512];
			LoadString(GetModuleHandle(NULL), ERR_MODIFYVIDCAP, buffer, 512);
			MessageBox(hwndParent, buffer, szAppname, MB_ICONEXCLAMATION);
		}
		return;
	}

	IUnknown* pFilterUnk;
	CAUUID caGUID;

	pVidcap->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);
	pPropVidcap->GetPages(&caGUID);
	pPropVidcap->Release();
	pPropVidcap = NULL;

	// Show the page. 

	OleCreatePropertyFrame(
			hwndParent, 									// Parent window
			0, 0, 									// (Reserved)
			OLESTR("Image"), 		// Caption for the dialog box
			1,											// Number of objects (just the filter)
			&pFilterUnk,						// Array of object pointers. 
			caGUID.cElems,					// Number of property pages
			caGUID.pElems,					// Array of property page CLSIDs
			0,											// Locale identifier
			0, NULL 								// Reserved
	);

	pFilterUnk->Release();
	CoTaskMemFree(caGUID.pElems);
}

void
WebCam::UploadImage(void)
{
	if (hFTP)
	{
		InternetCloseHandle(hFTP);
		hFTP = NULL;
	}
	if (bTempFile)
	{
		DeleteFile(szwFilenameJPEG);
		bTempFile = false;
	}

	HRESULT hr;

	// Stream interface for bitmap
	//
	IStream* pBitmapStream = NULL;

	if (uDeviceType == deviceType_camera)
	{
		// Get sample grabber interface
		//
		ISampleGrabber *pGrabber = NULL;

		hr = pSampleGrabber->QueryInterface(IID_ISampleGrabber,
		  reinterpret_cast<void**>(&pGrabber));
		if (FAILED(hr))
		{
			return;
		}

		try
		{
			// Get size of captured frame
			//
			long lBufferSize = 0;
			hr = pGrabber->GetCurrentBuffer(&lBufferSize, NULL);
			if (FAILED(hr))
			{
				throw 0;
			}

			// Allocate memory for captured frame
			//
			void* pBuffer = new char[lBufferSize];

			try
			{

				// Get captured frame
				//
				hr = pGrabber->GetCurrentBuffer(&lBufferSize, (long*)pBuffer);
				if (FAILED(hr))
				{
					throw 0;
				}

				// Get media type

				AM_MEDIA_TYPE MediaType; 
				ZeroMemory(&MediaType, sizeof(MediaType)); 
				hr = pGrabber->GetConnectedMediaType(&MediaType); 
				if (FAILED(hr)) throw 0;

				try
				{
					// Get a pointer to the video header

					VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)MediaType.pbFormat;
					if (pVideoHeader == NULL) throw 0;

					// Create a bitmap file header

					BITMAPFILEHEADER BmpFileHeader;
					BmpFileHeader.bfType = MAKEWORD('B','M');
					BmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
						+ pVideoHeader->bmiHeader.biSizeImage;
					BmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
					BmpFileHeader.bfReserved1 = 0;
					BmpFileHeader.bfReserved2 = 0;

					// Create stream
					hr = CreateStreamOnHGlobal(NULL, TRUE, &pBitmapStream);
					if (FAILED(hr)) throw 0;

					ULARGE_INTEGER ularge;
					ularge.QuadPart = BmpFileHeader.bfSize;
					pBitmapStream->SetSize(ularge);
					LARGE_INTEGER ilarge;
					ilarge.QuadPart = 0;
					pBitmapStream->Seek(ilarge, STREAM_SEEK_SET, NULL);
					pBitmapStream->Write(&BmpFileHeader, sizeof(BITMAPFILEHEADER), NULL);
					pBitmapStream->Write(&(pVideoHeader->bmiHeader), sizeof(BITMAPINFOHEADER), NULL);
					pBitmapStream->Write(pBuffer, lBufferSize, NULL);
				}
				catch(int)
				{
				}
				FreeMediaType(MediaType);

			}
			catch(int)
			{
			}
			delete[] pBuffer;
		}
		catch (int)
		{
		}
		pGrabber->Release();
		pGrabber = NULL;

		if (!pBitmapStream) return;
	}
	else if (uDeviceType == deviceType_file)
	{

	}
	else
	{
		assert(false);
	}

	TCHAR* szPathname = new TCHAR[MAX_PATH];

	try
	{
		// Get temporary path name
		//
		DWORD dwPathSize = GetTempPath(MAX_PATH, szPathname);
		if (dwPathSize == 0)
		{
			SetStatus(ERR_CAPTURE);
			throw 0;
		}

		if (dwPathSize > MAX_PATH)
		{
			delete[] szPathname;
			szPathname = new TCHAR[dwPathSize];
			DWORD dwNewPathSize = GetTempPath(dwPathSize, szPathname);
			if ((dwNewPathSize == 0) || (dwNewPathSize > dwPathSize))
			{
				SetStatus(ERR_CAPTURE);
				throw 0;
			}
		}

		// Create Image object
		//
		Image* image = NULL;

		if (uDeviceType == deviceType_camera)
		{
			image = Bitmap::FromStream(pBitmapStream);
			pBitmapStream->Release();
			pBitmapStream = NULL;
		}
		else if (uDeviceType == deviceType_file)
		{
			if (imageFile.get())
			{
				image = imageFile->Clone();
			}
		}

		if (image && (Ok == image->GetLastStatus()))
		{
			Bitmap* image2 = new Bitmap(uImageWidth, uImageHeight, PixelFormat24bppRGB);
			if (image2)
			{
				Graphics* g2 = Graphics::FromImage(image2);
				if (g2)
				{
					g2->SetInterpolationMode(InterpolationModeHighQualityBicubic);
					if (!bCrop)
					{
						g2->DrawImage(image, 0, 0, uImageWidth, uImageHeight);
					}
					else
					{
						g2->DrawImage(
							image,
							RectF(0., 0., (REAL)uImageWidth, (REAL)uImageHeight),
							(REAL)uCropX, (REAL)uCropY,
							(REAL)uCropWidth, (REAL)uCropHeight,
							UnitPixel,
							NULL,
							NULL,
							NULL
							);
					}
					delete g2;
					g2 = NULL;
				}
				delete image;
				image = image2;
				image2 = NULL;
			}

			// Flip image
			if (bFlipHorizontal)
			{
				image->RotateFlip(RotateNoneFlipX);
			}
			if (bFlipVertical)
			{
				image->RotateFlip(RotateNoneFlipY);
			}

			// Add overlay to image
			AddOverlay(image, NULL, false, uOverlayOpacity,
				uOverlayScale,
				(float)uOverlayScalePercent, (float)uOverlayScalePercent,
				bOverlayColourKey,
				GetRValue(rgbColourKey), GetGValue(rgbColourKey), GetBValue(rgbColourKey),
				uOverlayPosition
				);

			// Add text to image
			AddText(image);

			IStream* pBMPStream = NULL;
			CreateStreamOnHGlobal(NULL, TRUE, &pBMPStream);
			if (pBMPStream)
			{
				CLSID idBMPEncoder;
				GetEncoderClsid(L"image/bmp", &idBMPEncoder);
				image->Save(pBMPStream, &idBMPEncoder);
				delete image;
				image = Bitmap::FromStream(pBMPStream);
				pBMPStream->Release();
				pBMPStream = NULL;
			}

			// Create a temporary file for the JPEG image.

			UINT uUnique = GetTempFileName(szPathname, TEXT("cam"), 0, szwFilenameJPEG);
			if (uUnique)
			{
				bTempFile = true;

				// Get the class id of the JPEG encoder
				//
				CLSID idEncoder;
				int iGotEncoder = GetEncoderClsid(L"image/jpeg", &idEncoder);

				// Set JPEG compression quality
				//
				ULONG quality = uJPEGQuality;
				EncoderParameters p;
				p.Count = 1;
				p.Parameter[0].Guid = EncoderQuality;
				p.Parameter[0].Type = EncoderParameterValueTypeLong;
				p.Parameter[0].NumberOfValues = 1;
				p.Parameter[0].Value = &quality;

				// Save the JPEG image
				//
				if (Ok == image->Save(szwFilenameJPEG, &idEncoder, &p))
				{
					// Save local file.
					//
					if (bSaveToFile)
					{
						CopyFile(szwFilenameJPEG, sLocalFilename.c_str(), FALSE);
						if (bOneShot && bDisableFTP)
						{
							SendMessage(hwndMain, uQuitMessage, 0, 0);
						}
					}

					// Start FTP session
					//
					if (!bDisableFTP)
					{
						if (bReadyToUpload && ConnectedToInternet()
							&& (sServer.length() > 1) && (sServer[0] != 0) )
						{
							DWORD dwFlags = 0;
							if (bPassiveFTP) dwFlags |= INTERNET_FLAG_PASSIVE;

							SetStatus(STATUS_CONNECTING);
							InternetConnect(
								hInternet,
								sServer.c_str(),
								INTERNET_DEFAULT_FTP_PORT,
								sUsername.c_str(),
								sPassword.c_str(),
								INTERNET_SERVICE_FTP,
								dwFlags,
								reinterpret_cast<DWORD_PTR>(this));
						}
						else if (bOneShot)
						{
							SendMessage(hwndMain, uQuitMessage, 0, 0);
						}
					}

				}
				else
				{
					// Image not saved! Uh oh.
					SetStatus(ERR_CAPTURE);
				}
			}
		}
		if (image)
			delete image;
		image = NULL;
	}
	catch(int)
	{
	}

	delete[] szPathname;
	szPathname = NULL;
}

void
WebCam::CaptureFrame(void)
{
	if (uDeviceType == deviceType_file)
	{
		InitImageFile();

		if (bDisableFTP && !bSaveToFile && bOneShot)
		{
			SendMessage(hwndMain, uQuitMessage, 0, 0);
			return;
		}

		UploadImage();
	}
	else if (uDeviceType == deviceType_camera)
	{
		
		
	/*
		if (time(NULL) > _expireTime)
		{
			throw ERR_EXPIRED;
		}
	*/
		if ((bDisableFTP && !bSaveToFile) || !pSampleGrabber)
		{
			if (bOneShot) SendMessage(hwndMain, uQuitMessage, 0, 0);
			return;
		}

		HRESULT hr;

		// Get sample grabber interface
		//
		ISampleGrabber *pGrabber = NULL;

		hr = pSampleGrabber->QueryInterface(IID_ISampleGrabber,
		  reinterpret_cast<void**>(&pGrabber));
		if (FAILED(hr))
		{
			if (bOneShot) SendMessage(hwndMain, uQuitMessage, 0, 0);
			return;
		}

		IMediaEventEx* pEvent = NULL;
		hr = pGraph->QueryInterface(IID_IMediaEventEx, (void**)&pEvent);
		if (FAILED(hr))
		{
			pGrabber->Release();
			pGrabber = NULL;
			if (bOneShot) SendMessage(hwndMain, uQuitMessage, 0, 0);
			return;
		}

		// Start buffering samples
		//

		pEvent->CancelDefaultHandling(EC_COMPLETE);
		pEvent->SetNotifyFlags(0);

		pGrabber->SetOneShot(TRUE);
		pGrabber->SetBufferSamples(TRUE);

		StartCapture();

		pEvent->Release();
		pEvent = NULL;

		pGrabber->Release();
		pGrabber = NULL;
	}
}

void
WebCam::StopCapture(void)
{
	if (!pGraph) return;
	IMediaControl* pMediaControl = NULL;
	if (SUCCEEDED(pGraph->QueryInterface(IID_IMediaControl, (void**)(&pMediaControl))))
	{
		pMediaControl->Stop();
		pMediaControl->Release();
	}
}

void
WebCam::StartCapture(void)
{
	if (!pGraph) return;
	IMediaControl* pMediaControl = NULL;
	if (SUCCEEDED(pGraph->QueryInterface(IID_IMediaControl, (void**)(&pMediaControl))))
	{
		pMediaControl->Stop();
		pMediaControl->Run();
		pMediaControl->Release();
	}
}

int
WebCam::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
 UINT num = 0; 				 // number of image encoders
 UINT size = 0;				 // size of the image encoder array in bytes

 ImageCodecInfo* pImageCodecInfo = NULL;

 GetImageEncodersSize(&num, &size);
 if(size == 0)
		return -1;	// Failure

 pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
 if(pImageCodecInfo == NULL)
		return -1;	// Failure

 GetImageEncoders(num, size, pImageCodecInfo);

 for(UINT j = 0; j < num; ++j)
 {
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			 *pClsid = pImageCodecInfo[j].Clsid;
			 free(pImageCodecInfo);
			 return j;	// Success
		} 	 
 }

 free(pImageCodecInfo);
 return -1;  // Failure
}

HRESULT
WebCam::GetPinCategory(IPin* pPin, GUID* pPinCategory)
{
	IKsPropertySet *pKs;
	HRESULT hr = pPin->QueryInterface(IID_IKsPropertySet, (void **)&pKs);
	if (SUCCEEDED(hr))
	{
		DWORD cbReturned;
		hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, 
			pPinCategory, sizeof(GUID), &cbReturned);
		pKs->Release();
		if (SUCCEEDED(hr))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

IPin*
WebCam::GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, unsigned iIndex)
{
		BOOL			 bFound = FALSE;
		IEnumPins  *pEnum;
		IPin			 *pPin;

		unsigned i = 0;

		pFilter->EnumPins(&pEnum);
		while(pEnum->Next(1, &pPin, 0) == S_OK)
		{
				PIN_DIRECTION PinDirThis;
				pPin->QueryDirection(&PinDirThis);

				if (PinDir == PinDirThis)
				{
					if (i == iIndex)
					{
						bFound = TRUE;
						break;
					}
					++i;
				}

				pPin->Release();
		}
		pEnum->Release();
		return (bFound ? pPin : 0);  
}

IPin*
WebCam::GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, const GUID& PinCategory)
{
		BOOL			 bFound = FALSE;
		IEnumPins  *pEnum;
		IPin			 *pPin;

		pFilter->EnumPins(&pEnum);
		while(pEnum->Next(1, &pPin, 0) == S_OK)
		{
				PIN_DIRECTION PinDirThis;
				pPin->QueryDirection(&PinDirThis);

				if (PinDir == PinDirThis)
				{
					GUID ThisCategory;
					HRESULT hr = GetPinCategory(pPin, &ThisCategory);
					if ((SUCCEEDED(hr)) && (IsEqualGUID(ThisCategory, PinCategory)))
					{
						bFound = TRUE;
						break;
					}
				}

				pPin->Release();
		}
		pEnum->Release();
		return (bFound ? pPin : 0);  
}

void WebCam::InitImageFile(void)
{
	std::auto_ptr<Image> i(Image::FromFile(sDevicePath.c_str()));
	if (i->GetLastStatus() != Ok)
	{
		return;
	}

	std::auto_ptr<Bitmap> b(new Bitmap(i->GetWidth(), i->GetHeight()));
	std::auto_ptr<Graphics> g(new Graphics(b.get()));
	g->DrawImage(i.get(), 0, 0, i->GetWidth(), i->GetHeight());
	g.reset();
	i.reset();

	imageFile.reset(b.get());
	b.release();

	RECT rVideoWindow;
	GetClientRect(GetDlgItem(hwndMain, IDC_PREVIEW), &rVideoWindow);
	LONG windowWidth = rVideoWindow.right - rVideoWindow.left;
	LONG windowHeight = rVideoWindow.bottom - rVideoWindow.top;

	float aspectRatio = (float)imageFile->GetWidth() / (float)imageFile->GetHeight();
	long destWidth, destHeight;
	if (aspectRatio * windowHeight > windowWidth)
	{
		destWidth = windowWidth;
		destHeight = (long)((float)destWidth / aspectRatio);
	}
	else
	{
		destHeight = windowHeight;
		destWidth = (long)((float)destHeight * aspectRatio);
	}

	imageFilePreview.reset(new Bitmap(destWidth, destHeight));

	g.reset(new Graphics(imageFilePreview.get()));
	g->DrawImage(imageFile.get(), 0, 0, destWidth, destHeight);
	g.reset();

	HBITMAP hbm;
	if (Ok != imageFilePreview->GetHBITMAP(Color::Color(0, 255, 255, 255), &hbm))
	{
		return;
	}
	HBITMAP oldBitmap = (HBITMAP)SendDlgItemMessage(hwndMain, IDC_PREVIEW, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)hbm);
	if (oldBitmap)
	{
		DeleteBitmap(oldBitmap);
	}
}

void
WebCam::InitDirectShow(void)
{
	HRESULT hr;

	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
		throw ERR_CREATEGRAPHBUILDER;

	// Create the capture graph builder
	//
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **)&pBuilder);
	if (FAILED(hr))
		throw ERR_CREATECAPTUREGRAPHBUILDER;

	pBuilder->SetFiltergraph(pGraph);

	// Create the system device enumerator.

	ICreateDevEnum *pDevEnum = NULL;
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, 
		IID_ICreateDevEnum, (void **)&pDevEnum);

	if (!pDevEnum)
		throw ERR_NOVIDEOCAPTURE;

	// Create an enumerator for video capture devices.
	IEnumMoniker *pClassEnum = NULL;
	pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);

	if (!pClassEnum)
	{
		pDevEnum->Release();
		throw ERR_NOVIDEOCAPTURE;
	}

	ULONG cFetched;
	IMoniker *pMoniker = NULL;
	wstring newDevicePath, newDeviceName;

	while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)
	{
		wstring friendlyName, devicePath;
		// Get the property bag of the moniker
		{
			IPropertyBagPtr propBag;
			hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag,
					reinterpret_cast<void**>(&propBag));
			if (FAILED(hr))
			{
				pMoniker->Release();
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
				friendlyName = v.bstrVal;
			}
			hr = propBag->Read(L"DevicePath", &v, 0);
			if (S_OK == hr)
			{
				devicePath = v.bstrVal;
			}
		}

		// Bind the first moniker to a filter object.
		if (pVidcap == NULL)
		{
			pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVidcap);
			newDeviceName = friendlyName;
			newDevicePath = devicePath;
		}
		else if (
			(devicePath == this->sDevicePath)
			||
			(this->sDevicePath.empty() && !devicePath.compare(0, 8, L"\\\\?\\usb#"))
			)
		{
			if (pVidcap)
			{
				pVidcap->Release();
				pVidcap = NULL;
			}
			pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVidcap);
			newDeviceName = friendlyName;
			newDevicePath = devicePath;
		}
		pMoniker->Release();
	}
	pClassEnum->Release();
	pDevEnum->Release();

	this->sDeviceName = newDeviceName;
	this->sDevicePath = newDevicePath;
	SaveSetting(L"devicePath", newDevicePath.c_str());

	if (!pVidcap)
		throw ERR_NOVIDEOCAPTURE;

	// add video capture to graph
	//
	hr = pGraph->AddFilter(pVidcap, L"vidcap");
	if (FAILED(hr))
		throw ERR_ADDVIDEOCAPTURE;

	// Set video capture size
	SetCaptureSize();

	// create sample grabber
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC,
		IID_IBaseFilter, (void **)&pSampleGrabber);
	if (FAILED(hr))
		throw ERR_CREATEGRABBER;

	// add sample grabber to graph
	//
	hr = pGraph->AddFilter(pSampleGrabber, L"sample grabber");
	if (FAILED(hr))
		throw ERR_ADDGRABBER;

	// set media format of sample grabber

	ISampleGrabber *pGrabber = NULL;
	hr = pSampleGrabber->QueryInterface(IID_ISampleGrabber,
    reinterpret_cast<void**>(&pGrabber));
	if (FAILED(hr))
		throw ERR_QUERYGRABBER;

	try
	{
		AM_MEDIA_TYPE mt;
		ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
		mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
		hr = pGrabber->SetMediaType(&mt);
		if (FAILED(hr))
			throw ERR_SETMEDIATYPE;
	}
	catch(...)
	{
		pGrabber->Release();
		throw;
	}
	pGrabber->Release();

	// create null renderer
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC,
		IID_IBaseFilter, (void **)&pNullRenderer);
	if (FAILED(hr))
		throw ERR_CREATENULLRENDERER;

	// add null renderer to graph
	//
	hr = pGraph->AddFilter(pNullRenderer, L"null renderer");
	if (FAILED(hr))
		throw ERR_ADDNULLRENDERER;

	// send video preview to renderer
	//
	hr = pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
		&MEDIATYPE_Video,
		pVidcap,
		NULL,
		NULL);

	// send video capture to null renderer
	//
	hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		pVidcap,
		NULL,
		pNullRenderer);

	// insert sample grabber in front of null renderer
	//

	// get null renderer input pin
	//
	IPin* pinNullIn = GetPin(pNullRenderer, PINDIR_INPUT, 0);
	if (pinNullIn)
	{
		// get pin that null renderer input pin is connected to
		IPin* pinBeforeNull;
		hr = pinNullIn->ConnectedTo(&pinBeforeNull);
		if (SUCCEEDED(hr) && pinBeforeNull)
		{

			IPin* pinGrabberIn = GetPin(pSampleGrabber, PINDIR_INPUT, 0);
			IPin* pinGrabberOut = GetPin(pSampleGrabber, PINDIR_OUTPUT, 0);

			if (pinGrabberIn && pinGrabberOut)
			{

				pGraph->Disconnect(pinBeforeNull);
				pGraph->Disconnect(pinNullIn);

				pGraph->Connect(pinBeforeNull, pinGrabberIn);
				pGraph->Connect(pinGrabberOut, pinNullIn);
			}

			if (pinGrabberIn)
				pinGrabberIn->Release();

			if (pinGrabberOut)
				pinGrabberOut->Release();

			pinBeforeNull->Release();
		}
		pinNullIn->Release();
	}

	// Set up crossbar
	//
	IPin* pinVidcapIn = GetPin(pVidcap, PINDIR_INPUT, PIN_CATEGORY_ANALOGVIDEOIN);
	if (pinVidcapIn)
	{
		HRESULT hr;
		CCrossbar* pCCrossbar = new CCrossbar(pinVidcapIn, &hr);
		if (pCCrossbar)
		{
			pCCrossbar->SetInputIndex(uCrossbarInput);
			delete pCCrossbar;
		}
		pinVidcapIn->Release();
	}

	// Set the HWND for event notification
	//
	IMediaEventEx* pEvent = NULL;
	hr = pGraph->QueryInterface(IID_IMediaEventEx, (void**)&pEvent);
	if (SUCCEEDED(hr))
	{
		pEvent->SetNotifyWindow((OAHWND)hwndMain, uCaptureMessage, 0);
		pEvent->Release();
	}

	// set up video window
	//
	IVideoWindow* pVideoWindow = NULL;

	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pVideoWindow);
	if (FAILED(hr))
		throw ERR_NOVIDEOCAPTURE;

	pVideoWindow->put_Owner((OAHWND)GetDlgItem(hwndMain, IDC_PREVIEW));

	long windowStyleVideo = WS_CHILD;
	pVideoWindow->put_WindowStyle(windowStyleVideo);

	RECT rVideoWindow;
	GetClientRect(GetDlgItem(hwndMain, IDC_PREVIEW), &rVideoWindow);

	pVideoWindow->put_Top(0);
	pVideoWindow->put_Left(0);
	pVideoWindow->put_Width(rVideoWindow.right - rVideoWindow.left);
	pVideoWindow->put_Height(rVideoWindow.bottom - rVideoWindow.top);
	pVideoWindow->put_AutoShow(OAFALSE);
	pVideoWindow->put_Visible(OATRUE);
	pVideoWindow->Release();

	// run the graph
	//
	if (IsWindowVisible(hwndMain))
	{
		IMediaControl* pMediaControl = NULL;
		hr = pGraph->QueryInterface(IID_IMediaControl, (void**)(&pMediaControl));
		if (FAILED(hr))
			throw ERR_NOVIDEOCAPTURE;
		pMediaControl->Run();
		pMediaControl->Release();
	}

	GetCaptureSize();
	CropPreview();

}

void
WebCam::ShutdownDirectShow(void)
{
	if (pNullRenderer)
		pNullRenderer->Release();
	pNullRenderer = NULL;

	if (pSampleGrabber)
		pSampleGrabber->Release();
	pSampleGrabber = NULL;

	if (pVidcap)
		pVidcap->Release();
	pVidcap = NULL;

	if (pGraph)
		pGraph->Release();
	pGraph = NULL;

	if (pBuilder)
		pBuilder->Release();
	pBuilder = NULL;
}

void
WebCam::FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}

	if (mt.pUnk != NULL)
	{
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

// Add text and time stamp to an image.
//
void
WebCam::AddText(Image* pImage)
{
	Graphics g(pImage);

	wstring sText(L"");

	if ((sWisdom.length() > 0) && (sWisdom[0] != 0))
	{
		sText.append(sWisdom.c_str());
	}

	// Get the date and time
	//
	if (bTimestamp || bDatestamp)
	{
		if (sText.length() > 0)
		{
			sText.append(L"\n");
		}

		TCHAR buffer[512];

		if (bDatestamp)
		{
			GetDateFormat(LOCALE_USER_DEFAULT,
				DATE_LONGDATE,
				NULL,
				NULL,
				buffer, 512);
			sText.append(buffer);
		}

		if (bDatestamp && bTimestamp)
		{
			sText.append(L" @ ");
		}

		if (bTimestamp)
		{
			GetTimeFormat(LOCALE_USER_DEFAULT,
				0,
				NULL,
				NULL,
				buffer, 512);
			sText.append(buffer);
		}
	}

	if (sText.length() < 1) return;

	// Set font and anti-alias options
	//
	FontFamily fontFamily(sFontname.c_str());
	FontStyle style = FontStyleRegular;
	if (bFontBold) style = static_cast<FontStyle>(style | static_cast<int>(FontStyleBold));
	if (bFontItalic) style = static_cast<FontStyle>(style | static_cast<int>(FontStyleItalic));
	if (bFontUnderline) style = static_cast<FontStyle>(style | static_cast<int>(FontStyleUnderline));
	if (bFontStrikeout) style = static_cast<FontStyle>(style | static_cast<int>(FontStyleStrikeout));
	Font font(&fontFamily, (REAL)uFontSize, style, UnitPixel);

	// Set antialias
	
	TextRenderingHint hint;
	switch(uSmoothType)
	{
		case smooth_none:
			hint = TextRenderingHintSingleBitPerPixelGridFit;
			break;
		case smooth_cleartype:
			hint = TextRenderingHintClearTypeGridFit;
			break;
		case smooth_standard:
		default:
			hint = TextRenderingHintAntiAliasGridFit;
	}

	g.SetTextRenderingHint(hint);

	// Set horizontal text position
	//
	StringFormat format;
	switch (uTextPosition)
	{
		case textposition_upperleft:
		case textposition_left:
		case textposition_lowerleft:
			format.SetAlignment(StringAlignmentNear);
			break;

		case textposition_top:
		case textposition_bottom:
		case textposition_centre:
			format.SetAlignment(StringAlignmentCenter);
			break;

		case textposition_upperright:
		case textposition_right:
		case textposition_lowerright:
			format.SetAlignment(StringAlignmentFar);
			break;
	}

	// Set vertical text position
	//
	RectF rWholeImage(0., 0., (REAL)uImageWidth, (REAL)uImageHeight);
	RectF rText(0., 0., (REAL)uImageWidth, (REAL)uImageHeight);
	g.MeasureString(sText.c_str(), -1, &font, rWholeImage, &format, &rText, NULL, NULL);

	switch (uTextPosition)
	{
		case textposition_lowerleft:
		case textposition_bottom:
		case textposition_lowerright:
			rText.Y += (uImageHeight - rText.Height);
			break;

		case textposition_left:
		case textposition_centre:
		case textposition_right:
			rText.Y += (uImageHeight - rText.Height) / 2;
			break;
	}

	// Draw text background
	switch (uTextBackground)
	{
		case textbackground_solid:
		{
			SolidBrush solidBrush(Color(GetRValue(rgbBackground), GetGValue(rgbBackground), GetBValue(rgbBackground)));
			g.FillRectangle(&solidBrush, rText);
			break;
		}

		case textbackground_translucent:
		{
			SolidBrush solidBrush(Color(127, GetRValue(rgbBackground), GetGValue(rgbBackground), GetBValue(rgbBackground)));
			g.FillRectangle(&solidBrush, rText);
			break;
		}
	}

	// Draw drop shadow
	if (bDropShadow)
	{
		SolidBrush brushDropShadow(Color(192, GetRValue(rgbBackground), GetGValue(rgbBackground), GetBValue(rgbBackground)));
		g.DrawString(sText.c_str(), -1, &font, rText, &format, &brushDropShadow);
	}

	// Draw text
	rText.X -= 1.5;
	rText.Y -= 1.5;
	SolidBrush bText(Color(GetRValue(rgbForeground), GetGValue(rgbForeground), GetBValue(rgbForeground)));
	g.DrawString(sText.c_str(), -1, &font, rText, &format, &bText);
}

// Tear down everything downstream of a given filter
void
WebCam::NukeDownstream(IBaseFilter *pf) 
{
	IPin *pP, *pTo;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;
	HRESULT hr = pf->EnumPins(&pins);

	pins->Reset();

	while(hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if(hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if(pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if(hr == NOERROR)
				{
					if(pininfo.dir == PINDIR_INPUT)
					{
						NukeDownstream(pininfo.pFilter);
						pGraph->Disconnect(pTo);
						pGraph->Disconnect(pP);
						pGraph->RemoveFilter(pininfo.pFilter);
					}
					pininfo.pFilter->Release();
				}
				pTo->Release();
			}
			pP->Release();
		}
	}

	if(pins)
		pins->Release();
}

void
WebCam::UnNuke(void)
{
	HRESULT hr;

	// add sample grabber to graph
	//
	hr = pGraph->AddFilter(pSampleGrabber, L"sample grabber");

	// add null renderer to graph
	//
	hr = pGraph->AddFilter(pNullRenderer, L"null renderer");
	if (FAILED(hr))
		throw ERR_ADDNULLRENDERER;

	// send video preview to renderer
	//
	hr = pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
		&MEDIATYPE_Video,
		pVidcap,
		NULL,
		NULL);

	// send video capture to null renderer
	//
	hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		pVidcap,
		NULL,
		pNullRenderer);

	// insert sample grabber in front of null renderer
	//

	// get null renderer input pin
	//
	IPin* pinNullIn = GetPin(pNullRenderer, PINDIR_INPUT, 0);
	if (pinNullIn)
	{
		// get pin that null renderer input pin is connected to
		IPin* pinBeforeNull;
		hr = pinNullIn->ConnectedTo(&pinBeforeNull);
		if (SUCCEEDED(hr) && pinBeforeNull)
		{

			IPin* pinGrabberIn = GetPin(pSampleGrabber, PINDIR_INPUT, 0);
			IPin* pinGrabberOut = GetPin(pSampleGrabber, PINDIR_OUTPUT, 0);

			if (pinGrabberIn && pinGrabberOut)
			{

				pGraph->Disconnect(pinBeforeNull);
				pGraph->Disconnect(pinNullIn);

				pGraph->Connect(pinBeforeNull, pinGrabberIn);
				pGraph->Connect(pinGrabberOut, pinNullIn);
			}

			if (pinGrabberIn)
				pinGrabberIn->Release();

			if (pinGrabberOut)
				pinGrabberOut->Release();

			pinBeforeNull->Release();
		}
		pinNullIn->Release();
	}

	// Set the HWND for event notification
	//
	IMediaEventEx* pEvent = NULL;
	hr = pGraph->QueryInterface(IID_IMediaEventEx, (void**)&pEvent);
	if (SUCCEEDED(hr))
	{
		pEvent->SetNotifyWindow((OAHWND)hwndMain, uCaptureMessage, 0);
		pEvent->Release();
	}

	// set up video window
	//
	IVideoWindow* pVideoWindow = NULL;

	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pVideoWindow);
	if (SUCCEEDED(hr))
	{
		pVideoWindow->put_Owner((OAHWND)GetDlgItem(hwndMain, IDC_PREVIEW));

		long windowStyleVideo = WS_CHILD;
		pVideoWindow->put_WindowStyle(windowStyleVideo);

		RECT rVideoWindow;
		GetClientRect(GetDlgItem(hwndMain, IDC_PREVIEW), &rVideoWindow);

		pVideoWindow->put_Top(0);
		pVideoWindow->put_Left(0);
		pVideoWindow->put_Width(rVideoWindow.right - rVideoWindow.left);
		pVideoWindow->put_Height(rVideoWindow.bottom - rVideoWindow.top);
		pVideoWindow->put_AutoShow(OAFALSE);
		pVideoWindow->put_Visible(OATRUE);
		pVideoWindow->Release();
	}

	// run the graph
	//
	IMediaControl* pMediaControl = NULL;
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)(&pMediaControl));
	if (SUCCEEDED(hr))
	{
		pMediaControl->Run();
		pMediaControl->Release();
	}
}

void
WebCam::SetCaptureSize(void)
{
	if (!pBuilder) return;

	IAMStreamConfig* pConfig = NULL;

	HRESULT hr;
	hr = pBuilder->FindInterface(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		pVidcap,
		IID_IAMStreamConfig,
		(void**)&pConfig);
	if (FAILED(hr))
		return;

	int iFormatCount = 0;
	int iFormatSize = 0;
	hr = pConfig->GetNumberOfCapabilities(&iFormatCount, &iFormatSize);
	if (FAILED(hr))
	{
		pConfig->Release();
		return;
	}

	BYTE* pSCC = new BYTE[iFormatSize];

	for (int i=0; i<iFormatCount; ++i)
	{
		AM_MEDIA_TYPE* pMediatype = NULL;
		hr = pConfig->GetStreamCaps(i, &pMediatype, pSCC);
		if (SUCCEEDED(hr))
		{
			if (pMediatype->formattype == FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER* pHeader = NULL;
				pHeader = (VIDEOINFOHEADER*)pMediatype->pbFormat;

				if ((pHeader->bmiHeader.biWidth == iCaptureWidth)
					&& (pHeader->bmiHeader.biHeight == iCaptureHeight))
				{
					hr = pConfig->SetFormat(pMediatype);
					break;
				}
			}
			else if (pMediatype->formattype == FORMAT_VideoInfo2)
			{
				VIDEOINFOHEADER2* pHeader = NULL;
				pHeader = (VIDEOINFOHEADER2*)pMediatype->pbFormat;

				if ((pHeader->bmiHeader.biWidth == iCaptureWidth)
					&& (pHeader->bmiHeader.biHeight == iCaptureHeight))
				{
					hr = pConfig->SetFormat(pMediatype);
					break;
				}
			}

			DeleteMediaType(pMediatype);
		}
	}

	delete[] pSCC;
	pConfig->Release();
}

void
WebCam::GetCaptureSize(void)
{
	if (!pBuilder) return;

	IAMStreamConfig* pConfig = NULL;

	HRESULT hr;
	hr = pBuilder->FindInterface(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		pVidcap,
		IID_IAMStreamConfig,
		(void**)&pConfig);
	if (FAILED(hr))
		return;

	AM_MEDIA_TYPE* pMediatype = NULL;
	hr = pConfig->GetFormat(&pMediatype);
	if (SUCCEEDED(hr))
	{
		if (pMediatype->formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER* pHeader = NULL;
			pHeader = (VIDEOINFOHEADER*)pMediatype->pbFormat;

			iCaptureWidth = pHeader->bmiHeader.biWidth;
			iCaptureHeight = pHeader->bmiHeader.biHeight;
		}
		else if (pMediatype->formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* pHeader = NULL;
			pHeader = (VIDEOINFOHEADER2*)pMediatype->pbFormat;

			iCaptureWidth = pHeader->bmiHeader.biWidth;
			iCaptureHeight = pHeader->bmiHeader.biHeight;
		}

		DeleteMediaType(pMediatype);
	}

	pConfig->Release();
}

void WebCam::CropPreview()
{
	if (uDeviceType != deviceType_camera)
	{
		return;
	}
	IVideoWindow* pWindow = NULL;
	IBasicVideo* pVideo = NULL;
	HRESULT hr;

	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pWindow);
	if (FAILED(hr))
		return;

	long windowWidth, windowHeight;
	pWindow->get_Width(&windowWidth);
	pWindow->get_Height(&windowHeight);

	hr = pGraph->QueryInterface(IID_IBasicVideo, (void**)&pVideo);
	if (SUCCEEDED(hr))
	{
		long sourceWidth, sourceHeight;
		long sourceX, sourceY;
		if (!bCrop)
		{
			pVideo->get_VideoWidth(&sourceWidth);
			pVideo->get_VideoHeight(&sourceHeight);
			sourceX = 0;
			sourceY = 0;
		}
		else
		{
			sourceWidth = uCropWidth;
			sourceHeight = uCropHeight;
			sourceX = uCropX;
			sourceY = uCropY;
		}

		pVideo->SetSourcePosition(sourceX, sourceY, sourceWidth, sourceHeight);

		float aspectRatio = (float)uImageWidth / (float)uImageHeight;
		long destWidth, destHeight;
		if (aspectRatio * windowHeight > windowWidth)
		{
			destWidth = windowWidth;
			destHeight = (long)((float)destWidth / aspectRatio);
		}
		else
		{
			destHeight = windowHeight;
			destWidth = (long)((float)destHeight * aspectRatio);
		}

		pVideo->SetDestinationPosition(
			(long)(((float)windowWidth - (float)destWidth) / 2.0f),
			(long)(((float)windowHeight - (float)destHeight) / 2.0f),
			destWidth, destHeight);

		pVideo->Release();
	}

	pWindow->Release();
}
