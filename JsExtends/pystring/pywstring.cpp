///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008-2010, Sony Pictures Imageworks Inc
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// Neither the name of the organization Sony Pictures Imageworks nor the
// names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#include "pywstring.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS) || defined(_MSC_VER)
#ifndef WINDOWS
#define WINDOWS
#endif
#endif

// This definition codes from configure.in in the python src.
// Strictly speaking this limits us to str sizes of 2**31.
// Should we wish to handle this limit, we could use an architecture
// specific #defines and read from ssize_t (unistd.h) if the header exists.
// But in the meantime, the use of int assures maximum arch compatibility.
// This must also equal the size used in the end = MAX_32BIT_INT default arg.

typedef size_t Py_ssize_t;

/* helper macro to fixup start/end slice values */
#define ADJUST_INDICES(start, end, len)         \
    if (end > len)                          \
        end = len;                          \
    else if (end < 0) {                     \
        end += len;                         \
        if (end < 0)                        \
        end = 0;                        \
    }                                       \
    if (start < 0) {                        \
        start += len;                       \
        if (start < 0)                      \
        start = 0;                      \
    }


namespace {

	static inline int py_isspace(wchar_t c)
	{
		return c > 0 && ::iswspace(c);
	}

	static inline int py_islower(wchar_t c)
	{
		return c > 0 && ::iswlower(c);
	}

	static inline int py_isupper(wchar_t c)
	{
		return c > 0 && ::iswupper(c);
	}

	static inline int py_isdigit(wchar_t c)
	{
		return c > 0 && ::iswdigit(c);
	}

	static inline int py_isalnum(wchar_t c)
	{
		return c > 0 && ::iswalnum(c);
	}

	static inline int py_isalpha(wchar_t c)
	{
		return c > 0 && ::iswalpha(c);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	/// why doesn't the std::reverse work?
	///
	static void reverse_strings(std::vector< std::wstring > & result)
	{
		for (std::vector< std::wstring >::size_type i = 0; i < result.size() / 2; i++)
		{
			std::swap(result[i], result[result.size() - 1 - i]);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////
	///
	///
	static void split_whitespace(const std::wstring & str, std::vector< std::wstring > & result, int maxsplit)
	{
		std::wstring::size_type i, j, len = str.size();
		for (i = j = 0; i < len; )
		{

			while (i < len && py_isspace(str[i])) i++;
			j = i;

			while (i < len && !py_isspace(str[i])) i++;



			if (j < i)
			{
				if (maxsplit-- <= 0) break;

				result.push_back(str.substr(j, i - j));

				while (i < len && py_isspace(str[i])) i++;
				j = i;
			}
		}
		if (j < len)
		{
			result.push_back(str.substr(j, len - j));
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	///
	///
	static void rsplit_whitespace(const std::wstring & str, std::vector< std::wstring > & result, int maxsplit)
	{
		std::wstring::size_type len = str.size();
		std::wstring::size_type i, j;
		for (i = j = len; i > 0; )
		{

			while (i > 0 && py_isspace(str[i - 1])) i--;
			j = i;

			while (i > 0 && !py_isspace(str[i - 1])) i--;



			if (j > i)
			{
				if (maxsplit-- <= 0) break;

				result.push_back(str.substr(i, j - i));

				while (i > 0 && py_isspace(str[i - 1])) i--;
				j = i;
			}
		}
		if (j > 0)
		{
			result.push_back(str.substr(0, j));
		}
		//std::reverse( result, result.begin(), result.end() );
		reverse_strings(result);
	}
} //anonymous namespace


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void pywstring::split(const std::wstring & str, std::vector< std::wstring > & result, const std::wstring & sep, int maxsplit)
{
	result.clear();

	if (maxsplit < 0) maxsplit = MAX_32BIT_INT;//result.max_size();


	if (sep.size() == 0)
	{
		split_whitespace(str, result, maxsplit);
		return;
	}

	std::wstring::size_type i, j, len = str.size(), n = sep.size();

	i = j = 0;

	while (i + n <= len)
	{
		if (str[i] == sep[0] && str.substr(i, n) == sep)
		{
			if (maxsplit-- <= 0) break;

			result.push_back(str.substr(j, i - j));
			i = j = i + n;
		}
		else
		{
			i++;
		}
	}

	result.push_back(str.substr(j, len - j));
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void pywstring::rsplit(const std::wstring & str, std::vector< std::wstring > & result, const std::wstring & sep, int maxsplit)
{
	if (maxsplit < 0)
	{
		split(str, result, sep, maxsplit);
		return;
	}

	result.clear();

	if (sep.size() == 0)
	{
		rsplit_whitespace(str, result, maxsplit);
		return;
	}

	Py_ssize_t i, j, len = (Py_ssize_t)str.size(), n = (Py_ssize_t)sep.size();

	i = j = len;

	while (i >= n)
	{
		if (str[i - 1] == sep[n - 1] && str.substr(i - n, n) == sep)
		{
			if (maxsplit-- <= 0) break;

			result.push_back(str.substr(i, j - i));
			i = j = i - n;
		}
		else
		{
			i--;
		}
	}

	result.push_back(str.substr(0, j));
	reverse_strings(result);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
#define LEFTSTRIP 0
#define RIGHTSTRIP 1
#define BOTHSTRIP 2

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
static std::wstring do_strip(const std::wstring & str, int striptype, const std::wstring & chars)
{
	Py_ssize_t len = (Py_ssize_t)str.size(), i, j, charslen = (Py_ssize_t)chars.size();
	if (len == 0)
		return str;

	if (charslen == 0)
	{
		i = 0;
		if (striptype != RIGHTSTRIP)
		{
			while (i < len && py_isspace(str[i]))
			{
				i++;
			}
		}

		j = len;
		if (striptype != LEFTSTRIP)
		{
			do
			{
				j--;
			} while (j >= i && py_isspace(str[j]));

			j++;
		}


	}
	else
	{
		const wchar_t * sep = chars.c_str();

		i = 0;
		if (striptype != RIGHTSTRIP)
		{
			while (i < len && wmemchr(sep, str[i], charslen))
			{
				i++;
			}
		}

		j = len;
		if (striptype != LEFTSTRIP)
		{
			do
			{
				j--;
			} while (j >= i &&  wmemchr(sep, str[j], charslen));
			j++;
		}


	}

	if (i == 0 && j == len)
	{
		return str;
	}
	else
	{
		return str.substr(i, j - i);
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void pywstring::partition(const std::wstring & str, const std::wstring & sep, std::vector< std::wstring > & result)
{
	result.resize(3);
	int index = find(str, sep);
	if (index < 0)
	{
		result[0] = str;
		result[1] = L"";
		result[2] = L"";
	}
	else
	{
		result[0] = str.substr(0, index);
		result[1] = sep;
		result[2] = str.substr(index + sep.size(), str.size());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void pywstring::rpartition(const std::wstring & str, const std::wstring & sep, std::vector< std::wstring > & result)
{
	result.resize(3);
	int index = rfind(str, sep);
	if (index < 0)
	{
		result[0] = L"";
		result[1] = L"";
		result[2] = str;
	}
	else
	{
		result[0] = str.substr(0, index);
		result[1] = sep;
		result[2] = str.substr(index + sep.size(), str.size());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::strip(const std::wstring & str, const std::wstring & chars)
{
	return do_strip(str, BOTHSTRIP, chars);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::lstrip(const std::wstring & str, const std::wstring & chars)
{
	return do_strip(str, LEFTSTRIP, chars);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::rstrip(const std::wstring & str, const std::wstring & chars)
{
	return do_strip(str, RIGHTSTRIP, chars);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::join(const std::wstring & str, const std::vector< std::wstring > & seq)
{
	std::vector< std::wstring >::size_type seqlen = seq.size(), i;

	if (seqlen == 0) return L"";
	if (seqlen == 1) return seq[0];

	std::wstring result(seq[0]);

	for (i = 1; i < seqlen; ++i)
	{
		result += str + seq[i];

	}


	return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

namespace
{
	/* Matches the end (direction >= 0) or start (direction < 0) of self
	 * against substr, using the start and end arguments. Returns
	 * -1 on error, 0 if not found and 1 if found.
	 */

	static int _string_tailmatch(const std::wstring & self, const std::wstring & substr,
		Py_ssize_t start, Py_ssize_t end,
		int direction)
	{
		Py_ssize_t len = (Py_ssize_t)self.size();
		Py_ssize_t slen = (Py_ssize_t)substr.size();

		const wchar_t* sub = substr.c_str();
		const wchar_t* str = self.c_str();

		ADJUST_INDICES(start, end, len);

		if (direction < 0) {
			// startswith
			if (start + slen > len)
				return 0;
		}
		else {
			// endswith
			if (end - start < slen || start > len)
				return 0;
			if (end - slen > start)
				start = end - slen;
		}
		if (end - start >= slen)
			return (!std::wmemcmp(str + start, sub, slen));

		return 0;
	}
}

bool pywstring::endswith(const std::wstring & str, const std::wstring & suffix, int start, int end)
{
	int result = _string_tailmatch(str, suffix,
		(Py_ssize_t)start, (Py_ssize_t)end, +1);
	//if (result == -1) // TODO: Error condition

	return static_cast<bool>(result);
}


bool pywstring::startswith(const std::wstring & str, const std::wstring & prefix, int start, int end)
{
	int result = _string_tailmatch(str, prefix,
		(Py_ssize_t)start, (Py_ssize_t)end, -1);
	//if (result == -1) // TODO: Error condition

	return static_cast<bool>(result);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

bool pywstring::isalnum(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;
	if (len == 0) return false;


	if (len == 1)
	{
		return py_isalnum(str[0]);
	}

	for (i = 0; i < len; ++i)
	{
		if (!py_isalnum(str[i])) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool pywstring::isalpha(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;
	if (len == 0) return false;
	if (len == 1) return py_isalpha((int)str[0]);

	for (i = 0; i < len; ++i)
	{
		if (!py_isalpha((int)str[i])) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool pywstring::isdigit(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;
	if (len == 0) return false;
	if (len == 1) return py_isdigit(str[0]);

	for (i = 0; i < len; ++i)
	{
		if (!py_isdigit(str[i])) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool pywstring::islower(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;
	if (len == 0) return false;
	if (len == 1) return py_islower(str[0]);

	for (i = 0; i < len; ++i)
	{
		if (!py_islower(str[i])) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool pywstring::isspace(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;
	if (len == 0) return false;
	if (len == 1) return py_isspace(str[0]);

	for (i = 0; i < len; ++i)
	{
		if (!py_isspace(str[i])) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool pywstring::istitle(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;

	if (len == 0) return false;
	if (len == 1) return py_isupper(str[0]);

	bool cased = false, previous_is_cased = false;

	for (i = 0; i < len; ++i)
	{
		if (py_isupper(str[i]))
		{
			if (previous_is_cased)
			{
				return false;
			}

			previous_is_cased = true;
			cased = true;
		}
		else if (py_islower(str[i]))
		{
			if (!previous_is_cased)
			{
				return false;
			}

			previous_is_cased = true;
			cased = true;

		}
		else
		{
			previous_is_cased = false;
		}
	}

	return cased;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
bool pywstring::isupper(const std::wstring & str)
{
	std::wstring::size_type len = str.size(), i;
	if (len == 0) return false;
	if (len == 1) return py_isupper(str[0]);

	for (i = 0; i < len; ++i)
	{
		if (!py_isupper(str[i])) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::capitalize(const std::wstring & str)
{
	std::wstring s(str);
	std::wstring::size_type len = s.size(), i;

	if (len > 0)
	{
		if (py_islower(s[0])) s[0] = (wchar_t) ::towupper(s[0]);
	}

	for (i = 1; i < len; ++i)
	{
		if (py_isupper(s[i])) s[i] = (wchar_t) ::towlower(s[i]);
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::lower(const std::wstring & str)
{
	std::wstring s(str);
	std::wstring::size_type len = s.size(), i;

	for (i = 0; i < len; ++i)
	{
		if (py_isupper(s[i])) s[i] = (wchar_t) ::towlower(s[i]);
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::upper(const std::wstring & str)
{
	std::wstring s(str);
	std::wstring::size_type len = s.size(), i;

	for (i = 0; i < len; ++i)
	{
		if (py_islower(s[i])) s[i] = (wchar_t) ::towupper(s[i]);
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::swapcase(const std::wstring & str)
{
	std::wstring s(str);
	std::wstring::size_type len = s.size(), i;

	for (i = 0; i < len; ++i)
	{
		if (py_islower(s[i])) s[i] = (wchar_t) ::towupper(s[i]);
		else if (py_isupper(s[i])) s[i] = (wchar_t) ::towlower(s[i]);
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::title(const std::wstring & str)
{
	std::wstring s(str);
	std::wstring::size_type len = s.size(), i;
	bool previous_is_cased = false;

	for (i = 0; i < len; ++i)
	{
		int c = s[i];
		if (py_islower(c))
		{
			if (!previous_is_cased)
			{
				s[i] = (wchar_t) ::towupper(c);
			}
			previous_is_cased = true;
		}
		else if (py_isupper(c))
		{
			if (previous_is_cased)
			{
				s[i] = (wchar_t) ::towlower(c);
			}
			previous_is_cased = true;
		}
		else
		{
			previous_is_cased = false;
		}
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::translate(const std::wstring & str, const std::wstring & table, const std::wstring & deletechars)
{
	std::wstring s;
	std::wstring::size_type len = str.size(), dellen = deletechars.size();

	if (table.size() != 256)
	{
		// TODO : raise exception instead
		return str;
	}

	//if nothing is deleted, use faster code
	if (dellen == 0)
	{
		s = str;
		for (std::wstring::size_type i = 0; i < len; ++i)
		{
			s[i] = table[s[i]];
		}
		return s;
	}


	int trans_table[256];
	for (int i = 0; i < 256; i++)
	{
		trans_table[i] = table[i];
	}

	for (std::wstring::size_type i = 0; i < dellen; i++)
	{
		trans_table[(int)deletechars[i]] = -1;
	}

	for (std::wstring::size_type i = 0; i < len; ++i)
	{
		if (trans_table[(int)str[i]] != -1)
		{
			s += table[str[i]];
		}
	}

	return s;

}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::zfill(const std::wstring & str, int width, wchar_t fillChar/* = '0'*/)
{
	int len = (int)str.size();

	if (len >= width)
	{
		return str;
	}

	std::wstring s(str);

	int fill = width - len;

	s = std::wstring(fill, fillChar) + s;


	if (s[fill] == '+' || s[fill] == '-')
	{
		s[0] = s[fill];
		s[fill] = fillChar;
	}

	return s;

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::ljust(const std::wstring & str, int width)
{
	std::wstring::size_type len = str.size();
	if (((int)len) >= width) return str;
	return str + std::wstring(width - len, ' ');
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::rjust(const std::wstring & str, int width)
{
	std::wstring::size_type len = str.size();
	if (((int)len) >= width) return str;
	return std::wstring(width - len, ' ') + str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::center(const std::wstring & str, int width)
{
	int len = (int)str.size();
	int marg, left;

	if (len >= width) return str;

	marg = width - len;
	left = marg / 2 + (marg & width & 1);

	return std::wstring(left, ' ') + str + std::wstring(marg - left, ' ');

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::slice(const std::wstring & str, int start, int end)
{
	ADJUST_INDICES(start, end, (int)str.size());
	if (start >= end) return L"";
	if (str.size() <= start) return L"";
	return str.substr(start, end - start);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::alignment(const std::wstring& str, int maxLen, int align /*= 0*/, wchar_t fillChar /*= ' ' */)
{
	int fillLen = maxLen - str.length();
	if (fillLen < 1)
		return str;

	if (align == 0)//left alignment
	{
		return str + std::wstring(fillLen, fillChar);
	}
	else if (align == 1)//center alignment 
	{
		std::wstring sRet;
		int left = (int)fillLen / (int)2;
		if (left > 0)
			sRet += std::wstring(left, fillChar);
		sRet += str;
		int right = fillLen - left;
		if (right > 0)
			sRet += std::wstring(right, fillChar);
		return sRet;
	}

	return str;
}

bool pywstring::iscempty(const std::wstring& str)
{
	return (length(str) == 0);
}

bool pywstring::equal(const std::wstring& str1, const std::wstring& str2, bool ignoreCase/* = false*/)
{
	if (ignoreCase)
		return _wcsicmp(str1.c_str(), str2.c_str()) == 0;
	else
		return wcscmp(str1.c_str(), str2.c_str()) == 0;
}

std::wstring pywstring::concat(const std::wstring& left, const std::wstring& right)
{
	return std::wstring(left.c_str()) + std::wstring(right.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int pywstring::find(const std::wstring & str, const std::wstring & sub, int start, int end)
{
	ADJUST_INDICES(start, end, (int)str.size());

	std::wstring::size_type result = str.find(sub, start);

	// If we cannot find the string, or if the end-point of our found substring is past
	// the allowed end limit, return that it can't be found.
	if (result == std::wstring::npos ||
		(result + sub.size() > (std::wstring::size_type)end))
	{
		return -1;
	}

	return (int)result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int pywstring::index(const std::wstring & str, const std::wstring & sub, int start, int end)
{
	return find(str, sub, start, end);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int pywstring::rfind(const std::wstring & str, const std::wstring & sub, int start, int end)
{
	ADJUST_INDICES(start, end, (int)str.size());

	std::wstring::size_type result = str.rfind(sub, end);

	if (result == std::wstring::npos ||
		result < (std::wstring::size_type)start ||
		(result + sub.size() > (std::wstring::size_type)end))
		return -1;

	return (int)result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int pywstring::rindex(const std::wstring & str, const std::wstring & sub, int start, int end)
{
	return rfind(str, sub, start, end);
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::expandtabs(const std::wstring & str, int tabsize)
{
	std::wstring s(str);

	std::wstring::size_type len = str.size(), i = 0;
	int offset = 0;

	int j = 0;

	for (i = 0; i < len; ++i)
	{
		if (str[i] == '\t')
		{

			if (tabsize > 0)
			{
				int fillsize = tabsize - (j % tabsize);
				j += fillsize;
				s.replace(i + offset, 1, std::wstring(fillsize, ' '));
				offset += fillsize - 1;
			}
			else
			{
				s.replace(i + offset, 1, L"");
				offset -= 1;
			}

		}
		else
		{
			j++;

			if (str[i] == '\n' || str[i] == '\r')
			{
				j = 0;
			}
		}
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
int pywstring::count(const std::wstring & str, const std::wstring & substr, int start, int end)
{
	int nummatches = 0;
	int cursor = start;

	while (1)
	{
		cursor = find(str, substr, cursor, end);

		if (cursor < 0) break;

		cursor += (int)substr.size();
		nummatches += 1;
	}

	return nummatches;


}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::wstring pywstring::replace(const std::wstring & str, const std::wstring & oldstr, const std::wstring & newstr, int count)
{
	int sofar = 0;
	int cursor = 0;
	std::wstring s(str);

	std::wstring::size_type oldlen = oldstr.size(), newlen = newstr.size();

	cursor = find(s, oldstr, cursor);

	while (cursor != -1 && cursor <= (int)s.size())
	{
		if (count > -1 && sofar >= count)
		{
			break;
		}

		s.replace(cursor, oldlen, newstr);
		cursor += (int)newlen;

		if (oldlen != 0)
		{
			cursor = find(s, oldstr, cursor);
		}
		else
		{
			++cursor;
		}

		++sofar;
	}

	return s;

}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///
void pywstring::splitlines(const std::wstring & str, std::vector< std::wstring > & result, bool keepends)
{
	result.clear();
	std::wstring::size_type len = str.size(), i, j, eol;

	for (i = j = 0; i < len; )
	{
		while (i < len && str[i] != '\n' && str[i] != '\r') i++;

		eol = i;
		if (i < len)
		{
			if (str[i] == '\r' && i + 1 < len && str[i + 1] == '\n')
			{
				i += 2;
			}
			else
			{
				i++;
			}
			if (keepends)
				eol = i;

		}

		result.push_back(str.substr(j, eol - j));
		j = i;

	}

	if (j < len)
	{
		result.push_back(str.substr(j, len - j));
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///
std::wstring pywstring::mul(const std::wstring & str, int n)
{
	// Early exits
	if (n <= 0) return L"";
	if (n == 1) return str;

	std::wostringstream os;
	for (int i = 0; i < n; ++i)
	{
		os << str;
	}
	return os.str();
}



//////////////////////////////////////////////////////////////////////////////////////////////
///
///
/// These functions are C++ ports of the python2.6 versions of os.path,
/// and come from genericpath.py, ntpath.py, posixpath.py

/// Split a pathname into drive and path specifiers.
/// Returns drivespec, pathspec. Either part may be empty.
void os_pathw::splitdrive_nt(std::wstring & drivespec, std::wstring & pathspec,
	const std::wstring & p)
{
	if (pywstring::slice(p, 1, 2) == L":")
	{
		std::wstring path = p; // In case drivespec == p
		drivespec = pywstring::slice(path, 0, 2);
		pathspec = pywstring::slice(path, 2);
	}
	else
	{
		drivespec = L"";
		pathspec = p;
	}
}

// On Posix, drive is always empty
void os_pathw::splitdrive_posix(std::wstring & drivespec, std::wstring & pathspec,
	const std::wstring & path)
{
	drivespec = L"";
	pathspec = path;
}

void os_pathw::splitdrive(std::wstring & drivespec, std::wstring & pathspec,
	const std::wstring & path)
{
#ifdef WINDOWS
	return splitdrive_nt(drivespec, pathspec, path);
#else
	return splitdrive_posix(drivespec, pathspec, path);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

// Test whether a path is absolute
// In windows, if the character to the right of the colon
// is a forward or backslash it's absolute.
bool os_pathw::isabs_nt(const std::wstring & path)
{
	std::wstring drivespec, pathspec;
	splitdrive_nt(drivespec, pathspec, path);
	if (pathspec.empty()) return false;
	return ((pathspec[0] == '/') || (pathspec[0] == '\\'));
}

bool os_pathw::isabs_posix(const std::wstring & s)
{
	return pywstring::startswith(s, L"/");
}

bool os_pathw::isabs(const std::wstring & path)
{
#ifdef WINDOWS
	return isabs_nt(path);
#else
	return isabs_posix(path);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::wstring os_pathw::abspath_nt(const std::wstring & path, const std::wstring & cwd)
{
	std::wstring p = path;
	if (!isabs_nt(p)) p = join_nt(cwd, p);
	return normpath_nt(p);
}

std::wstring os_pathw::abspath_posix(const std::wstring & path, const std::wstring & cwd)
{
	std::wstring p = path;
	if (!isabs_posix(p)) p = join_posix(cwd, p);
	return normpath_posix(p);
}

std::wstring os_pathw::abspath(const std::wstring & path, const std::wstring & cwd)
{
#ifdef WINDOWS
	return abspath_nt(path, cwd);
#else
	return abspath_posix(path, cwd);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::wstring os_pathw::join_nt(const std::vector< std::wstring > & paths)
{
	if (paths.empty()) return L"";
	if (paths.size() == 1) return paths[0];

	std::wstring path = paths[0];

	for (unsigned int i = 1; i < paths.size(); ++i)
	{
		std::wstring b = paths[i];

		bool b_nts = false;
		if (path.empty())
		{
			b_nts = true;
		}
		else if (isabs_nt(b))
		{
			// This probably wipes out path so far.  However, it's more
			// complicated if path begins with a drive letter:
			//     1. join('c:', '/a') == 'c:/a'
			//     2. join('c:/', '/a') == 'c:/a'
			// But
			//     3. join('c:/a', '/b') == '/b'
			//     4. join('c:', 'd:/') = 'd:/'
			//     5. join('c:/', 'd:/') = 'd:/'

			if ((pywstring::slice(path, 1, 2) != L":") ||
				(pywstring::slice(b, 1, 2) == L":"))
			{
				// Path doesnt start with a drive letter
				b_nts = true;
			}
			// Else path has a drive letter, and b doesn't but is absolute.
			else if ((path.size() > 3) ||
				((path.size() == 3) && !pywstring::endswith(path, L"/") && !pywstring::endswith(path, L"\\")))
			{
				b_nts = true;
			}
		}

		if (b_nts)
		{
			path = b;
		}
		else
		{
			// Join, and ensure there's a separator.
			// assert len(path) > 0
			if (pywstring::endswith(path, L"/") || pywstring::endswith(path, L"\\"))
			{
				if (pywstring::startswith(b, L"/") || pywstring::startswith(b, L"\\"))
				{
					path += pywstring::slice(b, 1);
				}
				else
				{
					path += b;
				}
			}
			else if (pywstring::endswith(path, L":"))
			{
				path += L"\\" + b;
			}
			else if (!b.empty())
			{
				if (pywstring::startswith(b, L"/") || pywstring::startswith(b, L"\\"))
				{
					path += b;
				}
				else
				{
					path += L"\\" + b;
				}
			}
			else
			{
				// path is not empty and does not end with a backslash,
				// but b is empty; since, e.g., split('a/') produces
				// ('a', ''), it's best if join() adds a backslash in
				// this case.
				path += L"\\";
			}
		}
	}

	return path;
}

// Join two or more pathname components, inserting "\\" as needed.
std::wstring os_pathw::join_nt(const std::wstring & a, const std::wstring & b)
{
	std::vector< std::wstring > paths(2);
	paths[0] = a;
	paths[1] = b;
	return join_nt(paths);
}

// Join pathnames.
// If any component is an absolute path, all previous path components
// will be discarded.
// Ignore the previous parts if a part is absolute.
// Insert a '/' unless the first part is empty or already ends in '/'.

std::wstring os_pathw::join_posix(const std::vector< std::wstring > & paths)
{
	if (paths.empty()) return L"";
	if (paths.size() == 1) return paths[0];

	std::wstring path = paths[0];

	for (unsigned int i = 1; i < paths.size(); ++i)
	{
		std::wstring b = paths[i];
		if (pywstring::startswith(b, L"/"))
		{
			path = b;
		}
		else if (path.empty() || pywstring::endswith(path, L"/"))
		{
			path += b;
		}
		else
		{
			path += L"/" + b;
		}
	}

	return path;
}

std::wstring os_pathw::join_posix(const std::wstring & a, const std::wstring & b)
{
	std::vector< std::wstring > paths(2);
	paths[0] = a;
	paths[1] = b;
	return join_posix(paths);
}

std::wstring os_pathw::join(const std::wstring & path1, const std::wstring & path2)
{
#ifdef WINDOWS
	return join_nt(path1, path2);
#else
	return join_posix(path1, path2);
#endif
}


std::wstring os_pathw::join(const std::vector< std::wstring > & paths)
{
#ifdef WINDOWS
	return join_nt(paths);
#else
	return join_posix(paths);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///


// Split a pathname.
// Return (head, tail) where tail is everything after the final slash.
// Either part may be empty

void os_pathw::split_nt(std::wstring & head, std::wstring & tail, const std::wstring & path)
{
#if 0
	std::wstring d, p;
	splitdrive_nt(d, p, path);

	// set i to index beyond p's last slash
	int i = (int)p.size();

	while (i > 0 && (p[i - 1] != '\\') && (p[i - 1] != '/'))
	{
		i = i - 1;
	}

	head = pywstring::slice(p, 0, i);
	tail = pywstring::slice(p, i); // now tail has no slashes

	// remove trailing slashes from head, unless it's all slashes
	std::wstring head2 = head;
	while (!head2.empty() && ((pywstring::slice(head2, -1) == L"/") ||
		(pywstring::slice(head2, -1) == L"\\")))
	{
		head2 = pywstring::slice(head2, 0, -1);
	}

	if (!head2.empty()) head = head2;
	head = d + head;
#else
	int nLen = (int)pywstring::length(path);
	while (nLen > 0 && (path[nLen - 1] == '\\' || path[nLen - 1] == '/'))
	{
		--nLen;
	}
	if (nLen <= 0)
	{
		head = L"";
		tail = L"";
		return;
	}

	size_t idx1 = path.rfind('\\', nLen - 1);
	if (idx1 == std::string::npos)
	{
		idx1 = path.rfind('/', nLen - 1);
	}
	else
	{
		size_t idx2 = path.rfind('/', nLen - 1);
		if (idx2 != std::string::npos && idx2 > idx1)
		{
			idx1 = idx2;
		}
	}

	if (idx1 == std::string::npos)
	{
		if (path.find(':') == std::string::npos)
		{
			head = L"";
			tail = path.substr(0, nLen);
		}
		else
		{
			head = path.substr(0, nLen);
			tail = L"";
		}
		return;
	}

	head = path.substr(0, idx1);
	tail = path.substr(idx1 + 1, nLen - (idx1 + 1));
#endif
}


// Split a path in head (everything up to the last '/') and tail (the
// rest).  If the path ends in '/', tail will be empty.  If there is no
// '/' in the path, head  will be empty.
// Trailing '/'es are stripped from head unless it is the root.

void os_pathw::split_posix(std::wstring & head, std::wstring & tail, const std::wstring & path)
{
#if 0
	int i = pywstring::rfind(p, L"/") + 1;

	head = pywstring::slice(p, 0, i);
	tail = pywstring::slice(p, i);

	if (!head.empty() && (head != pywstring::mul(L"/", (int)head.size())))
	{
		head = pywstring::rstrip(head, L"/");
	}
#else
	int nLen = (int)pywstring::length(path);
	while (nLen > 0 && path[nLen - 1] == '/')
	{
		--nLen;
	}
	if (nLen <= 0)
	{
		head = L"";
		tail = L"";
		return;
	}

	size_t idx1 = path.rfind('/', nLen - 1);
	if (idx1 == std::string::npos)
	{
		head = L"";
		tail = path.substr(0, nLen);
		return;
	}

	head = path.substr(0, idx1);
	tail = path.substr(idx1 + 1, nLen - (idx1 + 1));
#endif
}

void os_pathw::split(std::wstring & head, std::wstring & tail, const std::wstring & path)
{
#ifdef WINDOWS
	return split_nt(head, tail, path);
#else
	return split_posix(head, tail, path);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

std::wstring os_pathw::basename_nt(const std::wstring & path)
{
	std::wstring head, tail;
	split_nt(head, tail, path);
	return tail;
}

std::wstring os_pathw::basename_posix(const std::wstring & path)
{
	std::wstring head, tail;
	split_posix(head, tail, path);
	return tail;
}

std::wstring os_pathw::basename_no_ext(const std::wstring & path)
{
	if (pywstring::endswith(path, L"\\") || pywstring::endswith(path, L"/"))
		return L"";

	std::wstring bn = basename(path);
	size_t idx = bn.find_last_of('.');
	if (idx != std::wstring::npos)
		return bn.substr(0, idx);
	else
		return bn;
}

std::wstring os_pathw::extension(const std::wstring & path)
{
	if (pywstring::endswith(path, L"\\") || pywstring::endswith(path, L"/"))
		return L"";

	size_t idx = path.find_last_of('.');
	if (idx == std::wstring::npos)
		return L"";

	std::wstring ext = path.substr(idx + 1);
	if (ext.find('/') != std::wstring::npos || ext.find('\\') != std::wstring::npos)
		return L"";
	return ext;
}

std::wstring os_pathw::basename(const std::wstring & path)
{
#ifdef WINDOWS
	return basename_nt(path);
#else
	return basename_posix(path);
#endif
}

std::wstring os_pathw::dirname_nt(const std::wstring & path)
{
	std::wstring head, tail;
	split_nt(head, tail, path);
	return head;
}

std::wstring os_pathw::dirname_posix(const std::wstring & path)
{
	std::wstring head, tail;
	split_posix(head, tail, path);
	return head;
}

std::wstring os_pathw::dirname(const std::wstring & path)
{
#ifdef WINDOWS
	return dirname_nt(path);
#else
	return dirname_posix(path);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////
///
///

// Normalize a path, e.g. A//B, A/./B and A/foo/../B all become A\B.
std::wstring os_pathw::normpath_nt(const std::wstring & p)
{
	std::wstring path = p;
	path = pywstring::replace(path, L"/", L"\\");

	std::wstring prefix;
	splitdrive_nt(prefix, path, path);

	// We need to be careful here. If the prefix is empty, and the path starts
	// with a backslash, it could either be an absolute path on the current
	// drive (\dir1\dir2\file) or a UNC filename (\\server\mount\dir1\file). It
	// is therefore imperative NOT to collapse multiple backslashes blindly in
	// that case.
	// The code below preserves multiple backslashes when there is no drive
	// letter. This means that the invalid filename \\\a\b is preserved
	// unchanged, where a\\\b is normalised to a\b. It's not clear that there
	// is any better behaviour for such edge cases.

	if (prefix.empty())
	{
		// No drive letter - preserve initial backslashes
		while (pywstring::slice(path, 0, 1) == L"\\")
		{
			prefix = prefix + L"\\";
			path = pywstring::slice(path, 1);
		}
	}
	else
	{
		// We have a drive letter - collapse initial backslashes
		if (pywstring::startswith(path, L"\\"))
		{
			prefix = prefix + L"\\";
			path = pywstring::lstrip(path, L"\\");
		}
	}

	std::vector<std::wstring> comps;
	pywstring::split(path, comps, L"\\");

	int i = 0;

	while (i < (int)comps.size())
	{
		if (comps[i].empty() || comps[i] == L".")
		{
			comps.erase(comps.begin() + i);
		}
		else if (comps[i] == L"..")
		{
			if (i > 0 && comps[i - 1] != L"..")
			{
				comps.erase(comps.begin() + i - 1, comps.begin() + i + 1);
				i -= 1;
			}
			else if (i == 0 && pywstring::endswith(prefix, L"\\"))
			{
				comps.erase(comps.begin() + i);
			}
			else
			{
				i += 1;
			}
		}
		else
		{
			i += 1;
		}
	}

	// If the path is now empty, substitute '.'
	if (prefix.empty() && comps.empty())
	{
		comps.push_back(L".");
	}

	return prefix + pywstring::join(L"\\", comps);
}

// Normalize a path, e.g. A//B, A/./B and A/foo/../B all become A/B.
// It should be understood that this may change the meaning of the path
// if it contains symbolic links!
// Normalize path, eliminating double slashes, etc.

std::wstring os_pathw::normpath_posix(const std::wstring & p)
{
	if (p.empty()) return L".";

	std::wstring path = p;

	int initial_slashes = pywstring::startswith(path, L"/") ? 1 : 0;

	// POSIX allows one or two initial slashes, but treats three or more
	// as single slash.

	if (initial_slashes && pywstring::startswith(path, L"//")
		&& !pywstring::startswith(path, L"///"))
		initial_slashes = 2;

	std::vector<std::wstring> comps, new_comps;
	pywstring::split(path, comps, L"/");

	for (unsigned int i = 0; i < comps.size(); ++i)
	{
		std::wstring comp = comps[i];
		if (comp.empty() || comp == L".")
			continue;

		if ((comp != L"..") || ((initial_slashes == 0) && new_comps.empty()) ||
			(!new_comps.empty() && new_comps[new_comps.size() - 1] == L".."))
		{
			new_comps.push_back(comp);
		}
		else if (!new_comps.empty())
		{
			new_comps.pop_back();
		}
	}

	path = pywstring::join(L"/", new_comps);

	if (initial_slashes > 0)
		path = pywstring::mul(L"/", initial_slashes) + path;

	if (path.empty()) return L".";
	return path;
}

bool os_pathw::equal_path(const std::wstring & path1, const std::wstring & path2)
{
#ifdef WINDOWS
	return equal_path_nt(path1, path2);
#else
	return equal_path_posix(path1, path2);
#endif
}

bool os_pathw::equal_path_nt(const std::wstring & path1, const std::wstring & path2)
{	
	std::wstring _path1 = normpath(path1);
	std::wstring _path2 = normpath(path2);

	if (pywstring::endswith(_path1, L"\\"))
		_path1[_path1.size()-1] = '\0';
	if (pywstring::endswith(_path2, L"\\"))
		_path2[_path2.size()-1] = '\0';

	_path1 = pywstring::lower(_path1);
	_path2 = pywstring::lower(_path2);

	return pywstring::equal(_path1, _path2);
}

bool os_pathw::equal_path_posix(const std::wstring & path1, const std::wstring & path2)
{
	std::wstring _path1 = normpath(path1);
	std::wstring _path2 = normpath(path2);

	if (pywstring::endswith(_path1, L"/"))
		_path1[_path1.size()-1] = '\0';
	if (pywstring::endswith(_path2, L"/"))
		_path2[_path2.size()-1] = '\0';

	return pywstring::equal(_path1, _path2);
}

std::wstring os_pathw::normpath(const std::wstring & path)
{
#ifdef WINDOWS
	return normpath_nt(path);
#else
	return normpath_posix(path);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///
///

// Split the extension from a pathname.
// Extension is everything from the last dot to the end, ignoring
// leading dots.  Returns "(root, ext)"; ext may be empty.
// It is always true that root + ext == p

static void splitext_generic(std::wstring & root, std::wstring & ext,
	const std::wstring & p,
	const std::wstring & sep,
	const std::wstring & altsep,
	const std::wstring & extsep)
{
	int sepIndex = pywstring::rfind(p, sep);
	if (!altsep.empty())
	{
		int altsepIndex = pywstring::rfind(p, altsep);
		sepIndex = std::max<>(sepIndex, altsepIndex);
	}

	int dotIndex = pywstring::rfind(p, extsep);
	if (dotIndex > sepIndex)
	{
		// Skip all leading dots
		int filenameIndex = sepIndex + 1;

		while (filenameIndex < dotIndex)
		{
			if (pywstring::slice(p, filenameIndex) != extsep)
			{
				root = pywstring::slice(p, 0, dotIndex);
				ext = pywstring::slice(p, dotIndex);
				return;
			}

			filenameIndex += 1;
		}
	}

	root = p;
	ext = L"";
}

void os_pathw::splitext_nt(std::wstring & root, std::wstring & ext, const std::wstring & path)
{
	return splitext_generic(root, ext, path,
		L"\\", L"/", L".");
}

void os_pathw::splitext_posix(std::wstring & root, std::wstring & ext, const std::wstring & path)
{
	return splitext_generic(root, ext, path,
		L"/", L"", L".");
}

void os_pathw::splitext(std::wstring & root, std::wstring & ext, const std::wstring & path)
{
#ifdef WINDOWS
	return splitext_nt(root, ext, path);
#else
	return splitext_posix(root, ext, path);
#endif
}


