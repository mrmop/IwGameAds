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

#if !defined(_CIW_GAME_IMAGE_H_)
#define _CIW_GAME_IMAGE_H_

#include "IwGx.h"
#include "Iw2D.h"
#include "IwList.h"
#include "IwGameUtil.h"
#include "IwGameFile.h"

//
//
//
//
// CIwGameImage - Represents a bitmapped image
//
// Note that images can be either loaded on demand (loaded on first call to getImage2D() or pre loaded by calling Load()
//
//
//
//
class CIwGameImage
{
public:
	enum eCIwGameImage_State
	{
		CIwGameImage_State_Invalid, 
		CIwGameImage_State_Loaded, 
	};

	// Properties
protected:
#if defined (_DEBUG)
	CIwGameString			Name;				// Image name
#endif	// _DEBUG
	unsigned int			NameHash;			// Image name hash
	eCIwGameImage_State		State;				// State of image
	CIw2DImage*				Image2D;			// Iw2D Image
	int						Width;				// Pixel width of image
	int						Height;				// Pixl height of image
public:
	void					setName(const char* name)
	{
#if defined (_DEBUG)
		Name = name;
#endif	// _DEBUG
		NameHash = CIwGameString::CalculateHash(name);
	}
	unsigned int			getNameHash()							{ return NameHash; }
	eCIwGameImage_State		getState() const						{ return State; }
	CIw2DImage*				getImage2D()							{ if (Load()) return Image2D; return NULL; }
	int						getWidth() const						{ return Width; }
	int						getHeight() const						{ return Height; }
	CIwGameImage*			getCopy();
	// Properties End
protected:
	CIwGameFile*			File;				// File object (if image if file based)
	bool					DecompressJPEG(char* jpeg_data, int jpeg_data_size);

public:
	CIwGameImage() : Image2D(NULL), File(NULL), State(CIwGameImage_State_Invalid), Width(0), Height(0)	{ }
	virtual ~CIwGameImage()
	{
		SAFE_DELETE(Image2D)
		SAFE_DELETE(File)
	}

	bool			Init(void* memory_file, int memory_file_size);	// Init an image from a memory based file (image is loaded)
	void			Init(const char* filename)						// Init an image from a file (image is not loaded)
	{
		File = new CIwGameFile();
		File->setFilename(filename);
	}

	bool			Load(bool blocking = true);						// Force load the image

	// Internal
	void			FinishLoad();									// Called back when aysnc loading is completed
};


#endif	// _CIW_GAME_IMAGE_H_
