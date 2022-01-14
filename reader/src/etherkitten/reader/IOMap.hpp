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
 * \brief Defines the IOMap struct used to hold an instance of SOEM's IOMap.
 */

#include <cstdint>

namespace etherkitten::reader
{
	/*!
	 * \brief The IOMap struct represents an IOMap as used for PDOs by SOEM
	 * as a buffer.
	 *
	 * This is a variable-length struct.
	 * It must be initialized via the new operator with ioMapSize variable set to
	 * the size of the flexible-length array. Afterwards, it must only be used
	 * through pointers and never be passed by value - that would truncate the buffer.
	 */
	struct IOMap
	{
		/*!
		 * \brief The size of the ioMap array in bytes.
		 */
		std::size_t ioMapSize; // NOLINT(misc-non-private-member-variables-in-classes)
		/*!
		 * \brief The data in this ioMap.
		 *
		 * The length of 1 is due to C++ language limitations. In actuality, the array has the
		 * length of ioMapSize.
		 */
		uint8_t ioMap[1]; // NOLINT

		/*!
		 * \brief Allocate space for a new IOMap with the given ioMapSize.
		 *
		 * Does not set the ioMapSize member variable to the given parameter.
		 * \param count the byte size of the IOMap struct (given an ioMap length of 1)
		 * \param ioMapSize the size of the ioMap data area
		 */
		void* operator new(std::size_t count, std::size_t ioMapSize)
		{
			IOMap* memory = reinterpret_cast<IOMap*>(new uint8_t[count + ioMapSize - 1]); // NOLINT
			return memory;
		}

		void operator delete(void* ptr) { delete[] reinterpret_cast<uint8_t*>(ptr); } // NOLINT
	};

} // namespace etherkitten::reader
