#ifndef WIN_AUDIO_ENGINE_H
#define WIN_AUDIO_ENGINE_H

#include <pulse/pulseaudio.h>

namespace win
{

struct sound
{
	sound() = default;
	sound(const sound &rhs)
		: parent(rhs.parent), id(rhs.id),
		start(rhs.start), pcm(rhs.pcm),
		size(rhs.size),
		target_size(rhs.target_size),
		done(rhs.done.load()),
		cancel(rhs.cancel.load()),
		stream(rhs.stream)
	{}
	sound &operator=(const sound &rhs)
	{
		parent = rhs.parent;
		id = rhs.id;
		start = rhs.start;
		pcm = rhs.pcm;
		size = rhs.size;
		target_size = rhs.target_size;
		done = rhs.done.load();
		cancel = rhs.cancel.load();
		stream = rhs.stream;

		return *this;
	}

	audio_engine *parent;
	int id; // unique sound instance id
	long long start; // start here next write
	short *pcm; // audio data
	std::atomic<unsigned long long> *size; // how much has been decoded
	unsigned long long target_size; // how big entire pcm buffer is
	std::atomic<bool> done;
	std::atomic<bool> cancel;
	pa_stream *stream;
};

struct sound_list
{
	struct node { sound snd; node *next; };

	sound_list() { head = NULL; }
	sound_list(const sound_list&) = delete;
	sound_list(sound_list &&rhs)
	{
		head = rhs.head;
		rhs.head = NULL;
	}
	~sound_list()
	{
		node *current = head;
		while(current != NULL)
		{
			node *next = current->next;
			delete current;
			current = next;
		}
		head = NULL;
	}
	void operator=(const sound_list&) = delete;
	sound_list &operator=(sound_list &&rhs)
	{
		head = rhs.head;
		rhs.head = NULL;
		return *this;
	}
	sound *add(const sound &s)
	{
		node *n = new node;
		n->snd = s;

		node *tmp = head;
		head = n;
		n->next = tmp;

		return &n->snd;
	}
	node *remove(node *n)
	{
		// find it
		node *current = head;
		node *prev = NULL;
		while(current != NULL)
		{
			if(current == n)
			{
				if(prev == NULL)
					head = current->next;
				else
					prev->next = current->next;

				node *tmp = current->next;
				delete current;
				return tmp;
			}

			prev = current;
			current = current->next;
		}

		std::cerr << "could not find the node";
		std::abort();
	}

	node *head;
};

class audio_engine
{
	friend display;

public:
	static constexpr int MAX_SOUNDS = 32;

	audio_engine(const audio_engine&) = delete;
	audio_engine(audio_engine&&);
	~audio_engine();

	void operator=(const audio_engine&) = delete;
	audio_engine &operator=(audio_engine&&);

	void play(const apack&, int);

	void pause_all();
	void resume_all();

private:
	audio_engine();
	void move(audio_engine&);
	void finalize();

	int next_id_;
	int active_sounds_;

#if defined WINPLAT_LINUX
	void cleanup(bool);

	pa_context *context_;
	pa_threaded_mainloop *loop_;
	sound_list sounds_;
#endif
};

}

#endif
