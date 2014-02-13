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

#if !defined(_IW_GAME_ADS_VIEW_H_)
#define _IW_GAME_ADS_VIEW_H_

#include "IwGeom.h"
#include "IwGameUtil.h"
#include "IwGameHttp.h"
#include "IwGameImage.h"
#include "IwGameSprite.h"
#include "IwGameAds.h"

#define	IW_GAME_ADS_VIEW		(CIwGameAdsView::getInstance())

class CIwGameAdsViewAnimator;

//
//
// CIwGameAdsViewData - The IwGameAdsView strucryre contains data used to display a single ad uhit
//
//
struct CIwGameAdsViewData
{
	CIwFVec2					Position;			// Position of ad on screem
	float						Scale;				// Scale of the ad
	float						Angle;				// Scale of the ad
	bool						Visible;			// Ad visibility
	CIwColour					Colour;				// Colour of ad
	CIwList<CIwGameAdsViewAnimator*> Animators;		// Attached animators will animate the ad
	CIwGameAds::eIwGameAdsError	Error;				// Contains error code if any if ad not received

	CIwGameAd*					Ad;					// Ad that this view should draw
	CIwGameBitmapSprite*		AdSprite;			// Sprite used by the ad view to display the ad
	bool						Tapped;
	CIwFVec2					tv[4];				// Transformed vertices

	CIwGameAdsViewData();
	~CIwGameAdsViewData();
};

//
//
// CIwGameAdsView - The IwGameAdsView class is responsible for collecting ads, rendering them, detecting clicks and launching the ad URL
//
//
class CIwGameAdsView
{
public:
	CDEFINE_SINGLETON(CIwGameAdsView)

	// Properties
protected:
	int							NewAdInterval;		// The number of seconds between collecting new ads (minimum is 5 seconds)
	CIwGameAds::eIwGameAdsError	MinError;			// Minimum allowed error before displaying an ad
	CIwGameAdsViewData			AdData[IW_GAME_MAX_CACHED_ADS];
	int							NumAdsVisible;		// number of ads visible
	bool						Visible;			// Ad visibility
	bool						Looped;				// Loops animations if true
	CIwGameAds::eAdProvider		AdProvider;			// Ad provider used for automated ad collection
public:
	void						setVisible(bool visible)					{ Visible = visible; }
	bool						isVisible() const							{ return Visible; }
	void						setVisible(int index, bool visible)			{ AdData[index].Visible = visible; }
	bool						isVisible(int index) const					{ return AdData[index].Visible; }
	void						setPosition(int index, float x, float y)	{ AdData[index].Position.x = x; AdData[index].Position.y = y; }
	CIwFVec2&					getPosition(int index)						{ return AdData[index].Position; }
	void						setScale(int index, float scale)			{ AdData[index].Scale = scale; }
	float						getScale(int index) const					{ return AdData[index].Scale; }
	void						setAngle(int index, float angle)			{ AdData[index].Angle = angle; }
	float						getAngle(int index) const					{ return AdData[index].Angle; }
	void						setColour(int index, int r, int g, int b, int a) { AdData[index].Colour.r = r; AdData[index].Colour.g = g; AdData[index].Colour.b = b; AdData[index].Colour.a = a;}
	void						addAnimator(int index, CIwGameAdsViewAnimator* animator)	{ AdData[index].Animators.push_back(animator); }
	void						setNumAdsVisible(int count)					{ NumAdsVisible = count; }
	int							getNumAdsVisible() const					{ return NumAdsVisible; }
	void						setAllowHouseAds(bool allow)				{ if (allow) MinError = CIwGameAds::ErrorHouseAd; else MinError = CIwGameAds::ErrorNone; }
	void						setNewAdInterval(int interval);	
	CIwGameAdsViewData*			getAdViewData(int index)					{ return &AdData[index]; }
	void						setLooped(bool looped)						{ Looped = looped; }
	void						setAdProvider(CIwGameAds::eAdProvider ad_provider)	{ AdProvider = ad_provider; }

	// Properties end

protected:
	bool						PreviousTouch;
	CIwGameAdsViewData*			PrevTappedAd;
	CIwGameTimer				NewAdTimer;				// Used to time new ad collection

	void						Transform();
	CIwGameAdsViewData*			getOldestAdSlot();

public:
	virtual bool				Init(const char* id);								// Initialises the Ads view system (returns true if ads are supported)
	virtual void				Release();											// Releases data used by the Ads view system
	virtual void				Update(float dt = 1.0f);							// Updates the Ads view system, called every frame
	virtual void				Draw();												// Renders the ads view
	
	// Utility
	void						RequestNewAd(CIwGameAds::eAdProvider ad_provider, bool text_ads = false);	// Forcesthe collection of a new ad
	void						ResetAllAnims();									// Resets all ad slot animations
	void						ResetAnims(CIwGameAdsViewData* data);				// Resets ad slot animations

	// Internal

};



#endif	// _IW_GAME_ADS_VIEW_H_