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

#if !defined(_IW_GAME_FILE_H_)
#define _IW_GAME_FILE_H_


// TODO: 
// - Add none blocking file I/O
// - Add timeout check to blocking http download
// - Add methods for reading and writing common types

#include "s3efile.h"
#include "IwGameString.h"
#include "IwGameUtil.h"
#include "IwGameHttp.h"


struct CIwGameFilePathComponents
{
	CIwGameString Folder;
	CIwGameString Filename;
	CIwGameString Extension;
};

//
//
// CIwGameFile - CIwGameFile respresents a Marmalade file
// 
//
class CIwGameFile
{
public:
	enum eiwGameFileError
	{
		ErrorNone, 
		ErrorInvalidPath, 
		ErrorOpenFailed, 
		ErrorAlreadyOpen, 
		ErrorEmptyMode, 
		ErrorHttp, 
	};
protected:
	// Properties
	s3eFile*			File;								// Marmalade file handle
	CIwGameString		Name;								// Name
	CIwGameString		Filename;							// filename
	bool				FileAvailable;						// True when ad is available
	CIwGameCallback		FileAvailableCallback;				// Callback to be called when file is available
	void*				FileAvailableCallbackData;			// Callback data to be passed back with callback
	eiwGameFileError	Error;								// Comntains error code if any if file not received
public:
	s3eFile*			getFileHandle()						{ return File; }
	void				setName(const char* name)			{ Name.setString(name); }
	CIwGameString&		getName()							{ return Name; }
	void				setFilename(const char* filename)	{ Filename.setString(filename); }
	CIwGameString&		getFilename()						{ return Filename; }
	void				setFileAvailable(bool available)	{ FileAvailable = available; }
	bool				isFileAvailable()					{ return FileAvailable; }
	void				setFileAvailableCallback(CIwGameCallback callback, void *data)	{ FileAvailableCallback = callback; FileAvailableCallbackData = data; }
	int					getFileSize();
	eiwGameFileError	getError() const					{ return Error; }
	void*				getContent();
	int					getContentLength() const;
	// Properties end

protected:
	bool				InMemory;
	CIwGameHttpRequest* Request;
	void				NotifyAvailable();
	bool				Download();										// Download file from an external location

public:
	CIwGameFile() :						File(NULL), Error(ErrorNone), Request(NULL), FileAvailableCallback(NULL), FileAvailableCallbackData(NULL) 	{	}
	CIwGameFile(const char* filename) : File(NULL), Error(ErrorNone), Request(NULL), FileAvailableCallback(NULL), FileAvailableCallbackData(NULL)
	{
		Filename.setString(filename);
	}
	virtual		~CIwGameFile()
	{
		SAFE_DELETE(Request)
		Close();
	}
	bool				Open(const char* filename = NULL, const char* mode = NULL, bool blocking = false);	// Open file for read or write
	bool				Open(void* memory_buffer, int memory_buffer_len);		// Open file for read or write from a memory buffer
	bool				Invalid();												// Releases file

	bool				Read(void* buffer, int len);
	bool				Write(void* buffer, int len);
	bool				Seek(int offset, s3eFileSeekOrigin origin);
	void				Close();

	// Utility
	static void			GetComponents(const char* file_path, CIwGameFilePathComponents& components);
	static bool			GetFileType(const char* file_path, CIwGameString& type);
	static bool			isHttp(const char* file_path, int path_len);


	// Internal
	void				FileReceived(CIwGameHttpRequest* request, int error);	// Called by the http callback internally when the file is received
	
};

#endif	// _IW_GAME_FILE_H_
