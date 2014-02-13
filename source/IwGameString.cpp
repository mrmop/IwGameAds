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

#include <stdio.h>
#include <stdlib.h>

#include "IwGameString.h" 


//
//
// CIwGameString implementation
//
//


CIwGameString::CIwGameString(const CIwGameString &string)
{
#if defined(_DEBUG)
	IwAssertMsg(MYAPP, string.GetSize() >= 0, ("Cannot alloc string with < 0 length")); 
#endif
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;
	if (string.c_str() == NULL)
	{
		Length = 0;
		Size = 0;
	}
	else
	{
		int len = (int)strlen(string.c_str());
		allocString(len);
		Length = len;
		strcpy(Data, string.c_str());

		if (AutoHash)
			DataHash = IwHashString(Data);
	}
}

CIwGameString::CIwGameString(const char *pString)
{
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;
	if (pString == NULL)
	{
		Length = 0;
		Size = 0;
	}
	else
	{
		int len = (int)strlen(pString);
		allocString(len);
		Length = len;
		strcpy(Data, pString);

		if (AutoHash)
			DataHash = IwHashString(Data);
	}
}

CIwGameString::CIwGameString(const char *pString, int start, int num_chars)
{
#if defined(_DEBUG)
	IwAssertMsg(MYAPP, num_chars >= 0, ("Cannot alloc string with < 0 length")); 
#endif
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;
	if (pString == NULL)
	{
		Length = 0;
		Size = 0;
	}
	else
	{
		int len = num_chars;
		allocString(len);
		Length = len;
		memcpy(Data, pString + start, num_chars);
		Data[len] = 0;

		if (AutoHash)
			DataHash = IwHashString(Data);
	}
}

CIwGameString::CIwGameString(int nNum)
{
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;
	char str[32];
	snprintf(str, 32, "%d", nNum);

	int len = (int)strlen(str);
	allocString(len);
	Length = len;

	strcpy(Data, str);

	if (AutoHash)
		DataHash = IwHashString(Data);
}

CIwGameString::CIwGameString(unsigned int nNum)
{
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;
	char str[32];
	snprintf(str, 32, "%u", nNum);

	int len = (int)strlen(str);
	allocString(len);
	Length = len;

	strcpy(Data, str);

	if (AutoHash)
		DataHash = IwHashString(Data);
}

CIwGameString::CIwGameString(float fNum)
{
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;
	char str[32];
	snprintf(str, 32, "%f", fNum);

	int len = (int)strlen(str);
	allocString(len);
	Length = len;

	strcpy(Data, str);

	if (AutoHash)
		DataHash = IwHashString(Data);
}

CIwGameString::CIwGameString(bool value)
{
	FindIndex = 0;
	Data = NULL;
	AutoHash = true;

	int len;
	
	if (value)
		len = 4;
	else
		len = 5;

	allocString(len);
	Length = len;

	if (value)
		strcpy(Data, "true");
	else
		strcpy(Data, "false");

	if (AutoHash)
		DataHash = IwHashString(Data);
}

void CIwGameString::reallocString(int len)
{
#if defined(_DEBUG)
	IwAssertMsg(MYAPP, len >= 0, ("Cannot realloc string with < 0 length")); 
#endif
	int old_size = Size;
	Size = len + 1;
	if (Size < 16) Size = 16;
	char* data = new char [Size];
	if (Data != NULL)
	{
		memcpy(data, Data, old_size);
		delete [] Data;
	}
	Data = data;
}
void CIwGameString::allocString(int len)
{
#if defined(_DEBUG)
	IwAssertMsg(MYAPP, len >= 0, ("Cannot alloc string with < 0 length")); 
#endif
	Size = len + 1;
	if (Size < 16) Size = 16;
	if (Data != NULL)
		delete [] Data;
	Data = new char[Size];
	Data[0] = 0;
}
void CIwGameString::reset()
{
	Length = 0;
	if (Size > 0 && Data != NULL)
		Data[0] = 0;
}
void CIwGameString::setString(const char *str)
{
	FindIndex = 0;
	if (str == NULL)
	{
		if (Data != NULL)
			delete [] Data;
		Data = NULL;
		Size = 0;
		Length = 0;
		return;
	}
	int len = strlen(str);
	allocString(len);
	strcpy(Data, str);
	Length = len;
	if (AutoHash)
		DataHash = IwHashString(Data);
}
void CIwGameString::setString(const char *str, int len)
{
	FindIndex = 0;
	if (str == NULL)
	{
		if (Data != NULL)
			delete [] Data;
		Data = NULL;
		Size = 0;
		Length = 0;
		return;
	}
	allocString(len);
	memcpy(Data, str, len);
	Data[len] = 0;
	Length = len;
	if (AutoHash)
		DataHash = IwHashString(Data);
}

CIwGameString CIwGameString::getSubString(int start, int max_chars)
{
	if ((start + max_chars) > Length)
		max_chars -= (start + max_chars) - Length;
	return CIwGameString(Data, start, max_chars);
}
void CIwGameString::setName(char *name)
{
	if (name == NULL)
		return;
	NameHash = IwHashString(name);
}

void CIwGameString::setAutoHash(bool enable)
{
	AutoHash = enable;
	if (enable)
	{
		if (Data != NULL)
			DataHash = IwHashString(Data);
	}
	else
	{
		DataHash = 0;
	}
}

int CIwGameString::GetAsInt() const
{
	if (Data == NULL)
		return 0;
	return atoi(Data);
}

float CIwGameString::GetAsFloat() const
{
	if (Data == NULL)
		return 0;
	return (float)atof(Data);
}

bool CIwGameString::GetAsBool() const
{
	if (Data == NULL)
		return false;
	return (Compare("1", 1) || Compare("true", 4));
}

int CIwGameString::GetAsListOfInt(int *int_pool)
{
	if (Data == NULL)
		return 0;

	int index = 0;

	// Separate Value by commas
	int			len = Length;
	const char* text = Data;
	char		c;
	char		num[64];

	while ((c = *text++) != 0)
	{
		// Find a number
		int count = 0;
		if (c >= '0' && c <= '9' || c == '-')
		{
			num[count] = c;
			// Read number to end
			while (count++ < 63)
			{
				c = *text;
				if (c < '0' || c > '9')
					break;
				text++;
				num[count] = c;
			}
			// Convert
			num[count] = 0;
			int n = atoi(num);
			int_pool[index++] = n;
		}
	}

	return index;
}

int CIwGameString::GetAsListOfFloat(float* float_pool)
{
	if (Data == NULL)
		return 0;

	int index = 0;

	// Separate Value by commas
	int			len = Length;
	const char* text = Data;
	char		c;
	char		num[64];

	while ((c = *text++) != 0)
	{
		// Find a number
		int count = 0;
		if (c >= '0' && c <= '9' || c == '-')
		{
			num[count] = c;
			// Read number to end
			while (count++ < 63)
			{
				c = *text;
				if ((c < '0' || c > '9') && c != '.')
					break;
				text++;
				num[count] = c;
			}
			// Convert
			num[count] = 0;
			float n = (float)atof((const char*)num);
			float_pool[index++] = n;
		}
	}

	return index;
}

void CIwGameString::Copy(const char* pString, int start, int count)
{
	if (Data == NULL)
	{
		allocString(count);
	}
	else
	{
		if (count >= Size)
			reallocString(count);
	}

	memcpy(Data, pString + start, count);
	Length = count;
	Data[count] = 0;

	if (AutoHash)
		DataHash = IwHashString(Data);
}

void CIwGameString::Copy(const char* pString)
{
	Copy(pString, 0, (int)strlen(pString));
}

void CIwGameString::Copy(CIwGameString& string)
{
	Copy((char *)string.c_str(), 0, string.GetLength());
}

char CIwGameString::operator[]	(int nIndex)
{
	if (nIndex < 0 || nIndex > GetLength())
		return 0;

	return *(Data + nIndex);
}

CIwGameString& CIwGameString::operator= (const CIwGameString &op)
{
	Length = op.GetLength();

	if (Data == NULL)
	{
		allocString(Length);
	}
	else
	{
		if (Length >= Size)
			reallocString(Length);
	}
	if (op.c_str() == NULL)
	{
		Data[0] = 0;
		return *this;
	}
	strcpy(Data, op.c_str());
	DataHash = op.getHash();

	return *this;
}

CIwGameString& CIwGameString::operator=	(const char *op)
{
	if (op == NULL)
		return *this;

	FindIndex = 0;
	Length = (int)strlen(op);

	if (Data == NULL)
	{
		allocString(Length);
	}
	else
	{
		if (Length >= Size)
			reallocString(Length);
	}

	if (op == NULL)
	{
		Data[0] = 0;
		return *this;
	}
	strcpy(Data, op);

	if (AutoHash)
		DataHash = IwHashString(Data);

	return *this;
}

CIwGameString& CIwGameString::operator+= (const CIwGameString &op)
{
	int len1 = GetLength();
	int	len2 = op.GetLength();

	if (Data == NULL)
	{
		allocString(len1 + len2);
		strcpy(Data, op.c_str());
	}
	else
	{
		if ((len1 + len2) >= Size)
			reallocString(len1 + len2);

		memcpy(Data + len1, op.c_str(), len2);
		Data[len1 + len2] = 0;
	}
	Length = len1 + len2;

	if (AutoHash)
		DataHash = IwHashString(Data);

	return *this;
}

CIwGameString& CIwGameString::operator+= (const char *op)
{
	if (op == NULL)
		return *this;

	int len1 = GetLength();
	int	len2 = (int)strlen(op);

	if (Data == NULL)
	{
		allocString(len1 + len2);
		strcpy(Data, op);
	}
	else
	{
		if ((len1 + len2) >= Size)
			reallocString(len1 + len2);

		memcpy(Data + len1, op, len2);
		Data[len1 + len2] = 0;
	}
	Length = len1 + len2;

	if (AutoHash)
		DataHash = IwHashString(Data);
	
	return *this;
}

CIwGameString& CIwGameString::operator=	(int nNum)
{
	FindIndex = 0;
	char str[32];
	snprintf(str, 32, "%d", nNum);

	int len = (int)strlen(str);

	if (len >= Size)
		reallocString(len);

	strcpy(Data, str);
	Length = len;

	if (AutoHash)
		DataHash = IwHashString(Data);
	
	return *this;
}

CIwGameString& CIwGameString::operator+=	(int nNum)
{
	char str[32];
	snprintf(str, 32, "%d", nNum);

	int len1 = GetLength();
	int	len2 = (int)strlen(str);

	if (Data == NULL)
	{
		allocString(len1 + len2);
		strcpy(Data, str);
	}
	else
	{
		if ((len1 + len2) >= Size)
			reallocString(len1 + len2);

		memcpy(Data + len1, str, len2);
		Data[len1 + len2] = 0;
	}

	Length = len1 + len2;

	if (AutoHash)
		DataHash = IwHashString(Data);
	
	return *this;
}


bool CIwGameString::operator==	(const CIwGameString &op)
{
	if (Data == NULL)
		return false;

	if (AutoHash && op.isAutohash())
	{
		if (DataHash == op.getHash())
			return true;
	}
	else
	{
		if (strcmp(op.c_str(), Data) == 0)
			return true;
	}
	return false;
}

bool CIwGameString::operator==	(unsigned int hash)
{
	if (Data == NULL)
		return false;

	if (!AutoHash)
		DataHash = CalculateHash(Data);

	if (DataHash == hash)
		return true;

	return false;
}

bool CIwGameString::operator==	(const char *op)
{
	if (Data == NULL)
		return false;

	if (strcmp(op, Data) == 0)
		return true;

	return false;
}

bool CIwGameString::Compare(const char* pString, int len) const
{
	return memcmp(Data, pString, len) == 0;
}

bool CIwGameString::Compare(int start, const char* pString, int len) const
{
	return memcmp(Data + start, pString, len) == 0;
}

unsigned int	CIwGameString::CalculateHash(const char* pString)
{
	if (pString == NULL)
		return 0;
		
	return IwHashString(pString);
}

int CIwGameString::Replace(const char* string, const char* with)
{
	int len = strlen(string);
	int wlen = strlen(with);

	// Find the string
	FindReset();

	// Count how many times the string occurs
	int occcurances = 0;
	int pos = 0;
	do
	{
		pos = FindNext(string, len);
		FindIndex = pos + len;
		if (pos >= 0)
			occcurances++;
	}
	while (pos >= 0);
	if (occcurances == 0)
		return -1;

	// Calculate new string size
	int new_size = Length - len * occcurances + wlen * occcurances;
	char* new_buffer = new char[new_size + 1];

	// Build new string with replacements
	FindReset();
	int start_pos = 0, buff_pos = 0;
	pos = 0;
	do
	{
		pos = FindNext(string, len);
		FindIndex = pos + len;
		if (pos < 0)
		{
			// Copy any remaining string
			memcpy(new_buffer + buff_pos, Data + start_pos, Length - start_pos);
			buff_pos += Length - start_pos;
			break;
		}

		// Copy string that doesnt match
		int plen = pos - start_pos;
		memcpy(new_buffer + buff_pos, Data + start_pos, plen);
		buff_pos += plen;

		// Copy replacement
		memcpy(new_buffer + buff_pos, with, wlen);
		buff_pos += wlen;

		start_pos = FindIndex;
	}
	while (pos >= 0);
	new_buffer[buff_pos] = 0;
	delete [] Data;
	Data = new_buffer;
	Length = new_size;
	Size = new_size + 1;

	if (AutoHash)
		DataHash = IwHashString(Data);

	return pos;
}

void CIwGameString::Replace(char chr, char with)
{
	char*	str = Data;

	for (int t = 0; t < Length; t++)
	{
		if (*str == chr)
		{
			*str = with;
		}
		str++;
	}
}

bool CIwGameString::ContainsSpecialCharacters() const
{
	// Check for chance of special char
	bool	found = false;
	char*	str = Data;

	for (int t = 0; t < Length; t++)
	{
		char c = *str++;
		if (c == '&')
		{
			return true;
		}
	}

	return false;
}

int CIwGameString::Contains(char c) const
{
	char*	str = Data;

	for (int t = 0; t < Length; t++)
	{
		if (*str++ == c)
			return t;
	}

	return -1;
}

void CIwGameString::ReplaceHTMLCodes()
{
	if (!ContainsSpecialCharacters())
		return;

	char* str = Data;
	char* nstr = str;

	// Replace all instances of special tags with ascii codes
	int len = Length;
	for (int t = 0; t < len; t++)
	{
		char c = *str++;
		if (c == '&')
		{
			if (t < len - 3)
			{
				if (*str == 'l' && *(str + 1) == 't' && *(str + 2) == ';')
				{
					*nstr++ = '<';
					str += 3;  t += 3;
				}
				else
				if (*str == 'g' && *(str + 1) == 't' && *(str + 2) == ';')
				{
					*nstr++ = '>';
					str += 3;  t += 3;
				}
				else
				if (t < len - 4)
				{
					if (*str == 'a' && *(str + 1) == 'm' && *(str + 2) == 'p' && *(str + 3) == ';')
					{
						*nstr++ = '&';
						str += 4;  t += 4;
					}
					else
					if (t < len - 4)
					{
						if (*str == 'q' && *(str + 1) == 'u' && *(str + 2) == 'o' && *(str + 3) == 't' && *(str + 4) == ';')
						{
							*nstr++ = '"';
							str += 5;  t += 5;
						}
						else
						if (*str == 'a' && *(str + 1) == 'p' && *(str + 2) == 'o' && *(str + 3) == 's' && *(str + 4) == ';')
						{
							*nstr++ = '\'';
							str += 5;  t += 5;
						}
						else
							*nstr++ = c;
					}
					else
						*nstr++ = c;
				}
				else
					*nstr++ = c;
			}
			else
				*nstr++ = c;
		}
		else
			*nstr++ = c;
	}
	*nstr = 0;
	
	FindIndex = 0;
	Length = (int)nstr - (int)Data;
	
	if (AutoHash)
		DataHash = IwHashString(Data);
}

void CIwGameString::URLEncode(const char* str)
{
	int slen = strlen(str);
	if (slen == 0)
		return;
	char* new_data = new char [slen * 3 + 1];

	char hexstr[4];
	int len = 0;
	for (int t = 0; t < slen; t++)
	{
		unsigned int c = (unsigned int)*str++;
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c < '9') || c == '$' || c == '-' || c == '_' || c == '.' || c == '!' || c == '*' || c == '\'' || c == '(' || c == ')')
		{
			*(new_data + len) = c; len++;
		}
		else
		{
			snprintf(hexstr, 4, "%x", c);
			*(new_data + len) = '%'; len++;
			if (c < 16)
			{
				*(new_data + len) = '0'; len++;
				*(new_data + len) = hexstr[0]; len++;
			}
			else
			{
				*(new_data + len) = hexstr[0]; len++;
				*(new_data + len) = hexstr[1]; len++;
			}
		}
	}
	*(new_data + len) = 0;

	delete [] Data;
	FindIndex = 0;
	Data = new_data;
	Size = Length * 3 + 1;
	Length = len;

	if (AutoHash)
		DataHash = IwHashString(Data);
}

void CIwGameString::URLEncode()
{
	URLEncode(Data);
}

void CIwGameString::URLDecode()
{
	if (Data == NULL)
		return;

	int len = 0;
	char* new_data = new char [Length];
	char* str = Data;

	for (int t = 0; t < Length; t++)
	{
		unsigned int c = (unsigned int)*str++;
		if (c == '%')
		{
			int h1 = GetValueFromHexDigit(*str++);
			int h2 = GetValueFromHexDigit(*str++);
			*(new_data + len) = (h1 << 4) | h2;
		}
		else
		{
			*(new_data + len) = c;
		}
		len++;
	}
	*(new_data + len) = 0;

	delete [] Data;
	FindIndex = 0;
	Data = new_data;
	Size = Length;
	Length = len;
	if (AutoHash)
		DataHash = IwHashString(Data);
}

bool CIwGameString::SplitFilename(CIwGameString& filename, CIwGameString& ext)
{
	int index = 0;
	
	// Find the dot
	for (int t = GetLength() - 1; t != 0; t--)
	{
		if (*(Data + t) == '.')
		{
			index = t;
			break;
		}
	}
	if (index == 0) return false;
	
	filename.Copy(Data, 0, index);
	ext.Copy(Data, index + 1, GetLength() - index - 1);
	
	return true;
}

bool CIwGameString::GetFilenameExt(CIwGameString& ext)
{
	int index = 0;
	
	// Find the dot
	for (int t = GetLength() - 1; t != 0; t--)
	{
		if (*(Data + t) == '.')
		{
			index = t;
			break;
		}
	}
	if (index == 0) return false;
	
	ext.Copy(Data, index + 1, GetLength() - index - 1);
	
	return true;
}

void CIwGameString::ToUpper()
{
	char* pData = Data;
	for (int t = 0; t < Length; t++)
	{
		char c = *pData;
		if (IsLower(c))
		{
			c += 'A' - 'a';
			*pData++ = c;
		}
		else
			pData++;
	}
}

void CIwGameString::ToLower()
{
	char* pData = Data;
	for (int t = 0; t < Length; t++)
	{
		char c = *pData;
		if (IsUpper(c))
		{
			c += 'a' - 'A';
			*pData++ = c;
		}
		else
			pData++;
	}
}

int	CIwGameString::FindNext(const char* string, int len)
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	
	// Find first character
	char fc = *string;
	while (count != 0)
	{
		char c = *str++;
		if (c == fc)
		{
			// if number of chars left in this string is less than the length of the test string then it cant match
			if (count < len)
				return -1;

			// Check the rest of the string
			char*	str1 = str;
			char*	str2 = (char*)string + 1;
			int		count2 = len - 1;
			bool	mismatch = false;

			while (count2-- != 0)
			{
				if (*str1++ != *str2++)
				{
					mismatch = true;
					break;
				}
			}
			
			// A match was found
			if (!mismatch)
				return FindIndex;
		}
		FindIndex++;
		count--;
	}

	return -1;
}

int	CIwGameString::FindNext(char chr)
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	
	while (count != 0)
	{
		char c = *str++;
		if (c == chr)
			return FindIndex;
		FindIndex++;
		count--;
	}

	return -1;
}

int	CIwGameString::Find(const char* string)
{
	FindIndex = 0;
	int index = FindNext(string);
	FindIndex = 0;
	return index;
}

int	CIwGameString::FindNext(const char* string)
{
	return FindNext(string, strlen(string));
}

void CIwGameString::FindReset()
{
	FindIndex = 0;
}

int CIwGameString::StepFindIndex(int amount)
{
	FindIndex += amount;
	if (FindIndex < 0)
		FindIndex = 0;
	else
	if (FindIndex > Length)
		FindIndex = Length;

	return FindIndex;
}

int CIwGameString::StepFindIndexNoneWhiteSpace()
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	
	while (count != 0)
	{
		char c = *str++;
		if (c != ' ' && c != '\t')
			return FindIndex;
		FindIndex++;
		count--;
	}

	return -1;
}

int CIwGameString::StepFindIndexWhiteSpace()
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	
	while (count != 0)
	{
		char c = *str++;
		if (c == ' ' && c == '\t')
			return FindIndex;
		FindIndex++;
		count--;
	}

	return -1;
}

int CIwGameString::GetNextMarkerOffset(char marker)
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	
	int start = FindIndex;

	while (count-- != 0)
	{
		char c = *str++;
		if (c == marker || c == 0)
		{
			FindIndex++;					// Skip the marker for next search
			return FindIndex - start - 1;	// return length from start
		}

		FindIndex++;
	}

	return -1;
}

int CIwGameString::GetNextString()
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	int start = FindIndex;
	
	while (count-- != 0)
	{
		char c = *str++;
		if (c == ' ' || c == 0)
			return FindIndex - start;
			
		FindIndex++;
	}

	return FindIndex - start;
}

int CIwGameString::GetNextMarkedString(char start_mark, char end_mark, int &offset)
{
	int		count = Length - FindIndex;
	char*	str = Data + FindIndex;
	if (count <= 0)
		return -1;
	
	offset = -1;

	while (count-- != 0)
	{
		char c = *str++;
		if (offset < 0)
		{
			// Find start marker
			if (c == start_mark)
				offset = FindIndex + 1;
		}
		else
		{
			// Find end marker
			if (c == end_mark)
				return FindIndex - offset;
		}

		FindIndex++;
	}

	return -1;
}

int CIwGameString::GetNextMarkedStringAfterString(const char* search_string, char start_mark, char end_mark, CIwGameString& out_string)
{
	int pos = FindNext(search_string);
	if (pos >= 0)
	{
		// Get the text label
		int offset;
		StepFindIndex(strlen(search_string));
		int len = GetNextMarkedString(start_mark, end_mark, offset);
		if (len > 0)
		{
			out_string.setString(getString() + offset, len);
			return pos;
		}
		
		return -1;
	}

	return -1;
}
