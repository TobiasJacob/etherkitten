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
 * \brief Defines the ErrorRingBuffer, a ring buffer used for calculating error frequencies.
 */

#include <etherkitten/datatypes/datapoints.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief The ErrorRingBuffer class holds a fixed number of elements of the given type
	 * and allows access to the front and back of the buffer.
	 *
	 * Adding elements initially fills the buffer until it reaches its maximum capacity.
	 * From then on, adding a new element will cause it to replace the oldest element
	 * currently in the buffer.
	 * \tparam T the type of the elements to store in the ErrorRingBuffer
	 */
	template<typename T>
	class ErrorRingBuffer
	{
	public:
		/*!
		 * \brief Construct a new ErrorRingBuffer with the given capacity.
		 * \param capacity the fixed capacity of the ring buffer
		 */
		ErrorRingBuffer(unsigned int capacity)
		    : next(0)
		    , size(0)
		    , capacity(capacity)
		{
			buffer.reserve(capacity);
		}

		/*!
		 * \brief Construct a new ErrorRingBuffer with a capacity of 10.
		 *
		 * This constructor exists purely to make ErrorRingBuffer default-constructible.
		 * Do not use it.
		 */
		ErrorRingBuffer() // NOLINT
		    : ErrorRingBuffer(10) // NOLINT
		{
		}

		/*!
		 * \brief Add the given element to the ring buffer.
		 *
		 * If the ring buffer is not full, it is added to the end.
		 * If the ring buffer is full, element will replace the oldest element
		 * currently in the ring buffer.
		 * \param element the element to add
		 */
		void add(T element)
		{
			if (size < capacity)
			{
				buffer.push_back(element);
				++size;
			}
			else
			{
				buffer[next] = element;
			}
			next = (next + 1) % capacity;
		}

		/*!
		 * \brief Get the element at the given index.
		 * \param index an index into the ring buffer in the range [0, getSize())
		 * \exception std::out_of_range iff index is not in the range [0, getSize())
		 * \return the element at the given index
		 */
		T& at(unsigned int index) { return buffer.at(index); }

		/*!
		 * \brief Get the most recently added element.
		 * \exception std::out_of_range iff the ring buffer is empty
		 * \return the most recently added element
		 */
		T& getNewestElement() { return buffer.at((capacity + next - 1) % capacity); }

		/*!
		 * \brief Get the element that has been in the ring buffer the longest.
		 * \exception std::out_of_range iff the ring buffer is empty
		 * \return the element that has been in the ring buffer the longest
		 */
		T& getOldestElement()
		{
			if (size == capacity)
			{
				return buffer.at(next);
			}
			return buffer.at(0);
		}

		/*!
		 * \brief Get the number of elements currently in the ring buffer.
		 * \return the number of elements in the ring buffer
		 */
		unsigned int getSize() { return size; }

		/*!
		 * \brief Get the capacity of the ring buffer.
		 * \return the capacity of the ring buffer
		 */
		unsigned int getCapacity() { return capacity; }

		/*!
		 * \brief Check whether the ring buffer is empty.
		 * \retval true iff the ring buffer is empty
		 * \retval false iff there is an element in the ring buffer
		 */
		bool isEmpty() { return buffer.size() == 0; }

	private:
		unsigned int next;
		unsigned int size;
		unsigned int capacity;
		std::vector<T> buffer;
	};
} // namespace etherkitten::reader
