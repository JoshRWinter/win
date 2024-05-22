#pragma once

#include <vector>
#include <unordered_map>
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

	int add(int texture, const std::filesystem::path &texturepath, int x, int y, int w, int h);
	void remove(int id);
	int start_drag(int x, int y);
	void continue_drag(int x, int y, bool snap);
	void set_padding(int pad);
	int get_padding() const;
	const std::vector<AtlasizerItem*> &get_items_layout_order() const;
	const std::vector<AtlasizerItem*> &get_items_display_order() const;

private:
	void check_validity();
	bool collide(const AtlasizerItem &a, const AtlasizerItem &b) const;
	CollisionSide collision_side(const AtlasizerItem &a, const AtlasizerItem &b) const;

	static int next_atlasitem_id;

	std::unordered_map<int, AtlasizerItem> items;
	std::vector<AtlasizerItem*> items_layout_order;
	std::vector<AtlasizerItem*> items_display_order;

	int padding = 0;
	bool selection_active = false;
	int grabx = 0, graby = 0;

	std::optional<int> left_barrier;
	std::optional<int> right_barrier;
	std::optional<int> bottom_barrier;
	std::optional<int> top_barrier;
};
