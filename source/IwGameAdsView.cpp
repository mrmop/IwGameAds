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

#include "IwGameAdsView.h"
#include "IwGameFile.h"
#include "IwGameInput.h"
#include "IwGameImage.h"
#include "IwGameSprite.h"
#include "IwGameUtil.h"
#include "IwGameAdsViewAnimator.h"

#include "s3eOSExec.h"

CDECLARE_SINGLETON(CIwGameAdsView)

CIwGameAdsViewData::CIwGameAdsViewData()
{
	Tapped = false;
	Position.x = 0;
	Position.y = 0;
	Scale = 1.0f;
	Angle = 0;
	Visible = false;
	Error = CIwGameAds::ErrorInvalidImage;
	Colour.r = 0xff;
	Colour.g = 0xff;
	Colour.b = 0xff;
	Colour.a = 0xff;
	Ad = NULL;
	AdSprite = new CIwGameBitmapSprite();
	AdSprite->Init();
}

CIwGameAdsViewData::~CIwGameAdsViewData()
{
	SAFE_DELETE(Ad);
	for (CIwList<CIwGameAdsViewAnimator*>::iterator it = Animators.begin(); it != Animators.end(); ++it)
	{
		delete *it;
	}
	Animators.clear();

	SAFE_DELETE(AdSprite)
}

CIwGameAdsViewData* CIwGameAdsView::getOldestAdSlot()
{
	if (NumAdsVisible == 1)
		return &AdData[0];

	CIwGameAdsViewData* oldest = NULL;

	// Only check visible ads
	for (int t = 0; t < NumAdsVisible; t++)
	{
		if (AdData[t].Ad == NULL)
		{
			// Ad slot not even used so return this one
			oldest = &AdData[t];
			break;
		}
		else
		{
			// Find oldest ad slot
			if (oldest == NULL)
			{
				oldest = &AdData[t];
			}
			else
			{
				if (AdData[t].Ad->AdTime < oldest->Ad->AdTime)
				{
					oldest = &AdData[t];
				}
			}
		}
	}

	return oldest;
}

void CIwGameAdsView::setNewAdInterval(int interval)
{
	if (interval == 0)
	{
		NewAdTimer.Stop();
		return;
	}

	if (interval < 5)
		interval = 5;
	NewAdInterval = interval;

	NewAdTimer.setDuration(NewAdInterval * 1000);
}
	
bool CIwGameAdsView::Init(const char* id)
{
	MinError = CIwGameAds::ErrorHouseAd;
	PrevTappedAd = NULL;
	NumAdsVisible = 1;
	Visible = true;

	CIwGameAds::Create();
	IW_GAME_ADS->Init();
	IW_GAME_ADS->setApplicationID(id);

	return true;
}

void CIwGameAdsView::Release()
{
	IW_GAME_ADS->Release();
	CIwGameAds::Destroy();
}

void CIwGameAdsView::Update(float dt)
{
	if (!Visible)
		return;

	// Update the ads system
	IW_GAME_ADS->Update();

	// Check to see if new ad has arrived
	CIwGameAdsViewData* data = getOldestAdSlot();
	if (IW_GAME_ADS->isAdAvailable())
	{
		CIwGameAds::eIwGameAdsError error = IW_GAME_ADS->getError();
		if (IW_GAME_ADS->getError() <= MinError)
		{
			// No error so update oldest ad data slot with new ad data
			SAFE_DELETE(data->Ad)
			data->Ad = IW_GAME_ADS->getAd().getCopy();
			if (data->Ad != NULL)
			{
				data->AdSprite->setImage(data->Ad->Image);
				data->AdSprite->setDestSize(data->Ad->Image->getWidth(), data->Ad->Image->getHeight());
				data->AdSprite->setSrcRect(0, 0, data->Ad->Image->getWidth(), data->Ad->Image->getHeight());
				data->Error = error;
				ResetAnims(data);
			}
		}
		else
		{
			// We ignore any faulty ad requests
		}
		IW_GAME_ADS->setAdAvailable(false);	// Allow the next ad request
	}
	else
	{
	}

	// Process a tapped ad
	int num_touches = IW_GAME_INPUT->getTouchCount();
	if (PrevTappedAd != NULL && num_touches == 0)
	{
		if (PrevTappedAd->AdSprite->HitTest(IW_GAME_INPUT->getTouch(0)->x, IW_GAME_INPUT->getTouch(0)->y))
		{
			// Launch the ad in the web browser
			if (PrevTappedAd->Ad != NULL)
				s3eOSExecExecute(PrevTappedAd->Ad->LinkURI.c_str(), false);
		}
		PrevTappedAd->Colour.a = 0xff;
		PrevTappedAd->Tapped = false;
		PrevTappedAd = NULL;
	}
	
	// Process ad view data
	int64 current_time = s3eTimerGetMs();
	for (int t = 0; t < NumAdsVisible; t++)
	{
		data = &AdData[t];

		// Mark any old or errored ads as invisible
		if (data->Ad != NULL)
		{
			int64 ad_age = current_time - data->Ad->AdTime;
			if (data->Error > MinError || ad_age >= IW_GAME_MAX_AD_AGE)
			{
				data->Visible = false;
			}
			else
			{
				data->Visible = true;
				if (PrevTappedAd == NULL)
				{
					// Check for tap inside area of ad
					if (num_touches != 0)
					{
						data->Tapped = true;
						if (data->AdSprite->HitTest(IW_GAME_INPUT->getTouch(0)->x, IW_GAME_INPUT->getTouch(0)->y))
						{
							data->Colour.a = 0x7f;
							PrevTappedAd = data;
						}
					}
				}

				// Update attached animators
				int total_phases_done = 0;
				for (CIwList<CIwGameAdsViewAnimator*>::iterator it = data->Animators.begin(); it != data->Animators.end(); ++it)
				{
					(*it)->Update(dt);
					if (Looped)
					{
						if ((*it)->getPhase() == CIwGameAdsViewAnimator::AnimPhaseDone)
						{
							total_phases_done++;
						}
					}
				}

				// If animation is looped then reset if done
				if (Looped)
				{
					if (data->Animators.size() == total_phases_done)
					{
						ResetAnims(data);
					}
				}
			}
		}
		else
		{
			data->Visible = false;
		}
	}

	// Request a new ad
	if (NewAdTimer.hasStarted())
	{
		if (NewAdTimer.HasTimedOut())
		{
			IW_GAME_ADS->RequestAd(AdProvider);
			NewAdTimer.setDuration(NewAdInterval * 1000);
		}
	}
}

void CIwGameAdsView::Draw()
{
	if (!Visible)
		return;

	for (int t = 0; t < NumAdsVisible; t++)
	{
		CIwGameAdsViewData* data = &AdData[t];

		data->AdSprite->setVisible(data->Visible);
		if (data->Visible)
		{
			data->AdSprite->setPosition(data->Position.x, data->Position.y);
			data->AdSprite->setScale(data->Scale);
			data->AdSprite->setAngle(data->Angle);
			data->AdSprite->setColour(data->Colour);
			data->AdSprite->Draw();
		}
	}
}

void CIwGameAdsView::RequestNewAd(CIwGameAds::eAdProvider ad_provider, bool text_ads)
{
	if (!Visible)
		return;

	IW_GAME_ADS->setTextAds(text_ads);
	IW_GAME_ADS->RequestAd(ad_provider);
}

void CIwGameAdsView::ResetAnims(CIwGameAdsViewData* data)
{
	for (CIwList<CIwGameAdsViewAnimator*>::iterator it = data->Animators.begin(); it != data->Animators.end(); ++it)
	{
		(*it)->setAdSize(data->AdSprite->getSrcWidth(), data->AdSprite->getSrcHeight());
		(*it)->Reset();
	}
}

void CIwGameAdsView::ResetAllAnims()
{
	for (int t = 0; t < IW_GAME_MAX_CACHED_ADS; t++)
	{
		if (AdData[t].Ad != NULL)
		{
			ResetAnims(&AdData[t]);
		}
	}
}









