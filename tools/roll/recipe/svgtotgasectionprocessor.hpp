#ifndef SVGTOTGASECTIONPROCESSOR_HPP
#define SVGTOTGASECTIONPROCESSOR_HPP

#include "rollfilesectionprocessor.hpp"

class SvgToTgaSectionProcessor : public RollFileSectionProcessor
{
public:
	SvgToTgaSectionProcessor(const std::vector<RollInputLine> inputs, bool);
    ~SvgToTgaSectionProcessor() override;

	bool update() const override;
	std::vector<RollItem> get_items() const override;

private:
	std::vector<RollItem> items;
	bool needs_update;
	bool force_update;
};

#endif
