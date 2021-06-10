#ifndef WIN_ATLAS_H
#define WIN_ATLAS_H

#include <memory>

namespace win
{

struct AtlasTexture
{
	unsigned short coords[4];
};

class Atlas
{
public:
	enum class Mode {LINEAR, NEAREST};

	Atlas(AssetRollStream, Mode = Mode::LINEAR);
	Atlas(const Atlas&) = delete;
	Atlas(Atlas&&) = delete;
	~Atlas();

	void operator=(const Atlas&) = delete;
	Atlas &operator=(Atlas&&) = delete;

	unsigned texture() const;
	const unsigned short *coords(int) const;

	static void corrupt();

private:
	std::uint16_t count;
	std::unique_ptr<AtlasTexture[]> textures;
	unsigned object;
};

}

#endif
