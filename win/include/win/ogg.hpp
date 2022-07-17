#ifndef WIN_OGG_HPP
#define WIN_OGG_HPP

#include <condition_variable>

#include <win/stream.hpp>
#include <win/pcmstream.hpp>

void decodeogg(win::Stream, win::PCMStream&, int, std::condition_variable&, std::mutex&);

#endif
