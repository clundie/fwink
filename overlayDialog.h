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

struct
OverlayDialog
{
	WebCam* cam;
	Image* imageBase;
	Image* imageOverlay;
	Image* imageComposite;
	UINT uPreviewWidth, uPreviewHeight;
	HWND hwndDlg;
	COLORREF clrKeyLast;
	HBRUSH hbrSampleBackground;

	void SetOverlayFile(LPCTSTR szFile);
	void UpdateComposite();

	void InitKeyControls();
	void InitScaleControls();

	OverlayDialog(HWND hwndDlg, UINT uPreviewWidth, UINT uPreviewHeight, COLORREF clrKeyLast)
	{
		imageBase = NULL;
		imageOverlay = NULL;
		imageComposite = NULL;
		this->uPreviewWidth = uPreviewWidth;
		this->uPreviewHeight = uPreviewHeight;
		this->hwndDlg = hwndDlg;
		this->clrKeyLast = clrKeyLast;
		hbrSampleBackground = NULL;
		clrKeyLast = RGB(0, 0, 0);
	}

	~OverlayDialog()
	{
		if (imageBase) delete imageBase;
		imageBase = NULL;

		if (imageOverlay) delete imageOverlay;
		imageOverlay = NULL;

		if (imageComposite) delete imageComposite;
		imageComposite = NULL;

		if (hbrSampleBackground) DeleteObject(hbrSampleBackground);
	}
};
