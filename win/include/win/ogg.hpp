#ifndef WIN_OGG_HPP
#define WIN_OGG_HPP

#include <condition_variable>

#include <win/stream.hpp>
#include <win/soundstream.hpp>

void decodeogg(win::Stream, win::SoundStream&, std::condition_variable&, std::mutex&);

#endif
