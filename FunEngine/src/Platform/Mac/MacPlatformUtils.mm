#include "FunEngine/Utils/PlatformUtils.h"

#import <AppKit/AppKit.h>

#include <string>

namespace FunEngine
{
	namespace
	{
		// Win32 filters look like: "Label (*.fe)\0*.fe\0\0"
		NSArray<NSString*>* AllowedTypesFromFilter(const char* filter)
		{
			if (!filter)
				return @[];

			NSMutableArray<NSString*>* types = [NSMutableArray array];
			const char* p = filter;
			while (!(p[0] == '\0' && (p == filter || p[-1] == '\0')))
			{
				if (*p == '\0')
				{
					++p;
					continue;
				}
				std::string token(p);
				p += token.size();
				if (token.size() > 2 && token[0] == '*' && token[1] == '.')
				{
					NSString* ext = [NSString stringWithUTF8String:token.c_str() + 2];
					if (ext.length > 0)
						[types addObject:ext];
				}
			}
			return types;
		}
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		@autoreleasepool
		{
			NSOpenPanel* panel = [NSOpenPanel openPanel];
			panel.canChooseFiles = YES;
			panel.canChooseDirectories = NO;
			panel.allowsMultipleSelection = NO;
			NSArray* types = AllowedTypesFromFilter(filter);
			if (types.count > 0)
				panel.allowedFileTypes = types;

			if ([panel runModal] == NSModalResponseOK && panel.URL)
				return std::string(panel.URL.path.UTF8String);
			return {};
		}
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		@autoreleasepool
		{
			NSSavePanel* panel = [NSSavePanel savePanel];
			NSArray* types = AllowedTypesFromFilter(filter);
			if (types.count > 0)
				panel.allowedFileTypes = types;

			if ([panel runModal] == NSModalResponseOK && panel.URL)
				return std::string(panel.URL.path.UTF8String);
			return {};
		}
	}
}
