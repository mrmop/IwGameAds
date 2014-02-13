// 
//
// IwGame - Cross Platform Multi-purpose Game Engine using the Marmalade SDK
//
// Developed by Matthew Hopwood of Pocketeers Limited - www.pocketeers.co.uk
//
// For updates, tutorials and more details check out my blog at www.drmop.com
//
// This code is provided free of charge and without any warranty whatsoever. The only restriction to its usage is that this header must remain intact and visible in all IwGame engine files.
// If you use this engine in your product, whilst it is not mandatory, a mention of the IwGame engine would be nice or a quick email to let us know where it is being used.
//
//

#include "IwGameUtil.h"
#include "IwGameString.h"
#include "IwGameFile.h"

#define DEBUG_FILENAME	"raw:///sdcard/debug.txt"
//#define DEBUG_FILENAME	"raw://c:\debug.txt"

//
//
// CIwGameUtils implementation
//
//
struct IwGameGraphicsMode
{
	const char* name;
	int			width;
	int			height;
};
static IwGameGraphicsMode g_CIwGameUtils_GraphicsModeNames[] = 
{
	{ "QQVGA", 160, 120 }, 
	{ "HQVGA", 240, 160 }, 
	{ "QVGA", 320, 240 }, 
	{ "WQVGA", 400, 240 }, 
	{ "HVGA", 480, 320 }, 
	{ "VGA", 640, 480 }, 
	{ "WVGA", 800, 480 }, 
	{ "FWVGA", 854, 480 }, 
	{ "SVGA", 800, 600 }, 
	{ "WSVGA", 1024, 576 }, 
	{ "WSVGA", 1024, 600 }, 
	{ "XVGA", 1024, 768 }, 
	{ "WXVGA", 1280, 768 }, 
	{ "XVGA+", 1152, 864 }, 
	{ "WXVGA+", 1140, 900 }, 
	{ "SXVGA", 1280, 1024 }, 
	{ "SXVGA+", 1400, 1050 }, 
	{ "WSXVGA+", 1680, 1050 }, 
	{ "UXVGA", 1600, 1200 }, 
	{ "WUXVGA", 1920 , 1200 }, 
	{ "QWXGA", 2048, 1152 }, 
	{ "QXGA", 2048, 1536 }, 
	{ "WQXGA", 2560, 1600 }, 
	{ "QSXGA", 2560, 2048 }, 
	{ "WQSXGA", 3200, 2048 }, 
	{ "QUXGA", 3200, 2400 }, 
	{ "WQUXGA", 3840, 2400 }, 
	{ "HXGA", 4096, 3072 }, 
	{ "WHXGA", 5120, 3200 }, 
	{ "HSXGA", 5120, 4096 }, 
	{ "WHSXGA", 6400, 4096 }, 
	{ "HUXGA", 6400, 4800 }, 
	{ "WHUXGA", 7680, 4800 }, 
	{ "nHD", 640, 360 }, 
	{ "qHD", 960, 540 }, 
	{ "WQHD", 2560, 1440 }, 
	{ "QFHD", 3840, 2160 }, 
};

const char* CIwGameUtils::GetGraphicModeName(int width, int height)
{
	for (int t = 0; t < 37; t++)
	{
		int w = g_CIwGameUtils_GraphicsModeNames[t].width;
		int h = g_CIwGameUtils_GraphicsModeNames[t].height;
		if ((width == w && height == h) || (width == h && height == w))
			return g_CIwGameUtils_GraphicsModeNames[t].name;
	}

	return NULL;
}

//
//
// CError implementation
//
//
void CIwGameError::LogError(const char* message)
{
	s3eDebugOutputString(message);

/*	s3eFile* file = s3eFileOpen(DEBUG_FILENAME, "ab");
	if (file != NULL)
	{
		s3eFileWrite((void*)message, strlen(message), 1, file);
		s3eFileWrite((void*)"\r\n", 2, 1, file);
		s3eFileClose(file);
	}*/
}
void CIwGameError::LogError(const char* message, const char* data)
{
	CIwGameString str;

	str.setString(message);
	str += (char*)data;

	s3eDebugOutputString(str.c_str());

/*	s3eFile* file = s3eFileOpen(DEBUG_FILENAME, "ab");
	if (file != NULL)
	{
		s3eFileWrite((void*)str.c_str(), str.GetLength(), 1, file);
		s3eFileWrite((void*)"\r\n", 2, 1, file);
		s3eFileClose(file);
	}*/
}
