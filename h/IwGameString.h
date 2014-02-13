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

#ifndef __IW_GAME_STRING_H__
#define __IW_GAME_STRING_H__

#include <string>
#include "IwHashString.h"

//
//
// CIwGameString - A simple named string builder style class
//
// NOTE: If you are building strings then its best to create a stering of the size you think you will need then concatenate it, this will save lots of memory allocations and fragmentation
// In short, new a CIwGameString() and then allocate some space for it using allocString(size_you_think_you_will_need)
//
//

class CIwGameString
{
	// Properties
protected:
	char*			Data;			// The string data
	int				Length;			// Length of the string
	int				Size;			// Size of memory reserved for the string
	unsigned int	DataHash;		// Hash value of the data (for fast comparison)
	unsigned int	NameHash;		// Hash value of the name (for fast searching)
	bool			AutoHash;		// true to calculate hash value every time string is updated
	int				FindIndex;		// Used to track multiple searches

public:
	void			reallocString(int len);
	void			allocString(int len);
	void			reset();
	void			setString(const char *str);
	void			setString(const char *str, int len);
	char*			getString()									{ return Data; }
	CIwGameString	getSubString(int start, int max_chars);
	void			setName(char *name);
	unsigned int	getNameHash() const							{ return NameHash; }
	unsigned int	getHash() const								{ return DataHash; }
	void			setAutoHash(bool enable);
	void			setLength(int length)						{ Length = length; }
	bool			isAutohash() const							{ return AutoHash; }
	// Properties end

private:

public:
	CIwGameString() : Data(NULL), Length(0), Size(0), AutoHash(true), FindIndex(0), DataHash(0), NameHash(0) {}
	CIwGameString(const CIwGameString &string);
	CIwGameString(const char *pString, int start, int num_chars);
	CIwGameString(const char *pString);
	CIwGameString(int nNum);
	CIwGameString(unsigned int nNum);
	CIwGameString(float fNum);
	CIwGameString(bool value);
	virtual ~CIwGameString()
	{
		if (Data != NULL)
			delete [] Data;
	}

	// Query
	int				GetSize() const { return Size; }
	int				GetLength() const { return Length; }
	char*			GetCharPtr() { return Data; }
	int				GetAsInt() const;
	bool			GetAsBool() const;
	float			GetAsFloat() const;
	int				GetAsListOfInt(int *int_pool);
	int				GetAsListOfFloat(float* float_pool);
	bool			IsEmpty() const { return Data == NULL || Length == 0; }
	
	/// Copy
	void			Copy(const char* pString);
	void			Copy(CIwGameString& pString);
	void			Copy(const char *pString, int start, int count);

	// Operators
	CIwGameString&	operator=	(const CIwGameString& op);
	CIwGameString&	operator=	(const char *op);
	char			operator[]	(int nIndex);
	CIwGameString&	operator+=	(const CIwGameString& op);
	CIwGameString&	operator+=	(const char* op);
	CIwGameString&	operator=	(int nNum);
	CIwGameString&	operator+=	(int nNum);
	bool			operator==	(const CIwGameString& op);
	bool			operator==	(const char* op);
	bool			operator==	(unsigned int hash);
	char*			str() const { return Data; }
	const char*		c_str() const { return (const char *)Data; }

	bool			Compare(const char* pString, int len) const;
	bool			Compare(int start, const char* pString, int len) const;

	// Searching
	int				Find(const char* string);											// Simple string search
	int				FindNext(const char* string, int len);								// Searches from last find position for text string
	int				FindNext(const char* string);										// Searches from last find position for text string
	int				FindNext(char chr);													// Searches from last find position for the specified character
	void			FindReset();														// Resets the find position to start of string
	int				StepFindIndex(int amount);											// Adjust the find position by the specified
	int				StepFindIndexNoneWhiteSpace();										// Adjust the find position to the next none white space
	int				StepFindIndexWhiteSpace();											// Adjust the find position to the next white space
	void			setFindIndex(int index) { FindIndex = index; }						// Sets the current find index
	int				getFindIndex() const { return FindIndex; }							// Gets the current find index
	int				GetNextMarkerOffset(char marker);									// Returns length to the end of paramneter marker
	int				GetNextString();													// Returns the next string
	int				GetNextMarkedString(char start_mark, char end_mark, int &offset);	// Returns a string marked by start and end marker characters
	int				GetNextMarkedStringAfterString(const char* search_string, char start_mark, char end_mark, CIwGameString& out_string);	// Returns the next marked string after finding a certain string

	// Utility
	int				Replace(const char* string, const char* with);
	void			Replace(char chr, char with);
	bool			ContainsSpecialCharacters() const;
	int				Contains(char c) const;
	void			ReplaceHTMLCodes();
	void			URLEncode(const char* str);
	void			URLEncode();
	void			URLDecode();
	bool			SplitFilename(CIwGameString& filename, CIwGameString& ext);
	bool			GetFilenameExt(CIwGameString& ext);

	static unsigned int	CalculateHash(const char* pString);

	inline static bool IsNumber(char c)
	{
		if (c >= '0' && c <= '9')
			return true;
		return false;
	}

	inline static bool IsAlpha(char c)
	{
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
			return true;
		return false;
	}

	inline static bool IsLower(char c)
	{
		if (c >= 'a' && c <= 'z')
			return true;
		return false;
	}

	inline static bool IsUpper(char c)
	{
		if (c >= 'A' && c <= 'Z')
			return true;
		return false;
	}

	inline static int GetValueFromHexDigit(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;

		return 0;
	}
	
	void		ToUpper();
	void		ToLower();
};


#endif	// __IW_GAME_STRING_H__
