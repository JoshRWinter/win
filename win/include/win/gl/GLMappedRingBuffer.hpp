#pragma once

#include <vector>

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

	int buffer_length;
	int start;
	int length;
};

struct GLMappedRingBufferLockedRange
{
	WIN_NO_COPY(GLMappedRingBufferLockedRange);

	GLMappedRingBufferLockedRange(int buffer_length, int start, int length)
		: sync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0))
		, reservation(buffer_length, start, length)
	{
		if (sync.get() == NULL)
			win::bug("glFenceSync() return NULL");
	}

	GLMappedRingBufferLockedRange(GLMappedRingBufferLockedRange &&rhs) noexcept = default;

	GLMappedRingBufferLockedRange &operator=(GLMappedRingBufferLockedRange &&rhs) noexcept = default;

	GLSyncObject sync;
	GLMappedRingBufferReservation reservation;
};

template <typename T> class GLMappedRingBuffer;
template <typename T, bool contiguous = false> class GLMappedRingBufferRange : public MappedRingBufferRange<T, contiguous>
{
	WIN_NO_COPY_MOVE(GLMappedRingBufferRange);
	friend class GLMappedRingBuffer<T>;

	// only should be called by GLMappedRingBuffer
	explicit GLMappedRingBufferRange(MappedRingBufferRange<T, contiguous> &&original)
		: MappedRingBufferRange<T, contiguous>(std::move(original))
		, locked(false)
	{}

public:
	~GLMappedRingBufferRange()
	{
		if (!locked)
			win::bug("GLMappedRingBufferRange was left unlocked!");
	}

private:
	bool locked;
};

template <typename T> class GLMappedRingBuffer
{
	WIN_NO_COPY(GLMappedRingBuffer);

public:
	GLMappedRingBuffer()
		: inner(NULL, -1)
	{}

	GLMappedRingBuffer(void *mem, int length_elements)
		: inner(mem, length_elements)
	{}

	GLMappedRingBuffer(GLMappedRingBuffer<T> &&rhs) noexcept = default;

	int head() const { return inner.head(); }
	int length() const { return inner.length(); }

	GLMappedRingBufferRange<T> reserve(int len)
	{
		wait_for_locked_range(inner.head(), len);

		return GLMappedRingBufferRange<T>(inner.reserve(len));
	}

	GLMappedRingBufferRange<T, true> reserve_contiguous(int len)
	{
		wait_for_locked_range(inner.head(), len);

		return GLMappedRingBufferRange<T, true>(inner.reserve_contiguous(len));
	}

	GLMappedRingBuffer<T> &operator=(GLMappedRingBuffer<T> &&rhs) noexcept = default;

	void lock(GLMappedRingBufferRange<T> &range)
	{
		lock(range.head(), range.length());
		range.locked = true;
	}

	void lock(GLMappedRingBufferRange<T, true> &range)
	{
		lock(range.head(), range.length());
		range.locked = true;
	}

private:
	void lock(int start, int len)
	{
		locks.emplace_back(inner.length(), start, len);
	}

	void wait_for_locked_range(const int start, const int length)
	{
		GLMappedRingBufferReservation reservation(inner.length(), start, length);

		for (auto it = locks.begin(); it != locks.end();)
		{
			if (reservation.conflicts(it->reservation))
			{
				wait(it->sync.get());
				it = locks.erase(it);
				continue;
			}

			++it;
		}
	}

	static void wait(GLsync sync)
	{
		GLbitfield flags = 0;
		GLuint64 timeout = 0;
		for (;;)
		{
			auto result = glClientWaitSync(sync, flags, timeout);

			switch(result)
			{
				case GL_WAIT_FAILED:
					win::bug("glClientWaitSync() returned GL_WAIT_FAILED");
				case GL_ALREADY_SIGNALED:
				case GL_CONDITION_SATISFIED:
					return;
				case GL_TIMEOUT_EXPIRED:
					flags = GL_SYNC_FLUSH_COMMANDS_BIT;
					timeout = 500'000'000; // halfsec
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
