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

#include "IwGameImage.h"
#include "IwGameFile.h"
#include "IwGameString.h"

#define _DEBUG

//
//
//
//
// JPEG utility
//
//
//
//
extern "C" 
{
#include "jpeglib.h"
}

typedef struct 
{
	jpeg_source_mgr	pub;
	char*			buf;
	char			buf_term[2];
	long			buf_size;
	long			pos;
	bool			read_started;
} IwGameImage_buf_source_mgr;

void IwGameImage_init_source_from_buf(j_decompress_ptr cinfo)
{
	IwGameImage_buf_source_mgr* src = (IwGameImage_buf_source_mgr*) cinfo->src;
	src->read_started = true;
}

void IwGameImage_skip_input_data_from_buf(j_decompress_ptr cinfo, long nbytes)
{
	IwGameImage_buf_source_mgr* src = (IwGameImage_buf_source_mgr*)cinfo->src;
	if (nbytes > 0)
	{
		src->pub.next_input_byte += (size_t) nbytes;
		src->pub.bytes_in_buffer -= (size_t) nbytes;
	}
}

boolean IwGameImage_fill_input_buffer_from_buf(j_decompress_ptr cinfo)
{
	IwGameImage_buf_source_mgr* src = (IwGameImage_buf_source_mgr*)cinfo->src;

	if (src->pos == src->buf_size)
	{
		src->buf_term[0] = (JOCTET) 0xFF;
		src->buf_term[1] = (JOCTET) JPEG_EOI;
		src->pub.next_input_byte = (JOCTET*)src->buf_term;
		src->pub.bytes_in_buffer = 2;
		src->read_started = false;
		return TRUE;
	}

	src->pub.next_input_byte = (JOCTET*)src->buf;
	src->pub.bytes_in_buffer = src->buf_size;
	src->pos = src->buf_size;
	src->read_started = false;

	return TRUE;
}

void IwGameImage_term_source_from_buf(j_decompress_ptr cinfo)
{
}

void IwGameImage_jpeg_buf_src(j_decompress_ptr cinfo, char* buf,long size)
{
	IwGameImage_buf_source_mgr* src = (IwGameImage_buf_source_mgr*) cinfo->src;
	if (cinfo->src == NULL)
	{
		cinfo->src = (struct jpeg_source_mgr *)
		(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(IwGameImage_buf_source_mgr));
		src = (IwGameImage_buf_source_mgr*)cinfo->src;
	}

	src = (IwGameImage_buf_source_mgr*) cinfo->src;
	src->pub.init_source = IwGameImage_init_source_from_buf;
	src->pub.fill_input_buffer = IwGameImage_fill_input_buffer_from_buf;
	src->pub.skip_input_data = IwGameImage_skip_input_data_from_buf;
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = IwGameImage_term_source_from_buf;
	src->pub.bytes_in_buffer = 0;
	src->pub.next_input_byte = (JOCTET*)NULL;

	src->buf = buf;
	src->read_started = false;
	src->buf_size = size;
	src->pos = 0;
}

bool IwGameImage_IsJPEG(const char *jpeg_data, int length)
{
	const char pJPEGSignature[] = { 0xFF, 0xD8, 0xFF };
	const char pJPEGSignature2a[] = { 0x4A, 0x46, 0x49, 0x46 };
	const char pJPEGSignature2b[] = { 0x45, 0x78, 0x69, 0x66 };

	if (length > 10)
	{
		if (!memcmp(jpeg_data, pJPEGSignature, 3))
		{
			if (!memcmp(jpeg_data + 6, pJPEGSignature2a, 4))
				return true;
			if (!memcmp(jpeg_data + 6, pJPEGSignature2b, 4))
				return true;
		}
	}

	return false;
}


//
//
//
//
// CIwGameImage implementation
//
//
//
//

CIwGameImage* CIwGameImage::getCopy()
{
	if (Image2D == NULL)
		return NULL;

	CIwGameImage* image = new CIwGameImage();
	image->NameHash = NameHash;
	image->State = State;
	image->Width = Width;
	image->Height = Height;
	image->Image2D = Iw2DCreateImage(Image2D->GetMaterial()->GetTexture()->GetImage());

	return image;
}

int32 CIwGameImage_FileRetrievedAsyncCallback(void* caller, void* data)
{
	CIwGameImage* image = (CIwGameImage*)data;
	image->FinishLoad();

	return 0;
}

bool CIwGameImage::Load(bool blocking)
{
	// If already loaded return true
	if (Image2D != NULL)
		return true;

	// Image is file based
	if (File != NULL)
	{
		File->setFileAvailableCallback(CIwGameImage_FileRetrievedAsyncCallback, this);
		if (File->Open(NULL, "rb", blocking))
			return true;
		else
			return false;
	}

	return false;
}

void CIwGameImage::FinishLoad()
{
	if (File != NULL)
	{
		if (File->isFileAvailable() && File->getError() == CIwGameFile::ErrorNone)
			Init(File->getContent(), File->getContentLength());
		SAFE_DELETE(File)
	}
}

bool CIwGameImage::Init(void* memory_file, int memory_file_size)
{
	CIwGameFile file;
	if (file.Open(memory_file, memory_file_size))
	{
		if (IwGameImage_IsJPEG((const char*)memory_file, memory_file_size))
		{
			if (!DecompressJPEG((char*)memory_file, memory_file_size))
				return false;
		}
		else
		{
			CIwImage image;
			image.ReadFile(file.getFileHandle());
			Image2D = Iw2DCreateImage(image);
			if (Image2D == NULL || image.GetFormat() == CIwImage::FORMAT_UNDEFINED)
			{
#if defined(_DEBUG)
				CIwGameError::LogError("Error: CIwGameImage::Init() - Could not create image!");
				return false;
#endif	// _DEBUG
			}
			else
			{
				Width = Image2D->GetWidth();
				Height = Image2D->GetHeight();
#if defined(_DEBUG)
				CIwGameError::LogError("Info: CIwGameImage::Init() - Size = ", CIwGameString(memory_file_size).c_str());
				CIwGameError::LogError("Info: CIwGameImage::Init() - Width = ", CIwGameString(Width).c_str());
				CIwGameError::LogError("Info: CIwGameImage::Init() - Height = ", CIwGameString(Height).c_str());
				CIwGameError::LogError("Info: CIwGameImage::Init() - Bit depth = ", CIwGameString(image.GetBitDepth()).c_str());
				CIwGameError::LogError("Info: CIwGameImage::Init() - Format = ", CIwGameString(image.GetFormat()).c_str());
#endif	// _DEBUG
			}
		}
	}

	// Sanity check
	if (Width <= 0 || Height <= 0 || Width > 16384 || Height > 16384)
		return false;

	State = CIwGameImage_State_Loaded;

	return true;
}

bool CIwGameImage::DecompressJPEG(char* jpeg_data, int jpeg_data_size)
{
	// Decompress the JPEG data
	int			length = jpeg_data_size;
	struct		jpeg_error_mgr jerr;
	struct		jpeg_decompress_struct cinfo;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress	(&cinfo);
	IwGameImage_jpeg_buf_src(&cinfo,(char*)jpeg_data, length);
	jpeg_read_header	(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	jpeg_start_decompress	(&cinfo);
	
	int newlen = cinfo.image_width * cinfo.image_height * 3;

	unsigned char * data = (unsigned char*)s3eMalloc(newlen);
	if (data == NULL)
	{
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return false;
	}
	unsigned char * linha = data;
   
	while (cinfo.output_scanline < cinfo.output_height)
	{
		linha = data + 3 * cinfo.image_width * cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo,&linha,1);
	}
    
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	Width = cinfo.output_width;
	Height = cinfo.output_height;

//	int stride = cinfo.output_components;

	// Create CIw2DImage from pixel data
	CIwImage image;
	image.SetFormat(CIwImage::RGB_888);
	image.SetWidth(Width);
	image.SetHeight(Height);
	image.SetOwnedBuffers((uint8*)data, 0);
	Image2D = Iw2DCreateImage(image);

//	s3eFree(data);

	return true;
}



