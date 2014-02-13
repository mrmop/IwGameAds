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

#include <math.h>

#include "IwGameAdsViewAnimator.h"
#include "IwGameAdsView.h"

void CIwGameAdsViewAnimator::Init()
{
	AnimPhase = AnimPhaseIn;
	InAnim = AnimFadeIn;
	OutAnim = AnimFadeOut;
	StayAnim = AnimNone;
	InDuration = 1000;
	OutDuration = 1000;
	StayDuration = 20000;
	StartColour.r = 0;
	StartColour.g = 0;
	StartColour.b = 0;
	StartColour.a = 0;
	StartPosition.x = 0;
	StartPosition.y = 0;
	StartScale = 1.0f;
	StartAngle = 0;
	TargetColour.r = 0;
	TargetColour.g = 0;
	TargetColour.b = 0;
	TargetColour.a = 0;
	TargetPosition.x = 0;
	TargetPosition.y = 0;
	TargetScale = 1.0f;
	TargetAngle = 0;
	RestingPosition.x = 0;
	RestingPosition.y = 0;
	AdViewDataIndex = 0;
	Looped = false;
}

void CIwGameAdsViewAnimator::Update(float dt)
{
	if (!IW_GAME_ADS_VIEW->isVisible())
		return;

	Width = IwGxGetScreenWidth();
	float screen_scale = ((float)Width * 0.7f) / AdWidth;
//	setRestingPosition((AdWidth * screen_scale) / 2, (AdHeight * screen_scale) / 2);

	switch (AnimPhase)
	{
		case AnimPhaseIn:
			State_PhaseIn();
			break;
		case AnimPhaseOut:
			State_PhaseOut();
			break;
		case AnimPhaseStay:
			State_PhaseStay();
			break;
		case AnimPhaseDone:
			break;
	}
}

void CIwGameAdsViewAnimator::Reset()
{
	switch (InAnim)
	{
	case AnimFadeIn:
		StartColour.r = 0;
		StartColour.g = 0;
		StartColour.b = 0;
		StartColour.a = 0;
		TargetColour.r = 255;
		TargetColour.g = 255;
		TargetColour.b = 255;
		TargetColour.a = 255;
		break;
	case AnimScaleIn:
		StartScale = 0.05f;
		TargetScale = 1.0f;
		break;
	case AnimSpinIn:
		StartAngle = -180.0f;
		TargetAngle = 0.0f;
		break;
	case AnimBottomSweepIn:
		StartPosition.x = 0;
		StartPosition.y = (float)Height;
		TargetPosition.x = 0;
		TargetPosition.y = 0;
		break;
	case AnimTopSweepIn:
		StartPosition.x = 0;
		StartPosition.y = (float)-Height;
		TargetPosition.x = 0;
		TargetPosition.y = 0;
		break;
	case AnimLeftSweepIn:
		StartPosition.x = (float)-Width;
		StartPosition.y = 0;
		TargetPosition.x = 0;
		TargetPosition.y = 0;
		break;
	case AnimRightSweepIn:
		StartPosition.x = (float)Width;
		StartPosition.y = Height / 2.0;
		TargetPosition.x = 0;
		TargetPosition.y = 0;
		break;
	}
	AnimTimer.setDuration(InDuration + OutDuration + StayDuration);
	AnimPhase = AnimPhaseIn;
}

void CIwGameAdsViewAnimator::State_PhaseIn()
{
	if (AnimTimer.hasStopped())
		return;

	int elapsed = (int)AnimTimer.GetElapsedTime();
	float max = (float)((elapsed > InDuration) ? InDuration : elapsed);

	switch (InAnim)
	{
	case AnimFadeIn:
		{
			// Interpolate  values
			int r = StartColour.r + ((TargetColour.r - StartColour.r) * max) / InDuration;
			int g = StartColour.g + ((TargetColour.g - StartColour.g) * max) / InDuration;
			int b = StartColour.b + ((TargetColour.b - StartColour.b) * max) / InDuration;
			int a = StartColour.a + ((TargetColour.a - StartColour.a) * max) / InDuration;
			IW_GAME_ADS_VIEW->setColour(AdViewDataIndex, r, g, b, a);
		}
		break;
	case AnimScaleIn:
		{
			// Interpolate  values
			float scale = StartScale + ((TargetScale - StartScale) * max) / InDuration;
			IW_GAME_ADS_VIEW->setScale(AdViewDataIndex, scale);
		}
		break;
	case AnimSpinIn:
		{
			// Interpolate  values
			float angle = StartAngle + ((TargetAngle - StartAngle) * max) / InDuration;
			IW_GAME_ADS_VIEW->setAngle(AdViewDataIndex, angle);
		}
		break;
	case AnimBottomSweepIn:
	case AnimTopSweepIn:
	case AnimLeftSweepIn:
	case AnimRightSweepIn:
		{
			// Interpolate  values
			float x = StartPosition.x + ((TargetPosition.x - StartPosition.x) * max) / InDuration;
			float y = StartPosition.y + ((TargetPosition.y - StartPosition.y) * max) / InDuration;
			IW_GAME_ADS_VIEW->setPosition(AdViewDataIndex, RestingPosition.x + x, RestingPosition.y + y);
		}
		break;
	}

	// Check for end of in phase
	if (elapsed > InDuration)
	{
		AnimPhase = AnimPhaseStay;
	}

	// Check to see if anim has timed out
	if (AnimTimer.HasTimedOut())
		AnimTimer.Stop();
}

void CIwGameAdsViewAnimator::State_PhaseStay()
{
	if (AnimTimer.hasStopped())
		return;

	int elapsed = (int)AnimTimer.GetElapsedTime() - InDuration;
	int it = 0;

	// Do a quick flash half way through stay phase
	int start_ms = StayDuration / 2;
	int end_ms = start_ms + 2000;
	if (elapsed >= start_ms && elapsed <= end_ms)
	{
		float wobble = sinf(((((elapsed - start_ms) * 720.0f) / 2000.0f) * PI) / 180.0f) * 25.0f;

		switch (StayAnim)
		{
		case AnimWobble:
			{
				IW_GAME_ADS_VIEW->setPosition(AdViewDataIndex, RestingPosition.x + wobble, RestingPosition.y);
			}
			break;
		case AnimScale:
			{
			}
			break;
		}
	}

	// Wait until we reach out phase of animation
	elapsed = (int)AnimTimer.GetElapsedTime();
	if (elapsed >= InDuration + StayDuration)
		AnimPhase = AnimPhaseOut;

	if (AnimTimer.HasTimedOut())
		AnimTimer.Stop();
}

void CIwGameAdsViewAnimator::State_PhaseOut()
{
	if (AnimTimer.hasStopped())
		return;

	int elapsed = (int)AnimTimer.GetElapsedTime() - InDuration - StayDuration;
	float max = (float)((elapsed > OutDuration) ? OutDuration : elapsed);

	switch (OutAnim)
	{
	case AnimFadeOut:
		{
			// Interpolate values
			int r = TargetColour.r - ((TargetColour.r - StartColour.r) * max) / OutDuration;
			int g = TargetColour.g - ((TargetColour.g - StartColour.g) * max) / OutDuration;
			int b = TargetColour.b - ((TargetColour.b - StartColour.b) * max) / OutDuration;
			int a = TargetColour.a - ((TargetColour.a - StartColour.a) * max) / OutDuration;
			IW_GAME_ADS_VIEW->setColour(AdViewDataIndex, r, g, b, a);
		}
		break;
	case AnimScaleOut:
		{
			// Interpolate  values
			float scale = TargetScale - ((TargetScale - StartScale) * max) / OutDuration;
			IW_GAME_ADS_VIEW->setScale(AdViewDataIndex, scale);
		}
		break;
	case AnimSpinOut:
		{
			// Interpolate  values
			float angle = TargetAngle - ((TargetAngle - StartAngle) * max) / OutDuration;
			IW_GAME_ADS_VIEW->setAngle(AdViewDataIndex, angle);
		}
		break;
	case AnimBottomSweepOut:
	case AnimTopSweepOut:
	case AnimLeftSweepOut:
	case AnimRightSweepOut:
		{
			// Interpolate  values
			float x = TargetPosition.x - ((TargetPosition.x - StartPosition.x) * max) / OutDuration;
			float y = TargetPosition.y - ((TargetPosition.y - StartPosition.y) * max) / OutDuration;
			IW_GAME_ADS_VIEW->setPosition(AdViewDataIndex, RestingPosition.x + x, RestingPosition.y + y);
		}
		break;
	}

	// Check to see if anim has timed out
	if (elapsed > OutDuration || AnimTimer.HasTimedOut())
	{
		AnimPhase = AnimPhaseDone;
		AnimTimer.Stop();
		if (Looped)
		{
			Reset();
		}
	}
}
