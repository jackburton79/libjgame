/*
 * FontLoader.h
 *
 * Where fonts come from. A game (or any code that needs a font) takes a
 * FontLoader and asks it for fonts by name, without knowing whether they are
 * read from files, from a game's resources or built in. To use fonts of its
 * own a game derives from FontLoader (and from Font, see Font.h).
 */
#pragma once

#include <jgame/Font.h>

#include <memory>
#include <string>

namespace jgame {

class FontLoader {
public:
	virtual ~FontLoader() {}

	// The font called `name`, or null if the loader has none by that name or
	// can't read it. What a name means (a file, a resource, a size...) is up to
	// the loader.
	virtual std::unique_ptr<Font> Load(const std::string& name) = 0;
};


// Has the built-in font only, as "builtin"
class BuiltinFontLoader final : public FontLoader {
public:
	virtual std::unique_ptr<Font> Load(const std::string& name) override;
};

} // namespace jgame
