#pragma once

#include <vector>
#include <optional>
#include <filesystem>

#include <win/Win.hpp>

struct AtlasizerItem
{
	AtlasizerItem(int id, int texture, const std::filesystem::path &texturepath, int x, int y, int w, int h)
		: id(id), texture(texture), texturepath(texturepath), x(x), y(y), w(w), h(h) {}

	int id;
	int texture;
	std::filesystem::path texturepath;
	int x;
	int y;
	int w;
	int h;
	bool valid = true;
};

class Atlasizer
{
	WIN_NO_COPY_MOVE(Atlasizer);

	enum class CollisionSide { left, right, bottom, top };

public:
	Atlasizer() = default;

	void add(int texture, const std::filesystem::path &texturepath, int x, int y, int w, int h);
	void remove(int id);
	void start_drag(int x, int y);
	void continue_drag(int x, int y, bool snap);
	void set_padding(int pad);
	int get_padding() const;
	const std::vector<AtlasizerItem> &get_items() const;

private:
	void check_validity();
	bool collide(const AtlasizerItem &a, const AtlasizerItem &b) const;
	CollisionSide collision_side(const AtlasizerItem &a, const AtlasizerItem &b) const;

	static int next_atlasitem_id;

	std::vector<AtlasizerItem> items;

	int padding = 0;
	bool selection_active = false;
	int grabx = 0, graby = 0;

	std::optional<int> left_barrier;
	std::optional<int> right_barrier;
	std::optional<int> bottom_barrier;
	std::optional<int> top_barrier;
};
