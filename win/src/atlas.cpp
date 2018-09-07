#include <string.h>

#include <win.h>

win::atlas::atlas()
{
	count_ = 0;
	object_ = (unsigned)-1;
}

win::atlas::atlas(data raw, mode fm)
{
	int index = 0;

	// check magic
	char magic[6];
	if(raw.read(magic, 5) != 5)
		atlas::corrupt();
	index += 5;
	magic[5] = 0;
	if(strcmp(magic, "ATLAS"))
		atlas::corrupt();

	// how many images
	if(raw.read(&count_, sizeof(count_)) != sizeof(count_))
		atlas::corrupt();
	index += sizeof(count_);

	std::uint16_t canvas_width = 0;
	std::uint16_t canvas_height = 0;

	if(raw.read(&canvas_width, sizeof(canvas_width)) != sizeof(canvas_width))
		atlas::corrupt();
	index += sizeof(canvas_width);
	if(raw.read(&canvas_height, sizeof(canvas_height)) != sizeof(canvas_height))
		atlas::corrupt();
	index += sizeof(canvas_height);

	textures_ = std::make_unique<atlas_texture[]>(count_);

	for(int i = 0; i < count_; ++i)
	{
		if(raw.read(&textures_[i].xpos, sizeof(textures_[i].xpos)) != sizeof(textures_[i].xpos))
			atlas::corrupt();
		index += sizeof(textures_[i].xpos);

		if(raw.read(&textures_[i].ypos, sizeof(textures_[i].ypos)) != sizeof(textures_[i].ypos))
			atlas::corrupt();
		index += sizeof(textures_[i].ypos);

		if(raw.read(&textures_[i].width, sizeof(textures_[i].width)) != sizeof(textures_[i].width))
			atlas::corrupt();
		index += sizeof(textures_[i].width);

		if(raw.read(&textures_[i].height, sizeof(textures_[i].height)) != sizeof(textures_[i].height))
			atlas::corrupt();
		index += sizeof(textures_[i].height);
	}

	if(raw.size() - index != canvas_width * canvas_height * 4)
		atlas::corrupt();

	glGenTextures(1, &object_);
	glBindTexture(GL_TEXTURE_2D, object_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fm == mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fm == mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas_width, canvas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw.get() + index);
}

win::atlas::atlas(atlas &&rhs)
{
	move(rhs);
}

win::atlas::~atlas()
{
	finalize();
}

win::atlas &win::atlas::operator=(atlas &&rhs)
{
	finalize();
	move(rhs);
	return *this;
}

unsigned win::atlas::texture() const
{
	return object_;
}

void win::atlas::corrupt()
{
	throw exception("Corrupt atlas");
}

void win::atlas::move(atlas &rhs)
{
	count_ = rhs.count_;

	object_ = rhs.object_;
	rhs.object_ = (unsigned)-1;

	textures_ = std::move(rhs.textures_);
}

void win::atlas::finalize()
{
	if(object_ == (unsigned)-1)
		return;

	glDeleteTextures(1, &object_);
	object_ = (unsigned)-1;
}
