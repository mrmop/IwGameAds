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

#if !defined(_IW_GAME_ADS_VIEW_ANIMATOR_H_)
#define _IW_GAME_ADS_VIEW_ANIMATOR_H_

#include "IwGeom.h"
#include "IwColour.h"
#include "IwGameUtil.h"

//
//
// CIwGameAdsViewAnimator - The IwGameAdsViewAnimator class is responsible for animating a CIwGameAdsView
//
//
class CIwGameAdsViewAnimator
{
public:
	enum eIwGameAdsAnimIn
	{
		AnimFadeIn,				// Fades in
		AnimScaleIn,			// Scales in
		AnimSpinIn,				// Spins in
		AnimBottomSweepIn,		// Sweeps in from bottom
		AnimTopSweepIn,			// Sweeps in from bottom
		AnimLeftSweepIn,		// Sweeps in from left
		AnimRightSweepIn,		// Sweeps in from right
	};
	enum eIwGameAdsAnimOut
	{
		AnimFadeOut,			// Fades in
		AnimScaleOut,			// Scales in
		AnimSpinOut,			// Spins in
		AnimBottomSweepOut,		// Sweeps in from bottom
		AnimTopSweepOut,		// Sweeps in from bottom
		AnimLeftSweepOut,		// Sweeps in from left
		AnimRightSweepOut,		// Sweeps in from right
	};
	enum eIwGameAdsAnimStay
	{
		AnimNone,				// No anim
		AnimWobble,				// Wobble
		AnimScale,				// Scale
	};
	enum eIwGameAdsAnimPhase
	{
		AnimPhaseIn,			// Animation phase in
		AnimPhaseStay,			// Animation phase stay
		AnimPhaseOut,			// Animation phase out
		AnimPhaseDone,			// Animation done
	};

	// Properties
protected:
	CIwFVec2					RestingPosition;	// resting position where ad should stop
	eIwGameAdsAnimIn			InAnim;				// Animation to show the ad
	eIwGameAdsAnimOut			OutAnim;			// Animation to hide the ad
	eIwGameAdsAnimStay			StayAnim;			// Animation shilst ad is visible
	int							InDuration;			// Amount of time to spend bringing the ad in
	int							OutDuration;		// Amount of time to spend sending the ad out
	int							StayDuration;		// Amount of time to ad shoud stay on screen
	int							Width, Height;		// Canvas width and height
	int							AdWidth, AdHeight;	// Ad width and height
	int							AdViewDataIndex;	// The index of this animations parent ad view data object in the ad view
	bool						Looped;				// if true then animation will loop continually
public:
	void						setRestingPosition(int x, int y)						{ RestingPosition.x = x; RestingPosition.y = y; }
	void						setInAnim(eIwGameAdsAnimIn anim, int duration_ms)		{ InAnim = anim; InDuration = duration_ms; Reset(); }
	void						setOutAnim(eIwGameAdsAnimOut anim, int duration_ms)		{ OutAnim = anim; OutDuration = duration_ms; }
	void						setStayAnim(eIwGameAdsAnimStay anim)					{ StayAnim = anim; }
	void						setStayDuration(int duration_ms)						{ StayDuration = duration_ms; }
	void						setCanvasSize(int width, int height)					{ Width = width; Height = height; }
	void						setAdSize(int width, int height)						{ AdWidth = width; AdHeight = height; }
	void						setAdViewDataIndex(int index)							{ AdViewDataIndex = index; }
	void						setLooped(bool looped)									{ Looped = looped; }
	eIwGameAdsAnimPhase			getPhase() const										{ return AnimPhase; }
	// Properties end

protected:
	CIwFVec2					TargetPosition;
	float						TargetAngle;
	float						TargetScale;
	CIwColour					TargetColour;
	CIwFVec2					StartPosition;
	float						StartAngle;
	float						StartScale;
	CIwColour					StartColour;
	eIwGameAdsAnimPhase			AnimPhase;
	CIwGameTimer				AnimTimer;
	void						State_PhaseIn();
	void						State_PhaseOut();
	void						State_PhaseStay();

public:
	CIwGameAdsViewAnimator() 	{}
	~CIwGameAdsViewAnimator()	{}
	virtual void				Init();
	virtual void				Update(float dt);	// Updates the anim view manager
	virtual void				Reset();			// Reset the animation
	
	// Utility
};



#endif	// _IW_GAME_ADS_VIEW_ANIMATOR_H_