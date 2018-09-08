#ifndef WIN_ATLAS_H
#define WIN_ATLAS_H

#include <memory>

namespace win
{

struct atlas_texture
{
	unsigned short coords[4];
};

class atlas
{
public:
	enum class mode {LINEAR, NEAREST};

	atlas();
	atlas(data, mode = mode::LINEAR);
	atlas(const atlas&) = delete;
	atlas(atlas&&);
	~atlas();

	void operator=(const atlas&) = delete;
	atlas &operator=(atlas&&);

	unsigned texture() const;
	const unsigned short *coords(int) const;

	static void corrupt();

private:
	void move(atlas&);
	void finalize();

	std::uint16_t count_;
	std::unique_ptr<atlas_texture[]> textures_;
	unsigned object_;
};

}

#endif
