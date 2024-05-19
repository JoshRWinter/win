#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include <win/Win.hpp>

class DialogManager
{
	WIN_NO_COPY_MOVE(DialogManager);

public:
	enum class MessageType { information, error };

	DialogManager() = default;
	virtual std::optional<std::vector<std::filesystem::path>> import_image() = 0;
	virtual std::optional<std::filesystem::path> import_layout() = 0;
	virtual std::optional<std::filesystem::path> export_layout() = 0;
	virtual void show_message(const std::string &title, const std::string &msg, MessageType type) = 0;
	virtual bool yesno(const std::string &title, const std::string &msg) = 0;

protected:
	~DialogManager() = default;
};
