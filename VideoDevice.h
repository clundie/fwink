#pragma once

#include <string>

class VideoDevice
{
public:
	VideoDevice(void);
public:
	~VideoDevice(void);

	std::wstring friendlyName;
	std::wstring devicePath;
};
