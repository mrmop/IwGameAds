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

#include "IwGameFile.h"
#include "IwGameUtil.h"

//
//
//
//
// CIwGameFile implementation
//
//
//
//
int	CIwGameFile::getFileSize()
{
	if (File == NULL)
		return -1;

	return s3eFileGetSize(File);
}

bool CIwGameFile::Read(void* buffer, int len)
{
	if (File == NULL)
		return false;

	if (s3eFileRead(buffer, len, 1, File) != 1)
	{
#if defined(_DEBUG)
		s3eFileGetError();
		CIwGameError::LogError("Error: CIwGameFile::Read(): ", s3eFileGetErrorString());
#endif	// _DEBUG
		Close();
		return false;
	}

	return true;
}

bool CIwGameFile::Write(void* buffer, int len)
{
	if (File == NULL)
		return false;

	if (s3eFileWrite(buffer, len, 1, File) != 1)
	{
#if defined(_DEBUG)
		s3eFileGetError();
		CIwGameError::LogError("Error: CIwGameFile::Write(): ", s3eFileGetErrorString());
		Close();
#endif	// _DEBUG
		return false;
	}

	return true;
}

bool CIwGameFile::Open(const char* path, const char* mode, bool blocking)
{
	FileAvailable = false;
	InMemory = false;

	// make sure user cannot open the file twice
	if (File != NULL)
	{
		Error = ErrorAlreadyOpen;
		return false;
	}

	// Get the path
	if (path != NULL)
		Filename.setString(path);
	else
		path = Filename.c_str();

	// Check we actually have a path
	if (path == NULL)
	{
		Error = ErrorInvalidPath;
		return false;
	}

	// Check to see if the file is located on the web
	if (isHttp(Filename.c_str(), Filename.GetLength()))
	{
		Download();

		if (blocking)
		{
			while (!FileAvailable)
			{
				IW_GAME_HTTP_MANAGER->Update();
				s3eDeviceYield(0);
			}
			if (Error != ErrorNone)
				return false;
		}

		return true;
	}
	else
	{
		if (mode == NULL)
		{
			Error = ErrorEmptyMode;
			return false;
		}
	}

	// Open the file
	File = s3eFileOpen(Filename.c_str(), mode);
	if (File == NULL)
	{
#if defined(_DEBUG)
		s3eFileGetError();
		CIwGameError::LogError("Error: CIwGameFile::Open(): ", s3eFileGetErrorString());
#endif	// _DEBUG
		Error = ErrorOpenFailed;
		return false;
	}
	NotifyAvailable();

	FileAvailable = true;

	return true;
}

bool CIwGameFile::Open(void* memory_buffer, int memory_buffer_len)
{
	FileAvailable = false;
	InMemory = true;

	File = s3eFileOpenFromMemory(memory_buffer, memory_buffer_len);
	if (File == NULL)
	{
#if defined(_DEBUG)
		s3eFileGetError();
		CIwGameError::LogError("Error: CIwGameFile::Open(memory): ", s3eFileGetErrorString());
#endif	// _DEBUG
		Error = ErrorOpenFailed;
		return false;
	}

	FileAvailable = true;

	return true;
}

void CIwGameFile::Close()
{
	if (File != NULL)
	{
		s3eFileClose(File);
		File = NULL;
	}
}

bool CIwGameFile::Seek(int offset, s3eFileSeekOrigin origin)
{
	if (File == NULL)
		return false;

	if (s3eFileSeek(File, offset, origin) != S3E_RESULT_SUCCESS)
		return false;

	return true;
}

void* CIwGameFile::getContent()
{
	if (!InMemory || Request == NULL)
		return NULL;
	if (FileAvailable && Request->getError() == 0)
		return (void*)Request->getContent().c_str();

	return NULL;
}

int CIwGameFile::getContentLength() const
{
	if (!InMemory || Request == NULL)
		return -1;
	if (FileAvailable && Request->getError() == 0)
		return Request->getContentLength();

	return -1;
}

int32 FileRetrievedCallback(void* caller, void* data)
{
	CIwGameHttpRequest* request = (CIwGameHttpRequest*)caller;

	if (request->getProcessed())									// Check to see if our request was processed by the http manager
	{
		((CIwGameFile*)data)->FileReceived(request, request->getError());
		IW_GAME_HTTP_MANAGER->RemoveRequest(request);				// Remove request from http manager queue
	}

	return 0;
}

bool CIwGameFile::Download()
{
	FileAvailable = false;

	// Build download request
	if (Request == NULL)
		Request = new CIwGameHttpRequest();
	Request->setGET();
	Request->setURI(Filename.c_str());
	Request->setContentAvailableCallback(&FileRetrievedCallback, (void*)this);
	IW_GAME_HTTP_MANAGER->AddRequest(Request);

#if defined(_DEBUG)
	CIwGameError::LogError("CIwGameFile::Download with URL:", Filename.c_str());
#endif	// _DEBUG

	return false;
}

void CIwGameFile::FileReceived(CIwGameHttpRequest* request, int error)
{
	// If there wwas an error then set the error
	if (error != 0)
	{
		Error = ErrorHttp;
#if defined(_DEBUG)
		CIwGameError::LogError("Error: CIwGameFile::FileReceived(): ", CIwGameString(error).c_str());
#endif	// _DEBUG
	}

	// Open the file
	Open((void*)request->getContent().c_str(), request->getContentLength());

	// Notify caller that the file is available
	NotifyAvailable();
}

void CIwGameFile::NotifyAvailable()
{
	// Call any user supplied callback
	if (FileAvailableCallback != NULL)
		FileAvailableCallback((void*)this, FileAvailableCallbackData);

	FileAvailable = true;
}

//
//
//
// Utility methods
//
//
//

void CIwGameFile::GetComponents(const char* file_path, CIwGameFilePathComponents& components)
{
	int							len = strlen(file_path) - 1;
	const char*					name_ptr = file_path + len;

	// Scan backwards looking for dot
	int index = 0;
	while (len >= 0)
	{
		if (*name_ptr == '.')
		{
			components.Extension.setString(name_ptr + 1, index);
			name_ptr--;
			len--;
			break;
		}
		else
		if (len == 0)
		{
			components.Extension.setString(name_ptr, index + 1);
			name_ptr--;
			len--;
			break;
		}
		name_ptr--;
		index++;
		len--;
	}
	if (len < 0)
		return;

	// Continue scanning for filename
	index = 0;
	while (len >= 0)
	{
		if (*name_ptr == '/' || *name_ptr == '\\')
		{
			components.Filename.setString(name_ptr + 1, index);
			name_ptr--;
			len--;
			break;
		}
		else
		if (len == 0)
		{
			components.Filename.setString(name_ptr, index + 1);
			name_ptr--;
			len--;
			break;
		}

		name_ptr--;
		index++;
		len--;
	}
	if (len < 0)
		return;

	len = (name_ptr - file_path) + 1;
	if (len > 0)
		components.Folder.setString(file_path, len);
}

bool CIwGameFile::GetFileType(const char* file_path, CIwGameString& type)
{
	int						len = strlen(file_path) - 1;
	const char*				name_ptr = file_path + len;

	// Scan backwards looking for dot
	int index = 0;
	while (len >= 0)
	{
		if (*name_ptr == '.')
		{
			type.setString(name_ptr + 1, index);
			type.ToLower();
			break;
		}
		else
		if (len == 0)
		{
			type.setString(name_ptr, index + 1);
			type.ToLower();
			break;
		}
		name_ptr--;
		index++;
		len--;
	}
	
	return true;
}

bool CIwGameFile::isHttp(const char* file_path, int path_len)
{
	// Ignore very short path names because they cannot be a URI
	if (path_len < 10)
		return false;

	int check = *((int*)file_path);
	if (check == (((int)'h') | ((int)'t' << 8) | ((int)'t' << 16) | ((int)'p' << 24)) || check == (((int)'H') | ((int)'T' << 8) | ((int)'T' << 16) | ((int)'P' << 24)))
	{
		if (*(file_path + 4) == ':' || *(file_path + 5) == ':')
			return true;
	}

	return false;
}

