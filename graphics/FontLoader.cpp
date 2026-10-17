#include <jgame/FontLoader.h>

namespace jgame {

/* virtual */
std::unique_ptr<Font>
BuiltinFontLoader::Load(const std::string& name)
{
	if (name != "builtin")
		return nullptr;
	return std::make_unique<BuiltinFont>();
}

} // namespace jgame
