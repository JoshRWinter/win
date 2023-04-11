#include <win/gl/GLMappedRingBuffer.hpp>

int successfull = 0;
#define assert(exp) do { if (!(exp)) { win::bug("assert failed on line " + std::to_string(__LINE__)); } else { ++successfull; } } while (false)

int main()
{
	// non-collision cases
	{
		/*       \\//         */
		win::GLMappedRingBufferReservation left(5, 0, 1);
		win::GLMappedRingBufferReservation right(5, 2, 1);

		assert(!left.conflicts(right));
		assert(!right.conflicts(left));
	}

	{
		/*       _///\        */
		win::GLMappedRingBufferReservation left(5, 4, 1);
		win::GLMappedRingBufferReservation right(5, 1, 3);

		assert(!left.conflicts(right));
		assert(!right.conflicts(left));
	}

	{
		/*       \\_// */
		win::GLMappedRingBufferReservation left(5, 0, 2);
		win::GLMappedRingBufferReservation right(5, 3, 2);

		assert(!left.conflicts(right));
		assert(!right.conflicts(left));
	}

	{
		/*       \\\//       */
		win::GLMappedRingBufferReservation left(5, 0, 3);
		win::GLMappedRingBufferReservation right(5, 3, 2);

		assert(!left.conflicts(right));
		assert(!right.conflicts(left));
	}

	// non collision wrap cases

	{
		/*       /\\//       */
		win::GLMappedRingBufferReservation left(5, 1, 2);
		win::GLMappedRingBufferReservation right(5, 3, 3);

		assert(!left.conflicts(right));
		assert(!right.conflicts(left));
	}

	{
		/*       \_//\       */
		win::GLMappedRingBufferReservation left(5, 4, 2);
		win::GLMappedRingBufferReservation right(5, 2, 2);

		assert(!left.conflicts(right));
		assert(!right.conflicts(left));
	}

	// collision cases
	{
		/*      _/X//        */
		win::GLMappedRingBufferReservation left(5, 2, 1);
		win::GLMappedRingBufferReservation right(5, 1, 4);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      _XXX        */
		win::GLMappedRingBufferReservation left(5, 1, 3);
		win::GLMappedRingBufferReservation right(5, 1, 3);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      __/X\        */
		win::GLMappedRingBufferReservation left(5, 3, 2);
		win::GLMappedRingBufferReservation right(5, 2, 2);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      _\X\\        */
		win::GLMappedRingBufferReservation left(5, 1, 4);
		win::GLMappedRingBufferReservation right(5, 2, 1);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      _\\X_        */
		win::GLMappedRingBufferReservation left(5, 1, 3);
		win::GLMappedRingBufferReservation right(5, 3, 1);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      /X\\_        */
		win::GLMappedRingBufferReservation left(5, 1, 3);
		win::GLMappedRingBufferReservation right(5, 0, 2);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	// collision wrap cases
	{
		/*      X//\\        */
		win::GLMappedRingBufferReservation left(5, 3, 3);
		win::GLMappedRingBufferReservation right(5, 0, 3);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      XX\//      */
		win::GLMappedRingBufferReservation left(5, 0, 3);
		win::GLMappedRingBufferReservation right(5, 3, 4);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	{
		/*      XXX_X      */
		win::GLMappedRingBufferReservation left(5, 4, 3);
		win::GLMappedRingBufferReservation right(5, 4, 3);

		assert(left.conflicts(right));
		assert(right.conflicts(left));
	}

	fprintf(stderr, "%d tests ran successfully\n", successfull);
}
