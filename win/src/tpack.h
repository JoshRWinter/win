#ifndef WIN_TPACK_H
#define WIN_TPACK_H

#include <memory>

namespace win
{

class tpack
{
public:
	enum class mode { LINEAR, NEAREST };

	tpack();
	tpack(const data_list&, mode = mode::LINEAR);
	tpack(const tpack&) = delete;
	tpack(tpack&&);
	~tpack();

	void operator=(const tpack&) = delete;
	tpack &operator=(tpack&&);
	unsigned operator[](int) const;

	void filter(int, mode);

	static void targa(data, unsigned, mode);

private:
	void move(tpack&);
	void finalize();

	std::unique_ptr<unsigned[]> textures_;
	int count_;
};

}

#endif
