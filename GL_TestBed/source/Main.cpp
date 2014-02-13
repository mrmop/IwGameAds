// Marmalade headers
#include "s3e.h"
#include "IwGL.h"
#include "GLES\gl.h"

// Game headers
#include "IwGameInput.h"
#include "IwGameAds.h"
#include "IwGameAdsView.h"
#include "IwGameAdsViewAnimator.h"
#include "IwGameAdsMediator.h"

// Uncomment ANIMATE_ADS if you want ahimnating ads
#define ANIMATE_ADS

void AdTest_Init()
{
	// Init http manager
	CIwGameHttpManager::Create();
	IW_GAME_HTTP_MANAGER->Init();

	// Create ad view
	CIwGameAdsView::Create();

	// Initialise with Application ID (yuo get this from your ad provider)
	IW_GAME_ADS_VIEW->Init("");

	// Set ad provider
	IW_GAME_ADS_VIEW->setAdProvider(CIwGameAds::InnerActive);

	// Set ad request interval in seconds
	IW_GAME_ADS_VIEW->setNewAdInterval(30);

	// Force a request for an initial ad
	IW_GAME_ADS_VIEW->RequestNewAd(CIwGameAds::InnerActive);

	// Set total number of ads visible in te ads view
	IW_GAME_ADS_VIEW->setNumAdsVisible(1);

	// Tell animators to loop
	IW_GAME_ADS_VIEW->setLooped(true);

#if !defined(ANIMATE_ADS)
	// Set the ads view position
	IW_GAME_ADS_VIEW->setPosition(0, IwGxGetScreenWidth() / 2, 20);
#elif defined(ANIMATE_ADS)
	// Create and attach an animator that fades the ad in over 1 second, pauses for 7 seconds and then fades the ad back out
	int width = IwGxGetScreenWidth();
	int height = IwGxGetScreenHeight();
	CIwGameAdsViewAnimator* anim = new CIwGameAdsViewAnimator();
	anim->Init();
	anim->setAdViewDataIndex(0);
	anim->setCanvasSize(width, height);
	anim->setInAnim(CIwGameAdsViewAnimator::AnimFadeIn, 1000);
	anim->setOutAnim(CIwGameAdsViewAnimator::AnimFadeOut, 1000);
	anim->setStayDuration(7000);
	IW_GAME_ADS_VIEW->addAnimator(0, anim);

	// Create and attach an animator that sweeps the ad in from the right the over 1,2 seconds, pauses for 7 seconds and then sweeps back out
	anim = new CIwGameAdsViewAnimator();
	anim->Init();
	anim->setAdViewDataIndex(0);
	anim->setCanvasSize(width, height);
	anim->setRestingPosition(0, -height / 8);
	anim->setInAnim(CIwGameAdsViewAnimator::AnimRightSweepIn, 1200);
	anim->setOutAnim(CIwGameAdsViewAnimator::AnimRightSweepOut, 1200);
	anim->setStayDuration(7000);
	anim->setRestingPosition(width / 2, 20);
	IW_GAME_ADS_VIEW->addAnimator(0, anim);

	// Create and attach an animator that scales the ad in over 1.5 seconds, pauses for 7 seconds and then scales back out
	anim = new CIwGameAdsViewAnimator();
	anim->Init();
	anim->setAdViewDataIndex(0);
	anim->setCanvasSize(width, height);
	anim->setInAnim(CIwGameAdsViewAnimator::AnimScaleIn, 1500);
	anim->setOutAnim(CIwGameAdsViewAnimator::AnimScaleOut, 1500);
	anim->setStayDuration(7000);
	IW_GAME_ADS_VIEW->addAnimator(0, anim);

	// Create and attach an animator that rotates the ad in over 1 second, pauses for 7 seconds and then rotates back out
	anim = new CIwGameAdsViewAnimator();
	anim->Init();
	anim->setAdViewDataIndex(0);
	anim->setCanvasSize(width, height);
	anim->setInAnim(CIwGameAdsViewAnimator::AnimSpinIn, 1000);
	anim->setOutAnim(CIwGameAdsViewAnimator::AnimSpinOut, 1000);
	anim->setStayDuration(7000);
	IW_GAME_ADS_VIEW->addAnimator(0, anim);
#endif	// ANIMATE_ADS
}

void AdTest_InitMediator()
{
	// Create ad mediator and attach it to the ads system
	CIwGameAdsMediator* ad_mediator = new CIwGameAdsMediator();
	IW_GAME_ADS->setMediator(ad_mediator);

	// Create inner-active ad party and ad it to the mediator
	CIwGameAdsParty* party = new CIwGameAdsParty();
	party->ApplicationID = "Put your ID here";
	party->Provider = CIwGameAds::InnerActive;
	ad_mediator->addAdParty(party);

	// Create Adfonic ad party and ad it to the mediator
	party = new CIwGameAdsParty();
	party->ApplicationID = "Put your ID here";
	party->Provider = CIwGameAds::AdFonic;
	ad_mediator->addAdParty(party);

	// Create Vserv ad party and ad it to the mediator
	party = new CIwGameAdsParty();
	party->ApplicationID = "Put your ID here";
	party->Provider = CIwGameAds::VServ;
	ad_mediator->addAdParty(party);

	// Create Mojiva ad party and ad it to the mediator
	party = new CIwGameAdsParty();
	party->ApplicationID = "Put your ID here";
	party->Provider = CIwGameAds::Mojiva;
	ad_mediator->addAdParty(party);

	// Create MillennialMedia ad party and ad it to the mediator
	party = new CIwGameAdsParty();
	party->ApplicationID = "Put your ID here";
	party->Provider = CIwGameAds::MillennialMedia;
	ad_mediator->addAdParty(party);

	// Create AdModa ad party and ad it to the mediator
	party = new CIwGameAdsParty();
	party->ApplicationID = "Put your ID here";
	party->Provider = CIwGameAds::AdModa;
	ad_mediator->addAdParty(party);
}

void AdTest_Release()
{
	// Shut down http manager
	IW_GAME_HTTP_MANAGER->Release();
	CIwGameHttpManager::Destroy();

	// Clean up ads system
	IW_GAME_ADS_VIEW->Release();
	CIwGameAdsView::Destroy();
}

void AdTest_Update()
{
	// Update http manager
	if (IW_GAME_HTTP_MANAGER != NULL)
	{
		IW_GAME_HTTP_MANAGER->Update();
	}

	// Update the ads view
	IW_GAME_ADS_VIEW->Update(1.0f);
	IW_GAME_ADS_VIEW->Draw();
}

void AdTest_Show(bool show)
{
	IW_GAME_ADS_VIEW->setVisible(show);
}

int main()
{
	// Initialise Marmalade graphics system and Iw2D module
	IwGxInit();
    Iw2DInit();
//	IwGLInit();

	// Initialise the input system
	CIwGameInput::Create();
	IW_GAME_INPUT->Init();

	// Init ads ssystem
	AdTest_Init();

	// Optionally add an ad mediator if you want to request ads from more than one network
//	AdTest_InitMediator();

	int w = IwGxGetScreenWidth();
	int h = IwGxGetScreenHeight();

	// Main Game Loop
	while (!s3eDeviceCheckQuitRequest())
	{
		// Check for back button quit
		if (IW_GAME_INPUT->isKeyDown(s3eKeyAbsBSK))
			break;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		IwGxClear(IW_GX_COLOUR_BUFFER_F | IW_GX_DEPTH_BUFFER_F);

		// Update input system
		IW_GAME_INPUT->Update();

		// GL render test
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glScalef(1, -1, 1);
		glTranslatef(0, -h, 0);

		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);

		GLfloat vertices[] = {1,0,0, 0,1,0, -1,0,0};
		GLfloat colours[] = {1,0,0,1, 0,1,0,1, 0,0,1,1};
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glScalef(1, -1, 1);
		glTranslatef(0, -h, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &colours);
		glVertexPointer(3, GL_FLOAT, 0, &vertices);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		glMatrixMode(GL_TEXTURE);
		glPopMatrix();

		// Update the ads
		AdTest_Update();

		// Show the surface
		Iw2DSurfaceShow();

		// Yield to the operating system
		s3eDeviceYield(0);
	}

	// Shit down the ads system
	AdTest_Release();

	// Shut down the input system
	IW_GAME_INPUT->Release();
	CIwGameInput::Destroy();

	// Shut down Marmalade graphics system and the Iw2D module
	Iw2DTerminate();
	IwGxTerminate();

    return 0;
}

