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

#if !defined(_IW_GAME_ADS_MEDIATOR_H_)
#define _IW_GAME_ADS_MEDIATOR_H_

#include "IwGeom.h"
#include "IwGameUtil.h"
#include "IwGameAds.h"

//
//
// CIwGameAdParty - The CIwGameAdParty structure represents a single ad mediation party which can request ads
//
//
struct CIwGameAdsParty
{
	CIwGameAds::eAdProvider	Provider;					// Ad provider
	CIwGameString			ApplicationID;				// ID of the application thats making the request (you will need to be assigned this from inner-active)
	CIwGameString			OtherID;					// Extra ID information
	CIwGameString			ExtraInfo;					// Pass in any extra pareneters as name vakue pairs, e.g. &city=london&ad_unit=1 (optional)

	CIwGameAdsParty() {}
	~CIwGameAdsParty()
	{
	}
};

//
//
// CIwGameAdsMediator - The CIwGameAdsMediator class is responsible for mediating ad requests between different ad providers to help impriove fill rates and monetisation
//
//
class CIwGameAdsMediator
{
public:
	// Public access for scene iteration
	typedef CIwList<CIwGameAdsParty*>::iterator	_Iterator;
	_Iterator				begin() { return AdParties.begin(); }
	_Iterator				end() { return AdParties.end(); }

	// Properties
protected:
	CIwList<CIwGameAdsParty*>	AdParties;
	int							NextAdParty;

public:
	void					addAdParty(CIwGameAdsParty* party)
	{
		AdParties.push_back(party);
	}
	void					removeAdParty(CIwGameAdsParty* party)
	{
		for (_Iterator it = AdParties.begin(); it != AdParties.end(); ++it)
		{
			if ((*it) == party)
			{
				delete *it;
				AdParties.erase(it);
				break;
			}
		}
	}
	void					clearParties()
	{
		for (_Iterator it = AdParties.begin(); it != AdParties.end(); ++it)
			delete *it;
		AdParties.clear();
	}
	CIwGameAdsParty*		getNextAdParty()
	{
		int max = AdParties.size();
		if (max == 0)
			return NULL;

		// If we ran out of ad partys then start again from first
		if (NextAdParty >= max)
			NextAdParty = 0;

		return AdParties.element_at(NextAdParty++);
	}
	void					reset()
	{
		NextAdParty = 0;
	}

	// Properties end

protected:


public:
	CIwGameAdsMediator() : NextAdParty(0)	{}
	~CIwGameAdsMediator()
	{
		clearParties();
	}
	
};


#endif	// _IW_GAME_ADS_MEDIATOR_H_