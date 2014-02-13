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

//
// Working ad providers
// --------------------
// Inner-active
// AdFonic
// VServ - Allows you to set up other networks including InMobi, BuzzCity, JumpTap, Zestadz and Inner-active
// Mojiva
// MillennialMedia - Allows you to set up other networks including AdMob, Amobee, JumpTap and Mojiva
// AdModa
//
// Ad providers that will never work
// ---------------------------------
// AdMob - Closed their REST API so will never be available
// MoPub - API is closed so will never be available
//
// Ad Providers with issues
// ------------------------
// MobClix - Cant find a REST API
// MobFox - Ads wont deliver over WiFi or Carrier and clicks are not registered
// InMobi - Cannot get a single ad, either get bad request error or no ad error
// Madvertise - Registering impressions but never returns any ads
// KomliMobile - Always receieve "your account  has been banned" message so gave up
//
// Ad Providers that need implementation
// -------------------------------------
// BuzzCity - 


#if !defined(_IW_GAME_ADS_H_)
#define _IW_GAME_ADS_H_

#include "IwGeom.h"
#include "IwGameUtil.h"
#include "IwGameHttp.h"
#include "IwGameImage.h"
#include "Iw2D.h"

//#define _AD_DO_NOT_USE_

#define	IW_GAME_ADS		(CIwGameAds::getInstance())

#define	IW_GAME_ADS_TIMEOUT		10000
#define	IW_GAME_MAX_CACHED_ADS	4
#define	IW_GAME_MAX_AD_AGE		(5 * 60000)

class CIwGameAdsMediator;

//
//
// CIwGameAd - The IwGameAd structure contains all data relating to the ad returned from the ad server
//
//
struct CIwGameAd
{
	CIwGameAd(): Image(NULL), isHtml(false), AdTime(0) {}
	~CIwGameAd()
	{
		SAFE_DELETE(Image)
	}

	bool			isHtml;			// If isHtml is set to true then only Text is valid and that cnotains the Ad HTML
	bool			isText;			// If true then the ad is a text ad
	CIwGameString	Text;			// Ad text
	CIwGameString	ImageURI;		// URI of image
	CIwGameString	LinkURI;		// URI to be called when user clicks the ad
	CIwGameImage*	Image;			// 2D image that represents the banner Ad
	int				ImageFormat;	// 2D Image format
	int64			AdTime;			// Time ad was collected

	CIwGameAd*		getCopy();
};

//
//
// CIwGameAds - The IwGameAds class is responsible for streaming ad data
//
//
class CIwGameAds
{
public:
	enum eAdProvider
	{
		InnerActive,
		AdFonic,
		VServ, 
		Mojiva,
		MillennialMedia, 
		AdModa, 
#if defined(_AD_DO_NOT_USE_)
		InMobi,
		MobClix,
		MobFox,
		Madvertise,
		KomliMobile, 
		BuzzCity, 
#endif	// _AD_DO_NOT_USE_
	};

	enum eIwGameAdsPortalType
	{
		PortalNone, 
		AndroidBanner, 
		AndroidText, 
		BadaBanner, 
		BadaText, 
		BlackberryBanner, 
		BlackberryText, 
		iPadBanner, 
		iPadText, 
		iPhoneBanner, 
		iPhoneText, 
		OVIBanner, 
		OVIText, 
		WebOSBanner, 
		WebOSText, 
		WinMobileBanner, 
		WinMobileText, 
		MobileWebBanner, 
		MobileWebText, 
	};
	enum eIwGameAdsGender
	{
		GenderInvalid, 
		GenderMale, 
		GenderFemale, 
	};
	enum eIwGameAdsError
	{
		ErrorNone, 
		ErrorHouseAd, 
		ErrorInternalError, 
		ErrorInvalidInput, 
		ErrorUnknownAppId, 
		ErrorNoAd, 
		ErrorHttp, 
		ErrorHttpImage, 
		ErrorInvalidImage, 
		ErrorRequestTimedOut, 
		ErrorInvalidAdData, 
	};
	enum eIwGameImageFormat
	{
		ImageFormatInvalid, 
		ImageFormatPNG, 
		ImageFormatGIF, 
		ImageFormatJPG, 
	};

	CDEFINE_SINGLETON(CIwGameAds)

	// Properties
protected:
	CIwGameString			Version;					// Protocol version string (for inner-active)
	CIwGameString			ApplicationID;				// ID of the application thats making the request (you will need to be assigned this from your ad provider)
	CIwGameString			OtherID;					// Extra ID information
	unsigned int			UDID;						// Unique ID of the device (used to identify device type, language and approx location to deliver more appropriate ads)
	int						Width, Height;				// Dimensions of device (used to collect more appropriate sized ads)
	eIwGameAdsPortalType	PortalType;					// Type of portal to collect ads from
	bool					TextAds;					// True if you want text ads to be returned, false if banner ads
	bool					HtmlAds;					// True if you want ads to be returned as html instead of xml (used by inner-active)
	int						UserAge;					// The users ages (optional)
	CIwGameString			UserAgent;					// User agent string (system will build this but it can be replaced)
	CIwGameString			IPAddress;					// IP Address of users device
	eIwGameAdsGender		UserGender;					// The users gender (optional)
	CIwGameString			UserLocation;				// Location string – comma separated list of country, state/province, city (optional)
	CIwGameString			UserGPSLocation;			// GPS Location string – ISO code location data in latitude, longitude format (optional)
	CIwGameString			Category;					// Single word description of the application (optional)
	CIwGameString			UserMobileNumber;			// Users mobile number - MSISDN format, with international prefix (optional)
	CIwGameString			UserKeywords;				// Comma separated list of keywords relevant to this user’s specific session (optional)
	CIwGameString			ExtraInfo;					// Pass in any extra pareneters as name vakue pairs, e.g. &city=london&ad_unit=1 (optional)
	bool					AdAvailable;				// True when ad is available
	CIwGameCallback			AdAvailableCallback;		// Callback to be called when Ad is available
	eIwGameAdsError			Error;						// Comntains error code if any if ad not received
	CIwGameString			ErrorString;				// Human readable error string
	CIwGameAd				AdInfo;						// The returned ad info
	CIwGameAdsMediator*		Mediator;					// Ad mediator (optional)

public:
	void					setVersion(const char* version)		{ Version = version; }
	CIwGameString&			getVersion()						{ return Version; }
	void					setApplicationID(const char* id)	{ ApplicationID = id; }
	CIwGameString&			getApplicationID()					{ return ApplicationID; }
	void					setOtherID(const char* id)			{ OtherID = id; }
	CIwGameString&			getOtherID()						{ return OtherID; }
	void					setUDID(unsigned int udid)			{ UDID = udid; }
	unsigned int			getUDID() const						{ return UDID; }
	void					setSize(int width, int height)		{ Width = width; Height = height; }
	int						getWidth() const					{ return Width; }
	int						getHeight() const					{ return Height; }
	void					setPortalType(eIwGameAdsPortalType type) { PortalType = type; }
	eIwGameAdsPortalType	getPortaltype() const				{ return PortalType; }
	void					setTextAds(bool text_ads)			{ if (TextAds != text_ads) { TextAds = text_ads; PortalType = FindPortalType(TextAds); } }
	bool					getTextAds() const					{ return TextAds; }
	void					setHtmlAds(bool html_ads)			{ HtmlAds = html_ads; }
	bool					getHtmlAds() const					{ return HtmlAds; }
	void					setUserAgent(const char* user_agent) { UserAgent.setString(user_agent); }
	CIwGameString&			getUserAgent() 						{ return UserAgent; }
	void					setUserAge(int age)					{ UserAge = age; }
	int						getUserAge() const					{ return UserAge; }
	void					setUserGender(eIwGameAdsGender gender)	{ UserGender = gender; }
	eIwGameAdsGender		getUserGender() const				{ return UserGender; }
	void					setUserLocation(const char* location) { UserLocation = location; }
	CIwGameString&			getUserLocation()					{ return UserLocation; }
	void					setUserGPSLocation(const char* location) { UserGPSLocation = location; }
	CIwGameString&			getUserGPSLocation()				{ return UserGPSLocation; }
	void					setMobileNumber(const char* number) { UserMobileNumber = number; }
	CIwGameString&			getUserMobileNumber()				{ return UserMobileNumber; }
	void					setUserKeywords(const char* keywords) { UserKeywords = keywords; }
	CIwGameString&			getUserKeywords()					{ return UserKeywords; }
	void					setCategory(const char* category)	{ Category = category; }
	CIwGameString&			getCategory()						{ return Category; }
	void					setExtraInfo(const char* extras)	{ ExtraInfo = extras; }
	CIwGameString&			getExtraInfo()						{ return ExtraInfo; }
	void					setAdAvailable(bool available)		{ AdAvailable = available; }
	bool					isAdAvailable() const				{ return AdAvailable; }
	void					setAdAvailableCallback(CIwGameCallback callback)	{ AdAvailableCallback = callback; }
	void					setError(eIwGameAdsError error)		{ Error = error; }
	eIwGameAdsError			getError() const					{ return Error; }
	void					setErrorString(const char* error)	{ ErrorString.setString(error); }
	CIwGameString&			getErrorString()					{ return ErrorString; }
	CIwGameAd&				getAd()								{ return AdInfo; }
	int						getMaxAds() const					{ return IW_GAME_MAX_CACHED_ADS; }
	void					setMediator(CIwGameAdsMediator* med) { Mediator = med; }
	CIwGameAdsMediator*		getMediator()						{ return Mediator; }

	// Properties end

protected:
	static unsigned int		ResponseCodes[];
	static int				PortalIDs[];

	CIwGameString			ContentType;
	int						OperatingSystem;
	CIwGameString			UserIP;
	CIwGameString			ClientID;
	CIwGameString			RequestURI;
	CIwGameHttpRequest		AdRequest;
	CIwGameTimer			BusyTimer;
	eAdProvider				AdProvider;

	bool					ExtractAd(CIwGameAd& ad, CIwGameString& ad_body);
	void					ErrorFromResponse(const char* error, int error_len);
	bool					RequestBannerImage(CIwGameAd& ad);
	void					NotifyAdAvailable();
	eIwGameImageFormat		GetImageFormatFromHeader();
	bool					RequestAdInnerActive();
	bool					RequestAdInMobi();
	bool					RequestAdMobClix();
	bool					RequestAdMobFox();
	bool					RequestAdAdFonic();
	bool					RequestAdMadvertise();
	bool					RequestAdMojiva();
	bool					RequestAdMillennialMedia();
	bool					RequestAdVServ();
	bool					RequestAdKomliMobile();
	bool					RequestAdAdModa();
	bool					ExtractLinkAndImageFromtHTML(CIwGameAd& ad, CIwGameString& html);
	bool					ExtractAdInnerActive(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdInMobi(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdMobClix(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdMobFox(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdAdFonic(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdMadvertise(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdMojiva(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdMillennialMedia(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdVServ(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdKomliMobile(CIwGameAd& ad, CIwGameString& ad_body);
	bool					ExtractAdAdModa(CIwGameAd& ad, CIwGameString& ad_body);


public:
	bool					Init();												// Initialises the Ads system (returns true if ads are supported)
	void					Release();											// Releases data used by the Ads system
	void					Update();											// Update ads
	bool					RequestAd(eAdProvider provider, bool reset_mediator = true);	// Requests an ad from the specified ad provider
	
	// Utility
	eIwGameAdsPortalType	FindPortalType(bool text_ad = false);				// Auto find portal type from OS type and ad type

	// Internal
	void					AdImageReceived(CIwGameHttpRequest* request, int error);	// Called by the http callback internally when an ad image is received
	void					AdReceived(CIwGameHttpRequest* request, int error);			// Called by the http callback internally when an ad is received

};


#endif	// _IW_GAME_ADS_H_