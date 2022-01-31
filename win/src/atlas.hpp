#ifndef WIN_ATLAS_HPP
#define WIN_ATLAS_HPP

#include <memory>

namespace win
{

struct AtlasItem
{
	float x1;
	float y1;
	float x2;
	float y2;
};

class Atlas
{
public:
	enum class Mode {LINEAR, NEAREST};

	Atlas(Stream, Mode = Mode::LINEAR);
	Atlas(const Atlas&) = delete;
	Atlas(Atlas&&) = delete;
	~Atlas();

	void operator=(const Atlas&) = delete;
	Atlas &operator=(Atlas&&) = delete;

	unsigned texture() const;
	const AtlasItem item(int) const;

	static void corrupt();

private:
	int count;
	std::unique_ptr<AtlasItem[]> textures;
	unsigned object;
};

}

#endif
