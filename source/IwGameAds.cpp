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

#include "IwGameAds.h"
#include "IwGameFile.h"
#include "IwGameImage.h"
#include "IwGameUtil.h"
#include "IwGameAdsMediator.h"

#define _DEBUG

CDECLARE_SINGLETON(CIwGameAds)

//
// Supported Portal ID's
//
int	CIwGameAds::PortalIDs[] = 
{
	0,		// Invalid
	559,	// Android banner
	600,	// Android text
	661,	// Bada banner
	846,	// Bada text
	635,	// Blackberry banner
	635,	// Blackberry text
	947,	// iPad banner
	946,	// iPad text
	642,	// iPhone banner
	632,	// iPhone text
	551,	// Nokia OVI banner
	519,	// Nokia OVI text
	738,	// Web OS banner
	737,	// Web OS text
	659,	// Windows Mobile banner
	788,	// Windows Mobile text
	1041,	// Mobile web banner
	1042,	// Mobile web text
};

//
// Server response codes
//
unsigned int CIwGameAds::ResponseCodes[] = 
{
	IwHashString("OK"), 
	IwHashString("House Ad"), 
	IwHashString("Internal Error"), 
	IwHashString("Invalid Input"), 
	IwHashString("Unknown App Id"), 
	IwHashString("noAD"), 
};


CIwGameAd* CIwGameAd::getCopy()
{
	if (Image == NULL)
		return NULL;
	if (Image->getImage2D() == NULL || LinkURI.IsEmpty())
		return NULL;

	CIwGameAd* ad = new CIwGameAd();
	ad->isHtml = isHtml;
	ad->isText= isText;
	ad->Text.setString(Text.c_str());
	ad->ImageURI.setString(ImageURI.c_str());
	ad->LinkURI.setString(LinkURI.c_str());
	ad->Image = Image;
	ad->ImageFormat = ImageFormat;
	ad->AdTime = AdTime;
	Image = NULL;	// Wipe out image as its not managed by the ad view system

	return ad;
}


#if defined(_DEBUG)
char g_HttpResponse[4096];
#endif	// _DEBUG

int32 AdInfoRetrievedCallback(void* caller, void *data)
{
	CIwGameHttpRequest* request = (CIwGameHttpRequest*)caller;

	if (request->getProcessed())									// Check to see if our request was processed by the http manager
	{
		IW_GAME_ADS->AdReceived(request, request->getError());
		IW_GAME_HTTP_MANAGER->RemoveRequest(request);				// Remove request from http manager queue
	}

	return 0;
}

int32 AdImageRetrievedCallback(void* caller, void *data)
{
	CIwGameHttpRequest* request = (CIwGameHttpRequest*)caller;

	if (request->getProcessed())									// Check to see if our request was processed by the http manager
	{
		IW_GAME_ADS->AdImageReceived(request, request->getError());
		IW_GAME_HTTP_MANAGER->RemoveRequest(request);				// Remove request from http manager queue
	}

	return 0;
}

bool CIwGameAds::Init()
{
	Mediator = NULL;
	AdAvailableCallback = NULL;
	Error = ErrorNone;
	ErrorString = "";
	ErrorString.setAutoHash(true);
	ClientID = "-1";
	HtmlAds = false;
	TextAds = false;
	UserAge = 0;
	UserGender = GenderInvalid;
	AdAvailable = false;
	UserAgent = IW_GAME_HTTP_MANAGER->getUserAgent();

	Version = "Sm2m-1.5.3";

	// Get device UDID
	int os = s3eDeviceGetInt(S3E_DEVICE_OS);
	if (os != S3E_OS_ID_IPHONE)
		UDID = s3eDeviceGetInt(S3E_DEVICE_UNIQUE_ID);
	else
		UDID = 0;

	// Get device surface dimensions
	Width = IwGxGetScreenWidth();
	Height = IwGxGetScreenHeight();

	// Find our which portal ID we should be using
	PortalType = FindPortalType(TextAds);

	// Allocate string large enough to hold our largest request
	RequestURI.allocString(2048);

	// Get previous session client ID
	CIwGameFile file;
	if (file.Open("\\iwgameads.dat", "rb"))
	{
		int len = file.getFileSize();
		ClientID.allocString(len);
		ClientID.setLength(len);
		file.Read((void*)ClientID.c_str(), len);
	}

//	ClientID = CIwGameString(UDID);

	return true;			// Pointer support
}

void CIwGameAds::Release()
{
	AdRequest.setContentAvailableCallback(NULL, NULL);

	// Save sessions client ID
	CIwGameFile file;
	if (file.Open("\\iwgameads.dat", "wb"))
	{
		file.Write((void*)ClientID.c_str(), ClientID.GetLength() + 1);
	}

	// Clean-up mediator
	SAFE_DELETE(Mediator)
}

void CIwGameAds::Update()
{
	// Check for request timing out
	if (!AdAvailable)
	{
		if (BusyTimer.hasStarted())
		{
			if (BusyTimer.HasTimedOut())
			{
#if defined(_DEBUG)
				CIwGameError::LogError("Info: CIwGameAds::RequestAd() timed out: ");
#endif	// _DEBUG
				Error = ErrorRequestTimedOut;
				NotifyAdAvailable();
			}
		}
	}
}

bool CIwGameAds::RequestAd(eAdProvider provider, bool reset_mediator)
{
#if defined(_DEBUG)
	CIwGameError::LogError("Info: CIwGameAds::RequestAd()");
#endif	// _DEBUG

#if defined(_DEBUG)
	CIwGameError::LogError("Info: Busy, cant request ad - Elapsed = ", CIwGameString((int)BusyTimer.GetElapsedTime()).c_str());
#endif	// _DEBUG
	// Dont allow request if busy already collecting ad or timeout has not occured
	if (BusyTimer.hasStarted())
	{
		return false;
	}

	// If we have an attached mediator then get details from the mediator
	if (Mediator != NULL)
	{
		// reset the mediator
		if (reset_mediator)
			Mediator->reset();

		// Set up custom ad request from mediator
		CIwGameAdsParty* party = Mediator->getNextAdParty();
		if (party != NULL)
		{
			provider = party->Provider;
			ApplicationID = party->ApplicationID;
			OtherID = party->OtherID;
			ExtraInfo = party->ExtraInfo;
		}
	}

	// Reset error
	Error = ErrorNone;
	ErrorString = "";
	AdAvailable = false;
	AdProvider = provider;
	switch (AdProvider)
	{
	case InnerActive:
		RequestAdInnerActive();
		break;
	case AdFonic:
		RequestAdAdFonic();
		break;
	case VServ:
		RequestAdVServ();
		break;
	case Mojiva:
		RequestAdMojiva();
		break;
	case MillennialMedia:
		RequestAdMillennialMedia();
		break;
	case AdModa:
		RequestAdAdModa();
		break;
#if defined(_AD_DO_NOT_USE_)
	case InMobi:
		RequestAdInMobi();
		break;
	case MobClix:
		RequestAdMobClix();
		break;
	case MobFox:
		RequestAdMobFox();
		break;
	case Madvertise:
		RequestAdMadvertise();
		break;
	case KomliMobile:
		RequestAdKomliMobile();
		break;
#endif	// _AD_DO_NOT_USE_
	}

#if defined(_DEBUG)
	CIwGameError::LogError("Info: CIwGameAds::RequestAd() with URL: ", RequestURI.c_str());
#endif	// _DEBUG

	return true;
}

CIwGameAds::eIwGameAdsPortalType CIwGameAds::FindPortalType(bool text_ad)
{
	// Get the devices operating system
	OperatingSystem = s3eDeviceGetInt(S3E_DEVICE_OS);

	// Calculate portal type from OS
	switch (OperatingSystem)
	{
	case S3E_OS_ID_SYMBIAN:
		if (text_ad)
			return OVIText;
		return OVIBanner;
	case S3E_OS_ID_WINMOBILE:
	case S3E_OS_ID_WINCE:
		if (text_ad)
			return WinMobileText;
		return WinMobileBanner;
	case S3E_OS_ID_QNX:
		if (text_ad)
			return BlackberryText;
		return BlackberryBanner;
/*	case S3E_OS_ID_BADA:
		if (text_ad)
			return BadaText;
		return BadaBanner;*/
	case S3E_OS_ID_ANDROID:
		if (text_ad)
			return AndroidText;
		return AndroidBanner;
	case S3E_OS_ID_IPHONE:
		if (Width > 960 && Height > 640 || Height > 960 && Width > 640)
		{
			if (text_ad)
				return iPadText;
			return iPadBanner;
		}
		else
		{
			if (text_ad)
				return iPhoneText;
			return iPhoneBanner;
		}
	case S3E_OS_ID_WEBOS:
		if (text_ad)
			return WebOSText;
		return WebOSBanner;
	case S3E_OS_ID_WINDOWS:
	case S3E_OS_ID_LINUX:
	case S3E_OS_ID_WIPI:
	case S3E_OS_ID_NDS:
	case S3E_OS_ID_ARM_SEMIH:
	case S3E_OS_ID_NUCLEUS:
	case S3E_OS_ID_NGI:
	case S3E_OS_ID_SHARPEMP:
	case S3E_OS_ID_OSX:
	case S3E_OS_ID_UIQ:
	case S3E_OS_ID_PS3:
	case S3E_OS_ID_X360:
	case S3E_OS_ID_PSP:
	case S3E_OS_ID_WII:
	default:
		if (text_ad)
			return MobileWebText;
		else
			return MobileWebBanner;
		break;
	}

	return PortalNone;
}
		
void CIwGameAds::NotifyAdAvailable()
{
	// if no ad returned and mediator present then request ad from another party
	if (Error != ErrorNone)
	{
		if (Mediator != NULL)
		{
			RequestAd(InnerActive, false);
			return;
		}
	}

	// Call any user supplied callback
	if (AdAvailableCallback != NULL)
		AdAvailableCallback((void*)this, NULL);

	// Allow collection of new ads
	BusyTimer.Stop();

	// Mark ad as availavle
	setAdAvailable(true);
}

void CIwGameAds::AdReceived(CIwGameHttpRequest* request, int error)
{
	// If there wwas an error then set the error
	if (error != 0)
	{
		IW_GAME_ADS->setError(ErrorHttp);
		IW_GAME_ADS->setErrorString("Http error");
#if defined(_DEBUG)
		CIwGameError::LogError("Error: CIwGameAds::AdReceived(): ", CIwGameString(error).c_str());
#endif	// _DEBUG
		NotifyAdAvailable();
	}
	else
	{
#if defined(_DEBUG)
		if (request->getContentLength() > 4096)
			memcpy(g_HttpResponse, request->getContent().c_str(), 4096);
		else
			memcpy(g_HttpResponse, request->getContent().c_str(), request->getContentLength());
		g_HttpResponse[request->getContentLength()] = 0;
		CIwGameError::LogError("Info: CIwGameAds::AdReceived(): *************");
		CIwGameError::LogError((const char*)&g_HttpResponse[0]);
		CIwGameError::LogError("****************************");
#endif	// _DEBUG
		// Replace html chars
		request->getContent().ReplaceHTMLCodes();
		if (!ExtractAd(AdInfo, request->getContent()))
		{
#if defined(_DEBUG)
			CIwGameError::LogError("Error: CIwGameAds::AdReceived(): Invalid ad data");
#endif	// _DEBUG
			Error = ErrorInvalidAdData;
			NotifyAdAvailable();
			return;
		}
		if (!HtmlAds && !TextAds && AdInfo.ImageURI.GetLength() != 0)
		{
			// Download the banner ad image
			RequestBannerImage(AdInfo);
		}
		else
		{
#if defined(_DEBUG)
			if (AdInfo.ImageURI.GetLength() == 0)
			{
				CIwGameError::LogError("Error: CIwGameAds::AdReceived(): Content length is 0!");
			}
#endif	// _DEBUG
			NotifyAdAvailable();
		}
	}
}

bool CIwGameAds::RequestBannerImage(CIwGameAd& ad)
{
	// Submit get request
	AdRequest.setGET();
	AdRequest.setURI(ad.ImageURI.c_str());
	AdRequest.setContentAvailableCallback(&AdImageRetrievedCallback, NULL);
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);

#if defined(_DEBUG)
	CIwGameError::LogError("Info: CIwGameAds::RequestBannerImage() with URL:", RequestURI.c_str());
#endif	// _DEBUG

	return true;
}

void CIwGameAds::AdImageReceived(CIwGameHttpRequest* request, int error)
{
	s3eDebugOutputString("Ad image received");
	
	// If there wwas an error then set the error
	if (error != 0)
	{
		IW_GAME_ADS->setError(ErrorHttpImage);
		IW_GAME_ADS->setErrorString("Http error retrieving image");
#if defined(_DEBUG)
		CIwGameError::LogError("Error: CIwGameAds::AdImageReceived(): ", CIwGameString(error).c_str());
#endif	// _DEBUG
	}
	else
	{
		eIwGameImageFormat format = GetImageFormatFromHeader();
		if (format == ImageFormatInvalid)
		{
			Error = ErrorInvalidImage;
			ErrorString = "Invalid image format";
		}
		else
		{
			Error = ErrorNone;
			ErrorString = "";

			AdInfo.ImageFormat = format;
#if defined(_DEBUG)
			CIwGameError::LogError("Info: CIwGameAds::AdImageReceived() - Image format - ", CIwGameString((int)format).c_str());
			CIwGameError::LogError("Info: CIwGameAds::AdImageReceived() - Image URL - ", AdInfo.ImageURI.c_str());
			CIwGameError::LogError("Info: CIwGameAds::AdImageReceived() - Link URL - ", AdInfo.LinkURI.c_str());
#endif	// _DEBUG

			// Delete previous image (if any)
			if (AdInfo.Image != NULL)
			{
				delete AdInfo.Image;
				AdInfo.Image = NULL;
			}

			// Create new banner image
			AdInfo.Image = new CIwGameImage();
			if (!AdInfo.Image->Init((void*)request->getContent().c_str(), request->getContentLength()))
			{
#if defined(_DEBUG)
				CIwGameError::LogError("Info: CIwGameAds::AdImageReceived() - Could hot create image!");
#endif	// _DEBUG
				Error = ErrorInvalidImage;
				ErrorString = "Invalid image format";
				delete AdInfo.Image;
				AdInfo.Image = NULL;
			}
			else
			{
CIwGameError::LogError("Info: Image Width - ", CIwGameString(AdInfo.Image->getWidth()).c_str());
CIwGameError::LogError("Info: Image Height - ", CIwGameString(AdInfo.Image->getHeight()).c_str());
			}
		}
	}
	NotifyAdAvailable();
}

//
//
//
//
//
//
// Utility methods
//
//
//
//
//
//
bool CIwGameAds::ExtractAd(CIwGameAd& ad, CIwGameString& ad_body)
{
	switch (AdProvider)
	{
	case InnerActive:
		return ExtractAdInnerActive(ad, ad_body);
		break;
	case AdFonic:
		return ExtractAdAdFonic(ad, ad_body);
		break;
	case VServ:
		return ExtractAdVServ(ad, ad_body);
		break;
	case Mojiva:
		return ExtractAdMojiva(ad, ad_body);
		break;
	case MillennialMedia:
		return ExtractAdMillennialMedia(ad, ad_body);
		break;
	case AdModa:
		return ExtractAdAdModa(ad, ad_body);
		break;
#if defined(_AD_DO_NOT_USE_)
	case InMobi:
		return ExtractAdInMobi(ad, ad_body);
		break;
	case MobClix:
		return ExtractAdMobClix(ad, ad_body);
		break;
	case MobFox:
		return ExtractAdMobFox(ad, ad_body);
		break;
	case Madvertise:
		return ExtractAdMadvertise(ad, ad_body);
		break;
	case KomliMobile:
		return ExtractAdKomliMobile(ad, ad_body);
		break;
#endif	// _AD_DO_NOT_USE_
	}

	return false;
}

void CIwGameAds::ErrorFromResponse(const char* error, int error_len)
{
	ErrorString.setString(error, error_len);

	unsigned int hash = ErrorString.CalculateHash(ErrorString.c_str());

	for (int t = 0; t < 5; t++)
	{
		if (ResponseCodes[t] == hash)
		{
			Error = (eIwGameAdsError)t;
			return;
		}
	}

	Error = ErrorNone;
}

CIwGameAds::eIwGameImageFormat CIwGameAds::GetImageFormatFromHeader()
{
	if (!IW_GAME_HTTP_MANAGER->GetHeader("CONTENT-TYPE", &ContentType))
		return ImageFormatInvalid;
	ContentType.setAutoHash(true);

	if (ContentType.Find("image/png"))
		return ImageFormatPNG;
	if (ContentType.Find("image/gif"))
		return ImageFormatGIF;
	if (ContentType.Find("image/jpeg") || ContentType.Find("image/jpg"))
		return ImageFormatJPG;

#if defined(_DEBUG)
	CIwGameError::LogError("Error: CIwGameAds::GetImageFormatFromHeader(): ", ContentType.c_str());
#endif	// _DEBUG

	return ImageFormatInvalid;
}

bool CIwGameAds::ExtractLinkAndImageFromtHTML(CIwGameAd& ad, CIwGameString& html)
{
	int pos, offset;

	// Check to see if the html has a clickable link
	html.FindReset();
	pos = html.FindNext("href=");
	if (pos > 0)
	{
		// Get the click link
		int idx = html.getFindIndex();
		int len = html.GetNextMarkedString('"', '"', offset);
		if (len < 0)
		{
			html.setFindIndex(idx);
			len = html.GetNextMarkedString('\'', '\'', offset);
		}
		if (len > 0)
		{
			ad.LinkURI.setString(html.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Link URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
		
			// Check to see if the html has an image available
			pos = html.FindNext("img ");
			if (pos > 0)
			{
				// Grab the image url
				pos = html.FindNext("src=");
				if (pos > 0)
				{
					idx = html.getFindIndex();
					len = html.GetNextMarkedString('"', '"', offset);
					if (len < 0)
					{
						html.setFindIndex(idx);
						len = html.GetNextMarkedString('\'', '\'', offset);
					}
					if (len > 0)
					{
						ad.ImageURI.setString(html.getString() + offset, len);
#if defined(_DEBUG)
						CIwGameError::LogError("Info: Ad Image URL: ", ad.ImageURI.c_str());
#endif	// _DEBUG
						return true;
					}
				}
			}
		}
	}

	return false;
}

//
//
//
//	Inner-Active specific implementation
//
//
//
bool CIwGameAds::RequestAdInnerActive()
{
	// Get device surface dimensions
	Width = IwGxGetScreenWidth();
	Height = IwGxGetScreenHeight();

	// Build M2M request URI string
	if (!HtmlAds)
		RequestURI = "http://m2m1.inner-active.com/simpleM2M/clientRequestAd?";
	else
		RequestURI = "http://m2m1.inner-active.com/simpleM2M/clientRequestHtmlAd?";
	RequestURI += "aid=";
	RequestURI += ApplicationID;
	RequestURI += "&po=";
	RequestURI += CIwGameString(PortalIDs[PortalType]);
	RequestURI += "&v=";
	RequestURI += Version;
	RequestURI += "&cid=";
	RequestURI += ClientID;
	RequestURI += "&hid=";
	RequestURI += CIwGameString(UDID);
	if (Width != 0)
	{
		RequestURI += "&w=";
		RequestURI += CIwGameString(Width);
	}
	if (Height != 0)
	{
		RequestURI += "&h=";
		RequestURI += CIwGameString(Height);
	}
	if (UserAge != 0)
	{
		RequestURI += "&a=";
		RequestURI += CIwGameString(UserAge);
	}
	if (UserGender != GenderInvalid)
	{
		if (UserGender == GenderFemale)
			RequestURI += "&g=f";
		else
			RequestURI += "&g=m";
	}
	if (!UserLocation.IsEmpty())
	{
		RequestURI += "&l=";
		RequestURI += UserLocation;
	}
	if (!Category.IsEmpty())
	{
		RequestURI += "&c=";
		RequestURI += Category;
	}
	if (!UserGPSLocation.IsEmpty())
	{
		RequestURI += "&lg=";
		RequestURI += UserGPSLocation;
	}
	if (!UserMobileNumber.IsEmpty())
	{
		RequestURI += "&mn=";
		RequestURI += UserMobileNumber;
	}
	if (!UserKeywords.IsEmpty())
	{
		RequestURI += "&k=";
		RequestURI += UserKeywords;
	}
	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.setBody("");
	AdRequest.SetHeader("Content-Length", "0");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdInnerActive(CIwGameAd& ad, CIwGameString& ad_body)
{
	// We dont use a full on XML parser to parse the returned XML, instead we just search for the required info
	int pos, offset, len;

	ad.isHtml = HtmlAds;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	// Get Error Response
	ad_body.FindReset();
	pos = ad_body.FindNext("<tns:Response Error=");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('"', '"', offset);
		if (len > 0)
		{
			ErrorFromResponse(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Response: ", ErrorString.c_str());
#endif	// _DEBUG
		}
	}

	// Get Client ID (we store this for future ad requests during the same session)
	pos = ad_body.FindNext("<tns:Client Id=");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('"', '"', offset);
		if (len > 0)
		{
			ClientID.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Client ID: ", ClientID.c_str());
#endif	// _DEBUG
		}
	}

	// Get Ad text
	pos = ad_body.FindNext("<tns:Text");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			ad.Text.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Text: ", ad.Text.c_str());
#endif	// _DEBUG
		}
	}

	// Get Ad link
	pos = ad_body.FindNext("<tns:URL");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			ad.LinkURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
		}
	}

	// Get Ad image
	pos = ad_body.FindNext("<tns:Image");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			ad.ImageURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Image URL: ", ad.ImageURI.c_str());
#endif	// _DEBUG
		}
	}

	return true;
}

//
//
//
//	ImMobi specific implementation
//
//
//
bool CIwGameAds::RequestAdInMobi()
{
	// Build M2M request URI string
	RequestURI = "http://w.inmobi.com/showad.asm";				// Live
//	RequestURI = "http://i.w.sandbox.inmobi.com/showad.asm";	// Test

	CIwGameString body;
	CIwGameString urlencoded;

	body = "mk-siteid=";
	body += ApplicationID;
	body += "&mk-carrier=";
	body += IW_GAME_HTTP_MANAGER->getIPAddress();
	body += "&h-user-agent=";
	urlencoded.URLEncode(UserAgent.c_str());
	urlencoded.ToLower();
	body += urlencoded;
	body += "&u-id=";
	body += CIwGameString(UDID);
	body += "&d-localization=";
	urlencoded.URLEncode(s3eDeviceGetString(S3E_DEVICE_LOCALE));
	urlencoded.ToLower();
	body += urlencoded;
//	body += "&d-netType=wifi";
	body += "&d-netType=carrier";
	body += "&mk-version=pr-spec-atata-20090521";
	if (UserAge != 0)
	{
		body += "&u-age=";
		body += CIwGameString(UserAge);
	}
	if (UserGender != GenderInvalid)
	{
		if (UserGender == GenderFemale)
			body += "&u-gender=f";
		else
			body += "&u-gender=m";
	}
	if (!UserGPSLocation.IsEmpty())
	{
		body += "&u-latlong=";
		body += UserGPSLocation;
	}
	if (!UserKeywords.IsEmpty())
	{
		body += "&u-interests=";
		body += UserKeywords;
	}
	if (!ExtraInfo.IsEmpty())
	{
		body += ExtraInfo;
	}
//	body.ToLower();

	AdRequest.setPOST();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("X-Mkhoj-SiteID", ApplicationID.c_str());
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", CIwGameString(body.GetLength()).c_str());
	AdRequest.setBody(body.c_str());
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdInMobi(CIwGameAd& ad, CIwGameString& ad_body)
{
	return false;
}


//
//
//
//	Mob Clix specific implementation
//
//
//
bool CIwGameAds::RequestAdMobClix()
{
	return false;
}
bool CIwGameAds::ExtractAdMobClix(CIwGameAd& ad, CIwGameString& ad_body)
{
	return false;
}

//
//
//
//	MobFox specific implementation
//
//
//
bool CIwGameAds::RequestAdMobFox()
{
	// Build request URI string
	RequestURI = "http://my.mobfox.com/request.php";

	CIwGameString body;
	CIwGameString urlencoded;

	body = "rt=api";
	body += "&u=";
	urlencoded.URLEncode(UserAgent.c_str());
	body += urlencoded;
	body += "&i=";
	body += IW_GAME_HTTP_MANAGER->getIPAddress();
	body += "&o=";
	body += CIwGameString(UDID);
	body += "&m=live";
	body += "&s=";
	body += ApplicationID;
	if (!ExtraInfo.IsEmpty())
	{
		body += ExtraInfo;
	}

	AdRequest.setPOST();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", CIwGameString(body.GetLength()).c_str());
	AdRequest.setBody(body.c_str());
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}

bool CIwGameAds::ExtractAdMobFox(CIwGameAd& ad, CIwGameString& ad_body)
{
	// We dont use a full on XML parser to parse the returned XML, instead we just search for the required info
	int pos, offset, len;

	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	// Get Error Response
	ad_body.FindReset();
	pos = ad_body.FindNext("<request type=");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('"', '"', offset);
		if (len > 0)
		{
			ErrorString.setString(ad_body.getString() + offset, len);
			if (ErrorString == "noAd")
			{
				Error = ErrorNoAd;
				return false;
			}
			else
				Error = ErrorNone;
			if (ErrorString == "textAd")
			{
				ad.isHtml = true;
				ad.isText = true;
			}
		}
	}

	// Get Ad text
	pos = ad_body.FindNext("<htmlString>");
	if (pos >= 0)
	{
		// Find closeing tag
		int end_pos = ad_body.FindNext("</htmlString>");
		if (end_pos > 0)
		{
			ad.Text.setString(ad_body.getString() + pos, end_pos - pos);
			ad.Text.ReplaceHTMLCodes();
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Text: ", ad.Text.c_str());
#endif	// _DEBUG
		}
		// If we can successfully extract the link and banner information from the text ad then unmark the ad as html/text
		if (ExtractLinkAndImageFromtHTML(ad, ad.Text))
		{
			ad.isHtml = false;
			ad.isText = false;
			return true;
		}
	}
	else
		ad_body.FindReset();

	// Get Ad link
	pos = ad_body.FindNext("<clickurl");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			ad.LinkURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
		}
	}

	// Get Ad image
	pos = ad_body.FindNext("<imageurl");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			ad.ImageURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Image URL: ", ad.ImageURI.c_str());
#endif	// _DEBUG
		}
	}

	return true;
}

//
//
//
//	AdFonic specific implementation
//
//
//
bool CIwGameAds::RequestAdAdFonic()
{
	// Get device surface dimensions
	Width = IwGxGetScreenWidth();
	Height = IwGxGetScreenHeight();

	CIwGameString urlencoded;

	RequestURI = "http://adfonic.net/ad/";
	RequestURI += ApplicationID;
	RequestURI += "?";
	RequestURI += "r.id=";
	urlencoded.URLEncode(CIwGameString(UDID).c_str());
	RequestURI += urlencoded;
	RequestURI += "&s.test=0";
	RequestURI += "&t.format=xml";
	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.setBody("");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdAdFonic(CIwGameAd& ad, CIwGameString& ad_body)
{
	// We dont use a full on XML parser to parse the returned XML, instead we just search for the required info
	int pos, offset, len;

	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	// Get Error Response
	ad_body.FindReset();
	pos = ad_body.FindNext("<status");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			ErrorString.setString(ad_body.getString() + offset, len);
			if (ErrorString == "error")
			{
				Error = ErrorInternalError;
				return false;
			}
			else
				Error = ErrorNone;
		}
	}

	// Get Ad format
	pos = ad_body.FindNext("<format");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('>', '<', offset);
		if (len > 0)
		{
			CIwGameString format;
			format.setString(ad_body.getString() + offset, len);
			if (format == "banner")
				ad.isText = false;
			else
				ad.isText = true;

#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Format: ", ad.Text.c_str());
#endif	// _DEBUG
		}
	}

	// Get Ad text
	pos = ad_body.FindNext("<![CDATA[");
	if (pos >= 0)
	{
		int end_pos = ad_body.FindNext("]]>");
		if (end_pos > 0)
		{
			ad.Text.setString(ad_body.getString() + pos + 9, end_pos - pos - 9);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Text: ", ad.Text.c_str());
#endif	// _DEBUG
			ad.isHtml = true;
		}
	}
	ad_body.FindReset();

	// Get Ad link
	pos = ad_body.FindNext("url=");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('"', '"', offset);
		if (len > 0)
		{
			ad.LinkURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
		}
	}

	// Get Ad image
	pos = ad_body.FindNext("src=");
	if (pos >= 0)
	{
		len = ad_body.GetNextMarkedString('"', '"', offset);
		if (len > 0)
		{
			ad.ImageURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
			CIwGameError::LogError("Info: Ad Image URL: ", ad.ImageURI.c_str());
#endif	// _DEBUG
		}
	}

	return true;
}

//
//
//
//	Madvertise specific implementation
//
//
//


bool CIwGameAds::RequestAdMadvertise()
{
	// Build M2M request URI string
	RequestURI = "http://ad.madvertise.de/site/";
	RequestURI += ApplicationID;

	CIwGameString body;
	CIwGameString urlencoded;

	body += "ua=";
	urlencoded.URLEncode(UserAgent.c_str());
	body += urlencoded;
//	body += "&ip=";
//	body += IW_GAME_HTTP_MANAGER->getIPAddress();
	body += "&requester=madvertise_api";
	body += "&version=api_2.1";
	body += "&unique_device_id=";
	body += CIwGameString(UDID);
	if (!ExtraInfo.IsEmpty())
	{
		body += ExtraInfo;
	}

/*	CIwGameString local = s3eDeviceGetString(S3E_DEVICE_LOCALE);
	int pos = local.Contains('_');
	if (pos >= 0)
	{
		// Strip language and underscore
		local.setString(local.c_str() + pos + 1, 2);
		local.ToUpper();
		body += "&country=";
		body += local;
	}*/

	if (UserAge != 0)
	{
		body += "&age=";
		body += CIwGameString(UserAge);
	}
	if (UserGender != GenderInvalid)
	{
		if (UserGender == GenderFemale)
			body += "&gender=F";
		else
			body += "&gender=M";
	}
	if (!UserKeywords.IsEmpty())
	{
		body += "&keywords=";
		body += UserKeywords;
	}

	AdRequest.setPOST();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Accept", "application/xml");
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", CIwGameString(body.GetLength()).c_str());
	AdRequest.setBody(body.c_str());
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdMadvertise(CIwGameAd& ad, CIwGameString& ad_body)
{
	return false;
}


//
//
//
//	Mojiva specific implementation
//
//
//
bool CIwGameAds::RequestAdMojiva()
{
	// Build M2M request URI string
	RequestURI = "http://ads.mojiva.com/ad?";

	CIwGameString urlencoded;

	RequestURI += "zone=";
	RequestURI += ApplicationID;
	RequestURI += "&ip=";
	RequestURI += IW_GAME_HTTP_MANAGER->getIPAddress();
	urlencoded.URLEncode(UserAgent.c_str());
	RequestURI += urlencoded;
	RequestURI += "&udid=";
	RequestURI += CIwGameString(UDID);

	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

/*	CIwGameString local = s3eDeviceGetString(S3E_DEVICE_LOCALE);
	int pos = local.Contains('_');
	if (pos >= 0)
	{
		// Strip language and underscore
		local.setString(local.c_str() + pos + 1, 2);
		local.ToUpper();
		body += "&country=";
		body += local;
	}*/

/*	if (UserAge != 0)
	{
		body += "&age=";
		body += CIwGameString(UserAge);
	}
	if (UserGender != GenderInvalid)
	{
		if (UserGender == GenderFemale)
			body += "&gender=F";
		else
			body += "&gender=M";
	}
	if (!UserKeywords.IsEmpty())
	{
		body += "&keywords=";
		body += UserKeywords;
	}*/

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", "0");
	AdRequest.setBody("");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdMojiva(CIwGameAd& ad, CIwGameString& ad_body)
{
	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	ExtractLinkAndImageFromtHTML(ad, ad_body);

	if (ad.LinkURI.IsEmpty() || ad.ImageURI.IsEmpty())
		return false;

	return true;
}

//
//
//
//	MillennialMedia specific implementation
//
//
//
bool CIwGameAds::RequestAdMillennialMedia()
{
	// Build M2M request URI string
	RequestURI = "http://ads.mp.mydas.mobi/getAd.php5?";
	CIwGameString urlencoded;

	RequestURI += "apid=";
	RequestURI += ApplicationID;
	RequestURI += "&auid=";
	RequestURI += CIwGameString(UDID);
	RequestURI += "&ua=";
	urlencoded.URLEncode(UserAgent.c_str());
	RequestURI += urlencoded;
//	RequestURI += "&mode=test";
//	RequestURI += "&uip=";
//	RequestURI += IW_GAME_HTTP_MANAGER->getIPAddress();

	if (UserAge != 0)
	{
		RequestURI += "&age=";
		RequestURI += CIwGameString(UserAge);
	}
	if (UserGender != GenderInvalid)
	{
		if (UserGender == GenderFemale)
			RequestURI += "&gender=female";
		else
			RequestURI += "&gender=male";
	}
	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Accept", "application/xml");
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", "0");
	AdRequest.setBody("");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}

bool CIwGameAds::ExtractAdMillennialMedia(CIwGameAd& ad, CIwGameString& ad_body)
{
	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	ExtractLinkAndImageFromtHTML(ad, ad_body);

	if (ad.LinkURI.IsEmpty() || ad.ImageURI.IsEmpty())
		return false;

	return true;
}


//
//
//
//	VServ specific implementation
//
//
//
bool CIwGameAds::RequestAdVServ()
{
	// Get device surface dimensions
	Width = IwGxGetScreenWidth();
	Height = IwGxGetScreenHeight();

	// Build M2M request URI string
	RequestURI = "http://a.vserv.mobi/delivery/adapi.php?";
	CIwGameString urlencoded;

	RequestURI += "zoneid=";
	RequestURI += ApplicationID;
	RequestURI += "&im=";
	RequestURI += CIwGameString(UDID);
	RequestURI += "&lc=";
	RequestURI += s3eDeviceGetString(S3E_DEVICE_LOCALE);
	RequestURI += "&app=1";
	RequestURI += "&ts=1";
	RequestURI += "&ua=";
	urlencoded.URLEncode(UserAgent.c_str());
	RequestURI += urlencoded;
	if (Width != 0)
	{
		RequestURI += "&sw=";
		RequestURI += CIwGameString(Width);
	}
	if (Height != 0)
	{
		RequestURI += "&sh=";
		RequestURI += CIwGameString(Height);
	}
	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Accept", "application/xml");
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", "0");
	AdRequest.setBody("");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdVServ(CIwGameAd& ad, CIwGameString& ad_body)
{
	// We dont use a full on XML parser to parse the returned XML, instead we just search for the required info
	int pos;

	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	ad_body.FindReset();

	// Find render section
	if (ad_body.FindNext("\"action\"") < 0)
	{
		ad_body.FindReset();
		ExtractLinkAndImageFromtHTML(ad, ad_body);

		if (ad.LinkURI.IsEmpty() || ad.ImageURI.IsEmpty())
			return false;

		return true;
	}

	// Find click URL
	pos = ad_body.GetNextMarkedStringAfterString("\"data\"", '"', '"', ad.LinkURI);
	if (pos >= 0)
	{
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
	}
	else
		return false;

	// Find render section
	if (ad_body.FindNext("\"render\"") < 0)
	{
		return false;
	}

	// Find Image URL
	pos = ad_body.GetNextMarkedStringAfterString("\"data\"", '"', '"', ad.ImageURI);
	if (pos > 0)
	{
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Image URL: ", ad.ImageURI.c_str());
#endif	// _DEBUG
	}
	else
		return false;

/*	// Find click URL
	pos = ad_body.GetNextMarkedStringAfterString("\"notify\"", '"', '"', ad.LinkURI);
	if (pos >= 0)
	{
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
	}
	else
		return false;(*/

	return true;
}

//
//
//
//	AdModa specific implementation
//
//
//
bool CIwGameAds::RequestAdAdModa()
{
	// Get device surface dimensions
	Width = IwGxGetScreenWidth();
	Height = IwGxGetScreenHeight();

	// Build M2M request URI string
	RequestURI = "http://www.admoda.com/ads/fetch.php?";
	CIwGameString urlencoded;

	RequestURI += "v=4";
	RequestURI += "&l=php";
	RequestURI += "&z=";
	RequestURI += ApplicationID;
//	RequestURI += "&a=";
//	RequestURI += IW_GAME_HTTP_MANAGER->getIPAddress();
	RequestURI += "&ua=";
	urlencoded.URLEncode(UserAgent.c_str());
	RequestURI += urlencoded;
	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", "0");
	AdRequest.setBody("");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}

bool CIwGameAds::ExtractAdAdModa(CIwGameAd& ad, CIwGameString& ad_body)
{
	int len, offset;

	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	ad_body.FindReset();

	// Get Ad image
	len = ad_body.GetNextMarkedString('|', '|', offset);
	if (len > 0)
	{
		ad.ImageURI.setString(ad_body.getString() + offset, len);
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Image: ", ad.ImageURI.c_str());
#endif	// _DEBUG
	}
	ad_body.StepFindIndex(-1);

	// Get Ad link
	len = ad_body.GetNextMarkedString('|', '|', offset);
	if (len > 1)
	{
		ad.LinkURI.setString(ad_body.getString() + offset, len - 1);
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
	}


	if (ad.ImageURI.IsEmpty() || ad.LinkURI.IsEmpty())
		return false;

	return true;
}

//
//
//
//	KomliMobile specific implementation
//
//
//
bool CIwGameAds::RequestAdKomliMobile()
{
	// Get device surface dimensions
	Width = IwGxGetScreenWidth();
	Height = IwGxGetScreenHeight();

	// Build M2M request URI string
	RequestURI = "http://a.zestadz.com/waphandler/deliverad?";
	CIwGameString urlencoded;

	RequestURI += "ip=";
	RequestURI += IW_GAME_HTTP_MANAGER->getIPAddress();
	RequestURI += "&cid=";
	RequestURI += ApplicationID;
	RequestURI += "&ua=";
	urlencoded.URLEncode(UserAgent.c_str());
	RequestURI += urlencoded;
	RequestURI += "&response_type=xml";
//	RequestURI += CIwGameString(UDID);
	if (!ExtraInfo.IsEmpty())
	{
		RequestURI += ExtraInfo;
	}

	AdRequest.setGET();
	AdRequest.setURI(RequestURI.c_str());
	AdRequest.setContentAvailableCallback(&AdInfoRetrievedCallback, NULL);
	AdRequest.SetHeader("User-Agent", UserAgent.c_str());
	AdRequest.SetHeader("Accept", "application/xml");
	AdRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
	AdRequest.SetHeader("Content-Length", "0");
	AdRequest.setBody("");
	IW_GAME_HTTP_MANAGER->AddRequest(&AdRequest);
	BusyTimer.setDuration(IW_GAME_ADS_TIMEOUT);

	return true;
}
bool CIwGameAds::ExtractAdKomliMobile(CIwGameAd& ad, CIwGameString& ad_body)
{
return false;
	// We dont use a full on XML parser to parse the returned XML, instead we just search for the required info
	int pos;

	ad.isHtml = false;
	ad.isText = false;
	ad.ImageURI = "";
	ad.LinkURI = "";
	ad.Text = "";
	ad.AdTime = s3eTimerGetMs();

	ad_body.FindReset();

	// Find render section
	if (ad_body.FindNext("\"action\"") < 0)
	{
		return false;
	}

	// Find click URL
	pos = ad_body.GetNextMarkedStringAfterString("\"data\"", '"', '"', ad.LinkURI);
	if (pos >= 0)
	{
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
	}
	else
		return false;

	// Find render section
	if (ad_body.FindNext("\"render\"") < 0)
	{
		return false;
	}

	// Find Image URL
	pos = ad_body.GetNextMarkedStringAfterString("\"data\"", '"', '"', ad.ImageURI);
	if (pos > 0)
	{
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Image URL: ", ad.ImageURI.c_str());
#endif	// _DEBUG
	}
	else
		return false;

/*	// Find click URL
	pos = ad_body.GetNextMarkedStringAfterString("\"notify\"", '"', '"', ad.LinkURI);
	if (pos >= 0)
	{
#if defined(_DEBUG)
		CIwGameError::LogError("Info: Ad Click URL: ", ad.LinkURI.c_str());
#endif	// _DEBUG
	}
	else
		return false;(*/

	return true;
}





