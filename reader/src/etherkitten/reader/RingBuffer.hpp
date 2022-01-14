/*
 * Copyright 2021 Niklas Arlt, Matthias Becht, Florian Bossert, Marwin Madsen, and Philip Scherer
 *
 * This file is part of EtherKITten.
 *
 * EtherKITten is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * EtherKITten is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EtherKITten.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
/*!
 * \file
 * \brief Defines the RingBuffer, a class used to calculate frequencies in a
 * thread-safe manner.
 */

#include <atomic>

#include <etherkitten/datatypes/datapoints.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief The AtomicWrapper class wraps an atomic in a copy-constructible type.
	 *
	 * Note that the copy-construction is not thread-safe.
	 * \tparam Type the type to store atomically
	 */
	template<typename Type>
	class AtomicWrapper // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		std::atomic<Type> a; // NOLINT(misc-non-private-member-variables-in-classes)

		AtomicWrapper()
		    : a()
		{
		}

		AtomicWrapper(const Type& value)
		    : a(value)
		{
		}

		AtomicWrapper(const std::atomic<Type>& a)
		    : a(a.load())
		{
		}

		AtomicWrapper(const AtomicWrapper& other)
		    : a(other.a.load())
		{
		}

		AtomicWrapper& operator=(const AtomicWrapper& other) { a.store(other._a.load()); }
	};

	/*!
	 * \brief The RingBuffer class holds elements in a FIFO fashion.
	 *
	 * This class contains a race condition when reading and writing at the same time from
	 * different threads. When adding an element, the `getOldest` and `getNewest` methods
	 * temporarily return the newest and second newest elements instead.
	 *
	 * In the situations in which we use this ring buffer, we can easily detect
	 * this race condition and simply retry the operations until we get the correct values.
	 * This is preferable to introducing a readers-writers lock at the time of writing.
	 * DO NOT use this class if you depend on this race condition not occurring.
	 * \tparam Type the type of elements to hold
	 * \tparam Capacity how many elements this RingBuffer can hold before overwriting old ones
	 */
	template<typename Type, size_t Capacity>
	class RingBuffer
	{
	public:
		RingBuffer()
		    : next(0)
		    , size(0)
		{
			buffer.reserve(Capacity);
		}

		/*!
		 * \brief Add the given element to the next place in the RingBuffer.
		 * \param element the element to be added
		 */
		void add(Type element)
		{
			// THIS METHOD IS ONLY THREAD-SAFE FOR ONE WRITER
			unsigned int localNext = next.load(std::memory_order_acquire);
			if (localNext >= buffer.size())
			{
				buffer.emplace_back(element);
			}
			else
			{
				buffer[localNext].a.store(element, std::memory_order_release);
			}

			next.store((localNext + 1) % Capacity, std::memory_order_release);
			if (size.load(std::memory_order_acquire) < Capacity)
			{
				size.fetch_add(1, std::memory_order_release);
			}
		}

		/*!
		 * \brief Get the element at the given index.
		 * \param index the index that identifies the element
		 * \return the element at the given index
		 */
		Type at(unsigned int index) { return buffer.at(index).a.load(std::memory_order_acquire); }

		/*!
		 * \brief Get the most recently added element.
		 * \return the most recently added element
		 */
		Type getNewestElement()
		{
			// Adding capacity fixes the -1 % Capacity == -1 problem
			return buffer.at((next.load(std::memory_order_acquire) + Capacity - 1) % Capacity)
			    .a.load(std::memory_order_acquire);
		}

		/*!
		 * \brief Get the oldest element in the RingBuffer.
		 * \return the oldest element in the RingBuffer
		 */
		Type getOldestElement()
		{
			if (size.load(std::memory_order_acquire) == Capacity)
			{
				return buffer.at(next.load(std::memory_order_acquire))
				    .a.load(std::memory_order_acquire);
			}
			return buffer.at(0).a.load(std::memory_order_acquire);
		}

		/*!
		 * \brief Get the size of the RingBuffer.
		 * \return the size of the RingBuffer
		 */
		unsigned int getSize() { return size.load(std::memory_order_acquire); }

		/*!
		 * \brief Get the capacity of the RingBuffer.
		 * \return the capacity of the RingBuffer
		 */
		unsigned int getCapacity() { return Capacity; }

		/*!
		 * \brief verifies whether the ring buffer is empty
		 * \return true if the ring buffer ist empty false otherwise
		 */
		bool isEmpty() { return buffer.size() == 0; }

	private:
		std::atomic_uint next;
		std::atomic_uint size;
		std::vector<AtomicWrapper<Type>> buffer;
	};
} // namespace etherkitten::reader
