#pragma once

#include <vector>
#include <cstdio>

#include <win/gl/GL.hpp>

#include <win/MappedRingBuffer.hpp>

using namespace win::gl;

namespace win
{

class GLSyncObject
{
	WIN_NO_COPY(GLSyncObject);

public:
	explicit GLSyncObject(GLsync s) : sync(s) {}
	GLSyncObject(GLSyncObject &&rhs) noexcept : sync(rhs.sync) { rhs.sync = NULL; }
	~GLSyncObject() { if (sync != NULL) glDeleteSync(sync); }

	GLSyncObject &operator=(GLSyncObject &&rhs) noexcept
	{
		if (&rhs == this) return *this;

		sync = rhs.sync;
		rhs.sync = NULL;

		return *this;
	}

	GLsync get() const { return sync; }

private:
	GLsync sync;
};

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
		: reservation(buffer_length, start, length)
		, sync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0))
	{
		if (sync.get() == NULL)
			win::bug("glFenceSync() return NULL");
	}

	GLMappedRingBufferLockedRange(GLMappedRingBufferLockedRange &&rhs) noexcept = default;

	GLMappedRingBufferLockedRange &operator=(GLMappedRingBufferLockedRange &&rhs) noexcept = default;

	GLMappedRingBufferReservation reservation;
	GLSyncObject sync;
};

template <typename T> class GLMappedRingBuffer;
template <typename T, bool contiguous = false> class GLMappedRingBufferRange : public MappedRingBufferRange<T, contiguous>
{
	WIN_NO_COPY(GLMappedRingBufferRange);
	friend class GLMappedRingBuffer<T>;

	// only should be called by GLMappedRingBuffer
	explicit GLMappedRingBufferRange(MappedRingBufferRange<T, contiguous> &&original)
		: MappedRingBufferRange<T, contiguous>(std::move(original))
		, locked(false)
	{}

public:
	GLMappedRingBufferRange()
		: locked(true) // pretend to be locked so the destructor doesn't get angry
	{}

	GLMappedRingBufferRange(GLMappedRingBufferRange<T, contiguous> &&rhs) noexcept
		: MappedRingBufferRange<T, contiguous>(std::move(rhs))
		, locked(rhs.locked)
	{
		// pretend to lock the rhs so its destructor doesn't complain
		rhs.locked = true;
	}

	~GLMappedRingBufferRange()
	{
		if (!locked)
			win::bug("GLMappedRingBufferRange was left unlocked!");
	}

	GLMappedRingBufferRange &operator=(GLMappedRingBufferRange<T, contiguous> &&rhs) noexcept
	{
		MappedRingBufferRange<T, contiguous>::operator=(std::move(rhs));
		locked = rhs.locked;

		// pretend to lock the rhs so its destructor doesn't complain
		rhs.locked = true;

		return *this;
	}

	void discard()
	{
		if (locked)
			win::bug("GLMappedRingBufferRange: discard() was called on a locked range! This is nonsensical.");

		// fool the destructor into thinking everything's fine
		locked = true;
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
						fprintf(stderr, "glClientWaitSync() has looped %u times!\n", loops);
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
