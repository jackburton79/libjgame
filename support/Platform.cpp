/*
 * Platform.cpp
 */

#include <jgame/Platform.h>

#include <SDL.h>

namespace jgame {


/* static */
bool
Platform::IsMobile()
{
#if defined(__ANDROID__) || defined(__IPHONEOS__)
	return true;
#else
	return false;
#endif
}


/* static */
bool
Platform::HasTouchScreen()
{
	return SDL_GetNumTouchDevices() > 0;
}


/* static */
std::string
Platform::UserDataDirectory(const char* organization, const char* application)
{
	// SDL_GetPrefPath() makes the folder too
	char* path = SDL_GetPrefPath(organization != nullptr ? organization : "",
		application != nullptr ? application : "");
	if (path == nullptr)
		return std::string();
	std::string result = path;
	SDL_free(path);
	return result;
}

} // namespace jgame
