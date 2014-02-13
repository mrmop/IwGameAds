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

#include "IwGameHttp.h"
#include "IwGx.h"
#include "s3eSocket.h"
#include "UserAgent.h"

// ** IMPORTANT NOTE - Change the ping domain to your own domain
// ** IMPORTANT NOTE - Change the ping domain to your own domain
// ** IMPORTANT NOTE - Change the ping domain to your own domain
#define PING_DOMAIN	"ping.pocketeers.co.uk"

//
//
//
//
// CIwGameHttpRequest - implementation
//
//
//
//
void CIwGameHttpRequest::EndRequest(int error)
{
	if (IW_GAME_HTTP_MANAGER == NULL)
		return;

	Error = error;
	IW_GAME_HTTP_MANAGER->setNoRequest();
	Processed = true;

	if (ContentAvailableCallback != NULL)
		ContentAvailableCallback((void*)this, ContentAvailableCallbackData);
}

//
//
//
//
// CIwGameHttpManager - implementation
//
//
//
//

CDECLARE_SINGLETON(CIwGameHttpManager)

void CIwGameHttpManager::Init()
{
	HttpObject = new CIwHTTP;

	CurrentRequest = NULL;

	// Generate user-agent and IP address
	DetermineUserAgent();
	DetermineIPAddress();
}

void CIwGameHttpManager::Release()
{
	if (HttpObject != NULL)
		HttpObject->Cancel();

//	s3eDeviceYield(1000);

	Requests.clear();

	CurrentRequest = NULL;
	SAFE_DELETE(HttpObject)
}

static int32 GotData(void*, void*)
{
    // This is the callback indicating that a ReadContent call has completed.  Either we've finished, or a bigger buffer is needed.  If the correct ammount of data was supplied initially,
	// then this will only be called once. However, it may well be called several times when using chunked encoding.
	if (IW_GAME_HTTP_MANAGER == NULL)
		return 0;
	CIwGameHttpRequest* req = IW_GAME_HTTP_MANAGER->getCurrentRequest();
	if (req == NULL)
		return 0;

    // Firstly see if there's an error condition.
    if (IW_GAME_HTTP_MANAGER->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
		req->EndRequest(-3);
		return -1;
    }
	else
	if (IW_GAME_HTTP_MANAGER->GetContentReceived() != IW_GAME_HTTP_MANAGER->GetContentLength())
	{
		// We have some data but not all of it. We need more space.
		int len = req->getContentLength();
		int old_len = len;

		// If iwhttp has a guess how big the next bit of data is (this basically means chunked encoding is being used), allocate that much space. Otherwise guess.
		if (len < IW_GAME_HTTP_MANAGER->GetContentExpected())
			len = IW_GAME_HTTP_MANAGER->GetContentExpected();
		else
			len += 1024;

        // Allocate some more space and fetch the data.
		req->reallocContent(len);
		if (IW_GAME_HTTP_MANAGER != NULL)
			IW_GAME_HTTP_MANAGER->ReadContent(req->getContent().str() + old_len, len - old_len, GotData);
	}
    else
    {
        // All data rerteived
		req->EndRequest(0);
    }

	return 0;
}

//
// Called when the response headers have been received
//
static int32 GotHeaders(void*, void*)
{
	if (IW_GAME_HTTP_MANAGER == NULL)
		return 0;
	CIwGameHttpRequest* req = IW_GAME_HTTP_MANAGER->getCurrentRequest();
	if (req == NULL)
		return 0;

    if (IW_GAME_HTTP_MANAGER->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
		req->EndRequest(-2);
		return -1;
    }
    else
    {
		req->setError(0);

        // Depending on how the server is communicating the content length, we may actually know the length of the content, or we may know the length of the first part of it, or we may
        // know nothing. ContentExpected always returns the smallest possible size of the content, so allocate that much space for now if it's non-zero. If it is of zero size, the server
        // has given no indication, so we need to guess. We'll guess at 1k.
		int len = IW_GAME_HTTP_MANAGER->GetContentExpected();
		if (len <= 0)
		{
			len = 1024;
		}

		req->allocContent(len);
		if (IW_GAME_HTTP_MANAGER != NULL)
			IW_GAME_HTTP_MANAGER->ReadContent(req->getContent().str(), req->getContentLength(), GotData, NULL);
    }

	return 0;
}

bool CIwGameHttpManager::GetHeader(const char* header_name, CIwGameString* header_data)
{
	std::string type;
	if (!IW_GAME_HTTP_MANAGER->getHttpObject()->GetHeader(header_name, type))
		return false;

	header_data->setString(type.c_str());

	return true;
}

void CIwGameHttpManager::Update()
{
	typedef CIwList<CIwGameHttpRequest*>::iterator _iterator;

	if (CurrentRequest == NULL)
	{
		for(_iterator it = Requests.begin(); it != Requests.end(); it++)
		{
			CIwGameHttpRequest* req = *it;

			// If request has not been processed then process it
			if (!req->getProcessed())
			{
				CurrentRequest = req;

//				if (req->getBody().c_str() == NULL)
				if (!req->isPOST())
				{
					// Process the GET request
					req->ApplyHeaders(HttpObject);
					if (HttpObject->Get(req->getURI().c_str(), GotHeaders, NULL) != S3E_RESULT_SUCCESS)
					{
						// Failed so set error and free up manager to process another request
						req->EndRequest(-1);
						break;
					}
				}
				else
				{
					// Process the POST request
					req->ApplyHeaders(HttpObject);
					if (HttpObject->Post(req->getURI().c_str(), req->getBody().c_str(), req->getBody().GetLength(), GotHeaders, NULL) != S3E_RESULT_SUCCESS)
					{
						// Failed so set error and free up manager to process another request
						req->EndRequest(-1);
						break;
					}
				}
				break;
			}
		}
	}
}

void CIwGameHttpManager::DetermineUserAgent()
{
	int width = IwGxGetScreenWidth();
	int height = IwGxGetScreenHeight();
	int os = s3eDeviceGetInt(S3E_DEVICE_OS);

	CIwGameError::LogError("device_os = ", s3eDeviceGetString(S3E_DEVICE_OS));
	CIwGameError::LogError("device_class = ", s3eDeviceGetString(S3E_DEVICE_CLASS));
	CIwGameError::LogError("device_id = ", s3eDeviceGetString(S3E_DEVICE_ID));
	CIwGameError::LogError("device_architecture = ", s3eDeviceGetString(S3E_DEVICE_ARCHITECTURE));
	CIwGameError::LogError("device_os_version = ", s3eDeviceGetString(S3E_DEVICE_OS_VERSION));
	CIwGameError::LogError("device_chipset = ", s3eDeviceGetString(S3E_DEVICE_CHIPSET));
	CIwGameError::LogError("device_sdk_version = ", s3eDeviceGetString(S3E_DEVICE_SDK_VERSION));
	CIwGameError::LogError("device_locale = ", s3eDeviceGetString(S3E_DEVICE_LOCALE));
	CIwGameError::LogError("device_name = ", s3eDeviceGetString(S3E_DEVICE_NAME));

	if (UserAgentAvailable())
	{
		UserAgent = ::getUserAgent();	// Get user-agent from extension
	}

	if (UserAgent.IsEmpty())
	{
		CIwGameString device_id(s3eDeviceGetString(S3E_DEVICE_ID));
		CIwGameString locale(s3eDeviceGetString(S3E_DEVICE_LOCALE));

		// Format locale string
		locale.ToLower();
		locale.Replace('_', '-');

		switch (os)
		{
		case S3E_OS_ID_SYMBIAN:
			UserAgent = "Mozilla/5.0 (SymbianOS/9.1; U; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/413 (KHTML, like Gecko) Safari/413";
			break;
		case S3E_OS_ID_WINMOBILE:
			UserAgent = device_id;
			UserAgent += " Opera/9.50 (Windows NT 5.1; U; ";
			UserAgent += locale;
			UserAgent += ")";
			break;
		case S3E_OS_ID_WINCE:
			UserAgent += "Mozilla/4.0 (compatible; MSIE 4.01; Windows CE; ";
			UserAgent = device_id;
			UserAgent += "; ";
			UserAgent += locale;
			UserAgent += "; )";
			UserAgent += CIwGameString(width);
			UserAgent += "x";
			UserAgent += CIwGameString(height);
			UserAgent += ")";
			break;
		case S3E_OS_ID_QNX:
			UserAgent = "Mozilla/5.0 (";
			UserAgent += s3eDeviceGetString(S3E_DEVICE_OS);
			UserAgent += "/";
			UserAgent += device_id;
			UserAgent += "; U; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/534.8+ (KHTML, like Gecko) Version/0.0.1 Safari/534.8+";
			break;
/*		case S3E_OS_ID_BADA:
			{
				UserAgent = "SAMSUNG; ";
				UserAgent += device_id;
				UserAgent += "; U; Bada/1.2; ";
				UserAgent += locale;
				UserAgent += ") AppleWebKit/533.1 (KHTML, like Gecko) Dolfin/2.2 Mobile ";
				const char* mode_name = CIwGameUtils::GetGraphicModeName(width, height);
				if (mode_name != NULL)
					UserAgent += mode_name;
				UserAgent += " SMM-MMS/1.2.0 OPN-B";
			}
			break;*/
		case S3E_OS_ID_ANDROID:
			UserAgent = "Mozilla/5.0 (Linux; U; Android 2.1; ";
			UserAgent += locale;
			UserAgent += "; ";
			UserAgent += device_id;
			UserAgent += ") AppleWebKit/530.17 (KHTML, like Gecko) Version/4.0 Mobile Safari/530.17";
			break;
		case S3E_OS_ID_IPHONE:
			UserAgent = "Mozilla/5.0 (";
			UserAgent += s3eDeviceGetString(S3E_DEVICE_OS);
			UserAgent += "/";
			UserAgent += device_id;
			UserAgent += "; U; ";
			UserAgent += locale;
			UserAgent += "; like Mac OS X; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8C148 Safari/6533.18.5";
			break;
		case S3E_OS_ID_WEBOS:
			UserAgent = "Mozilla/5.0 (webOS/1.0; U; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/525.27.1 (KHTML, like Gecko) Version/1.0 Safari/525.27.1 ";
			UserAgent += device_id;
			break;
		case S3E_OS_ID_WINDOWS:
			UserAgent = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0; ";
			UserAgent += locale;
			UserAgent += ")";
			break;
		case S3E_OS_ID_LINUX:
			UserAgent = "Mozilla/5.0 (";
			UserAgent += s3eDeviceGetString(S3E_DEVICE_OS);
			UserAgent += "/";
			UserAgent += device_id;
			UserAgent += "; U; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/534.8+ (KHTML, like Gecko) Version/0.0.1 Safari/534.8+";
			break;
		case S3E_OS_ID_OSX:
			UserAgent = "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_8; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/533.21.1 (KHTML, like Gecko) Version/5.0.5 Safari/533.21.1";
			break;
		default:
			UserAgent = "Mozilla/5.0 (";
			UserAgent += s3eDeviceGetString(S3E_DEVICE_OS);
			UserAgent += "/";
			UserAgent += device_id;
			UserAgent += "; U; ";
			UserAgent += locale;
			UserAgent += ") AppleWebKit/534.8+ (KHTML, like Gecko) Version/0.0.1 Safari/534.8+";
			break;
		}
	}

	CIwGameError::LogError("UserAgent: ", UserAgent.c_str());
}

bool CIwGameHttpManager::DetermineIPAddress()
{
	s3eSocket* sock = s3eSocketCreate(S3E_SOCKET_TCP, 0);
	if (sock != NULL)
	{
		s3eInetAddress addr;
		memset(&addr, 0, sizeof(addr));
		if (s3eInetLookup(PING_DOMAIN, &addr, NULL, NULL) != S3E_RESULT_ERROR)
		{
			addr.m_Port = s3eInetHtons(80);

	        uint64 start_time = s3eTimerGetMs();
			while (1)
			{
				s3eResult res = s3eSocketConnect(sock, &addr, NULL, NULL);
				if (res == S3E_RESULT_SUCCESS)
				{
					s3eInetAddress address;
					memset(&address, 0, sizeof(address));
					if (s3eSocketGetLocalName(sock, &address) != S3E_RESULT_ERROR)
					{
						IPAddress.allocString(32);
						IPAddress = s3eInetNtoa(address.m_IPAddress, IPAddress.str(), 32);
						CIwGameError::LogError("IPAddress: ", IPAddress.c_str());
						s3eSocketClose(sock);
						return true;
					}
					s3eSocketClose(sock);
					return false;
				}
				else
				{
					switch (s3eSocketGetError())
					{
						case S3E_SOCKET_ERR_INPROGRESS:
						case S3E_SOCKET_ERR_ALREADY:
						case S3E_SOCKET_ERR_WOULDBLOCK:
							break;
						default:
							s3eSocketClose(sock);
							return false;
					}
				}
				// Socket request timed out
				if ((s3eTimerGetMs() - start_time) > 10000)
				{
					s3eSocketClose(sock);
					return false;
				}
				s3eDeviceYield(30);
			}
		}
	}

	return false;
}


