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

#if !defined(_IW_GAME_HTTP_H_)
#define _IW_GAME_HTTP_H_

#include <IwList.h> 
#include "s3e.h"
#include "IwHTTP.h"

#include "IwGameUtil.h"
#include "IwGameString.h"

#define IW_GAME_HTTP_MANAGER		CIwGameHttpManager::getInstance()

class CIwGameHttpManager;

//
//
// CIwGameHttpHeader - An http header
// 
//
class CIwGameHttpHeader
{
	// Properties
private:
	CIwGameString		Header;			// Header name
	CIwGameString		HeaderData;		// header data
public:
	void		setHeader(const char* header)
	{
		Header.setString(header);
	}
	CIwGameString&		getHeader()		{ return Header; }
	void				setHeaderData(const char* header_data)
	{
		HeaderData.setString(header_data);
	}
	const char*			getHeaderData() const { return HeaderData.c_str(); }
	// Properties end
public:
	CIwGameHttpHeader()
	{
		Header.setAutoHash(true);
	}
	~CIwGameHttpHeader()
	{
	}
};
typedef CIwList<CIwGameHttpHeader*>::iterator CIwGameHttpHeaderIterator;

//
//
// CIwGameHttpPostData - http POST data
// 
//
class CIwGameHttpPostData
{
private:
	CIwList<CIwGameHttpHeader*>	Headers;		// Headers collection
	CIwGameString				Body;			// POST body
public:
	CIwGameHttpPostData() {}
	~CIwGameHttpPostData()
	{
		ClearHeaders();
	}

	void SetHeader(const char* header, const char* header_data)
	{
		unsigned int header_hash = IwHashString(header);

		// Check to see if header already present, if so update the header info
		for (CIwGameHttpHeaderIterator it = Headers.begin(); it != Headers.end(); it++)
		{
			if (header_hash == (*it)->getHeader().getHash())
			{
				(*it)->setHeaderData(header_data);
				return;
			}
		}

		// Header was not already present so add new header
		CIwGameHttpHeader* head = new CIwGameHttpHeader();
		head->setHeader(header);
		head->setHeaderData(header_data);
		Headers.push_back(head);
	}
	void ClearHeaders()
	{
		for (CIwGameHttpHeaderIterator it = Headers.begin(); it != Headers.end();  ++it)
			delete (*it);
		Headers.clear();
	}
	void setBody(const char* body)
	{
		Body.setString(body);
	}
	const CIwGameString& getBody() const { return Body; }

	void ApplyHeaders(CIwHTTP* http_object)
	{
		for (CIwGameHttpHeaderIterator it = Headers.begin(); it != Headers.end(); ++it)
		{
			http_object->SetRequestHeader((*it)->getHeader().c_str(), (*it)->getHeaderData());
		}
	}
};

//
//
// CIwGameHttpRequest - An http request including the header, body and response
//
//
class CIwGameHttpRequest
{
	// Properties
private:
	CIwGameString				URI;			// URI end point
	CIwGameString				Content;		// Receieved content
	int							ContentLength;	// Received content lenngth
	bool						Processed;		// Processed marker (becomes true when Http Manager has processed the request, this does not mean result is available)
	int							Error;			// Any errors that occured or 0 for none
	CIwGameHttpPostData*		PostData;		// Collection of headers and the main body to POST
	CIwGameCallback				ContentAvailableCallback;		// Callback to be called when content is available
	void*						ContentAvailableCallbackData;	// Data to be passed to the callback
	bool						Post;			// True if request is a POST, otherwise a GET
public:
	const CIwGameString&		getURI() const							{ return URI; }
	void						setURI(const char* uri)
	{
		URI.setString(uri);
	}
	CIwGameString&				getContent()							{ return Content; }
	void						setContent(char* content, int len)
	{
		Content.allocString(len);
		Content.Copy(content, 0, len);
		ContentLength = len;
	}
	void						allocContent(int len)
	{
		Content.allocString(len);
		Content.setLength(len);
		ContentLength = len;
	}
	void						reallocContent(int new_len)
	{
		Content.reallocString(new_len);
		Content.setLength(new_len);
		ContentLength = new_len;
	}
	int							getContentLength() const							{ return ContentLength; }
	bool						getProcessed() const								{ return Processed; }
	void						setProcessed(bool processed)						{ Processed = processed; }
	void						SetHeader(const char* header, const char* header_data) { PostData->SetHeader(header, header_data); }
	void						ClearHeaders()										{ PostData->ClearHeaders(); }
	void						setBody(const char* body)							{ PostData->setBody(body); }
	const CIwGameString&		getBody() const										{ return PostData->getBody(); }
	void						setContentAvailableCallback(CIwGameCallback callback, void *data)	{ ContentAvailableCallback = callback; ContentAvailableCallbackData = data; }
	void						setPOST()											{ Post = true; }
	void						setGET()											{ Post = false; }
	bool						isPOST() const										{ return Post; }
	// Properties end

private:

public:
	CIwGameHttpRequest() : ContentLength(0), Processed(false), Error(0), ContentAvailableCallback(NULL), ContentAvailableCallbackData(NULL), Post(false) { PostData = new CIwGameHttpPostData(); }
	~CIwGameHttpRequest()
	{
		SAFE_DELETE(PostData)
	}

	void setError(int error)				{ Error = error; }
	int getError() const					{ return Error; }	

	void EndRequest(int error);
	void ApplyHeaders(CIwHTTP* http_object) { PostData->ApplyHeaders(http_object); }

};

//
//
// CIwGameHttpManager - Handles the queueing of http requests
//
//
class CIwGameHttpManager
{
	CDEFINE_SINGLETON(CIwGameHttpManager)

protected:
	typedef CIwList<CIwGameHttpRequest*>::iterator _Iterator;
	CIwList<CIwGameHttpRequest*> Requests;		// Request queue (caller owns requests)

	// Properties
	CIwGameHttpRequest*			CurrentRequest;	// Current request thats being processed or null if not busy
	CIwHTTP*					HttpObject;		// The Marmalade SDK Http Object
	CIwGameString				UserAgent;		// Browser style user-agent
	CIwGameString				IPAddress;		// IP address of device
public:
	CIwGameHttpRequest*			getCurrentRequest()						{ return CurrentRequest; }
	void						setNoRequest()							{ CurrentRequest = NULL; }
	bool						GetHeader(const char* header_name, CIwGameString* header_data);
	CIwHTTP*					getHttpObject()							{ return HttpObject; }
	void						setUserAgent(const char* user_agent)	{ UserAgent.setString(user_agent); }	// Aoto determined but can be changed if needed
	CIwGameString&				getUserAgent()							{ return UserAgent; }
	void						setIPAddress(const char* ip_address)	{ IPAddress.setString(ip_address); }	// Aoto determined but can be changed if needed
	CIwGameString&				getIPAddress()							{ return IPAddress; }
	// Properties end

protected:
	void					DetermineUserAgent();
	bool					DetermineIPAddress();
public:
	void					Init();
	void					Release();
	void					AddRequest(CIwGameHttpRequest* request)
	{
		Requests.push_back(request);
		request->setProcessed(false);
	}
	void					RemoveRequest(CIwGameHttpRequest* request)
	{
		for (_Iterator it = Requests.begin(); it != Requests.end(); it++)
		{
			if (*it == request)
			{
				Requests.erase(it);
				break;
			}
		}
	}
	void					ClearRequests()
	{
		Requests.clear();
	}
	void					CancelRequests()
	{
		ClearRequests();
		if (HttpObject != NULL)
			HttpObject->Cancel();
	}
	
	s3eResult				GetStatus() const			{ return HttpObject->GetStatus(); }
	int						GetContentReceived() const	{ return HttpObject->ContentReceived(); }
	int						GetContentLength() const	{ return HttpObject->ContentLength(); }
	int						GetContentExpected() const	{ return HttpObject->ContentExpected(); }
//	int						ReadContent(char *buf, int max_bytes, s3eCallback callback = NULL, void *cb_data = NULL) { return HttpObject->ReadContent(buf, max_bytes, callback, cb_data); }
	int						ReadContent(char *buf, int max_bytes, s3eCallback callback = NULL, void *cb_data = NULL) { HttpObject->ReadDataAsync(buf, max_bytes, 10000, callback); return 0; }
	void					Update();
	bool					IsEmpty() const				{ return Requests.size() > 0; }
};


//
// Testing
//

#endif // _IW_GAME_HTTP_H_