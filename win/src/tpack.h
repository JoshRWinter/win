#ifndef WIN_TPACK_H
#define WIN_TPACK_H

#include <memory>

namespace win
{

class tpack
{
public:
	tpack();
	tpack(const data_list&);
	tpack(const tpack&) = delete;
	tpack(tpack&&);
	~tpack();

	void operator=(const tpack&) = delete;
	tpack &operator=(tpack&&);

	unsigned operator[](int) const;

	static void targa(data, unsigned);

private:
	void move(tpack&);
	void finalize();

	std::unique_ptr<unsigned[]> textures_;
	int count_;
};

}

#endif
