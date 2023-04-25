#pragma once

#include <vector>
#include <cstdio>

#include <win/gl/GL.hpp>

#include <win/MappedRingBuffer.hpp>

using namespace win::gl;

namespace win
{

struct GLMappedRingBufferReservation
{
	GLMappedRingBufferReservation(int buffer_length, int start, int length)
		: buffer_length(buffer_length)
		, start(start)
		, length(length)
	{}

	bool conflicts(const GLMappedRingBufferReservation &rhs) const
	{
		return
			(start < rhs.start + rhs.length && start + length > rhs.start) ||
			(start + buffer_length < rhs.start + rhs.length && start + buffer_length + length > rhs.start) ||
			(start < rhs.start + buffer_length + rhs.length && start + length > rhs.start + buffer_length);
	}

	const int buffer_length;
	int start;
	int length;
};

struct GLMappedRingBufferLockedRange
{
	GLMappedRingBufferLockedRange(int buffer_length, int start, int length)
		: reservation(buffer_length, start, length)
		, sync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0))
	{
		if (sync == NULL)
			win::bug("glFenceSync() return NULL");
	}

	~GLMappedRingBufferLockedRange()
	{
		glDeleteSync(sync);
	}

	GLMappedRingBufferReservation reservation;
	GLsync sync;
};

template <typename T> class GLMappedRingBuffer;
template <typename T, bool contiguous = false> class GLMappedRingBufferRange : public MappedRingBufferRange<T, contiguous>
{
public:
	GLMappedRingBufferRange(int head, int length, GLMappedRingBuffer<T> &parent)
		: MappedRingBufferRange<T, contiguous>(head, length, parent)
		, parent(parent)
		, locked(false)
	{}

	~GLMappedRingBufferRange()
	{
		if (!locked)
			win::bug("GLMappedRingBufferRange was left unlocked!");
	}

	void lock()
	{
		locked = true;
		parent.lock(*this);
	}

private:
	GLMappedRingBuffer<T> &parent;
	bool locked;
};

template <typename T> class GLMappedRingBuffer
{
	WIN_NO_COPY_MOVE(GLMappedRingBuffer);

public:
	GLMappedRingBuffer(void *mem, int length_elements)
		: inner(mem, length_elements)
	{}

	GLMappedRingBufferRange<T> reserve(int len)
	{
		GLMappedRingBufferReservation reservation(inner.length(), inner.head(), len);
		wait_for_locked_range(reservation);

		return inner.reserve(len);
	}

	GLMappedRingBufferRange<T, true> reserve_contiguous(int len)
	{
		GLMappedRingBufferReservation reservation(inner.length(), inner.head(), len);
		wait_for_locked_range(reservation);

		return inner.reserve_contiguous(len);
	}

	void lock(GLMappedRingBufferRange<T> &range) { lock(range.head(), range.length()); }
	void lock(GLMappedRingBufferRange<T, true> &range) { lock(range.head(), range.length()); }

private:

	void lock(int start, int len)
	{
		locks.emplace_back(inner.length(), start, len);
	}

	void wait_for_locked_range(const GLMappedRingBufferReservation &reservation)
	{
		for (auto it = locks.begin(); it != locks.end();)
		{
			if (reservation.conflicts(it->reservation))
			{
				wait(it->sync);
				it = locks.erase(it);
				continue;
			}

			++it;
		}
	}

	static void wait(GLsync sync)
	{
		unsigned loops = 0;
		for (;;)
		{
			auto result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

			switch(result)
			{
				case GL_WAIT_FAILED:
					win::bug("glClientWaitSync() returned GL_WAIT_FAILED");
				case GL_ALREADY_SIGNALED:
				case GL_CONDITION_SATISFIED:
					return;
				case GL_TIMEOUT_EXPIRED:
					if (++loops > 10)
					{
						fprintf(stderr, "glClientWaitSync() has looped %u times!\n", result);
					}
					break;
				default:
					win::bug("glClientWaitSync(): unrecognized result (" + std::to_string(result) + ")");
			}
		}
	}

	std::vector<GLMappedRingBufferLockedRange> locks;
	MappedRingBuffer<T> inner;
};

}
