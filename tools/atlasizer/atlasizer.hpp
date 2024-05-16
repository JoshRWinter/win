#pragma once

#include <list>
#include <memory>
#include <cmath>
#include <string>

#include <win/FileReadStream.hpp>
#include <win/Targa.hpp>

void gui();
void compileatlas(const std::string&, const std::string&);
std::unique_ptr<unsigned char[]> convert_to_bgra8(const win::Targa &targa);
