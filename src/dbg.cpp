//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>	// isprint
#include <alloca.h>	// alloca

#if RTM_PLATFORM_ANDROID
#include <android/log.h>
#endif

#if RTM_PLATFORM_IOS || RTM_PLATFORM_OSX
#if defined(__OBJC__)
#import <Foundation/NSObjCRuntime.h>
#else
#include <CoreFoundation/CFString.h> 
extern "C" void NSLog(CFStringRef _format, ...); 
#endif
#endif

namespace rapp {

int32_t snprintf(char* _out, int32_t _max, const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	int32_t total = vsnprintf(_out, _max, _format, argList);
	va_end(argList);
	return total;
}

int32_t vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList)
{
	va_list argList;
	va_copy(argList, _argList);
	int32_t total = 0;
#if RTM_COMPILER_MSVC
	int32_t len = -1;
	if (NULL != _out)
	{
		va_list argListCopy;
		va_copy(argListCopy, _argList);
		len = ::vsnprintf_s(_out, _max, size_t(-1), _format, argListCopy);
		va_end(argListCopy);
	}
	total = -1 == len ? ::_vscprintf(_format, argList) : len;
#else  // RTM_COMPILER_MSVC
	total = ::vsnprintf(_out, _max, _format, argList);
#endif // RTM_COMPILER_MSVC
	va_end(argList);
	return total;
}

void debugOutput(const char* _out)
{
#if RTM_PLATFORM_ANDROID
	__android_log_write(ANDROID_LOG_DEBUG, "", _out);
#elif  RTM_PLATFORM_WINDOWS \
	|| RTM_PLATFORM_WINRT \
	|| RTM_PLATFORM_XBOXONE
	OutputDebugStringA(_out);
#elif RTM_PLATFORM_IOS || RTM_PLATFORM_OSX
#	if defined(__OBJC__)
	NSLog(@"%s", _out);
#	else
	NSLog(__CFStringMakeConstantString("%s"), _out);
#	endif // defined(__OBJC__)
#elif RTM_PLATFORM_PS4
	::printf("%s",_out);
#else
	RTM_UNUSED(_out);
#endif // RTM_PLATFORM_
}

void dbgPrintfVargs(const char* _format, va_list _argList)
{
	char temp[8192];
	char* out = temp;
	int32_t len = vsnprintf(out, sizeof(temp), _format, _argList);
	if ( (int32_t)sizeof(temp) < len)
	{
		out = (char*)alloca(len+1);
		len = vsnprintf(out, len, _format, _argList);
	}
	out[len] = '\0';
	debugOutput(out);
}

void dbgPrintf(const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	dbgPrintfVargs(_format, argList);
	va_end(argList);
}

#define DBG_ADDRESS "%" PRIxPTR

void dbgPrintfData(const void* _data, uint32_t _size, const char* _format, ...)
{
#define HEX_DUMP_WIDTH 16
#define HEX_DUMP_SPACE_WIDTH 48
#define HEX_DUMP_FORMAT "%-" RAPP_DBG_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "." RAPP_DBG_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "s"

	va_list argList;
	va_start(argList, _format);
	dbgPrintfVargs(_format, argList);
	va_end(argList);

	dbgPrintf("\ndata: " DBG_ADDRESS ", size: %d\n", _data, _size);

	if (NULL != _data)
	{
		const uint8_t* data = reinterpret_cast<const uint8_t*>(_data);
		char hex[HEX_DUMP_WIDTH*3+1];
		char ascii[HEX_DUMP_WIDTH+1];
		uint32_t hexPos = 0;
		uint32_t asciiPos = 0;
		for (uint32_t ii = 0; ii < _size; ++ii)
		{
			snprintf(&hex[hexPos], sizeof(hex)-hexPos, "%02x ", data[asciiPos]);
			hexPos += 3;

			ascii[asciiPos] = isprint(data[asciiPos]) ? data[asciiPos] : '.';
			asciiPos++;

			if (HEX_DUMP_WIDTH == asciiPos)
			{
				ascii[asciiPos] = '\0';
				dbgPrintf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
				data += asciiPos;
				hexPos = 0;
				asciiPos = 0;
			}
		}

		if (0 != asciiPos)
		{
			ascii[asciiPos] = '\0';
			dbgPrintf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
		}
	}

#undef HEX_DUMP_WIDTH
#undef HEX_DUMP_SPACE_WIDTH
#undef HEX_DUMP_FORMAT
}

} // namespace rapp
