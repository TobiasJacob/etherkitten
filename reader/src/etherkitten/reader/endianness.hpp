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
 * \brief Defines helper methods to handle endian conversions.
 */

namespace etherkitten::reader
{
	/*!
	 * \brief Reorder the bytes of a given value if the host system is big-endian.
	 * \param input the value to reorder the bytes of
	 * \return the same value with the bytes reordered if necessary
	 */
	template<typename T>
	T flipBytesIfBigEndianHost(T input)
	{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		T output = 0;
		uint8_t* basePointer = reinterpret_cast<uint8_t*>(&input);
		uint8_t* outputPointer = reinterpret_cast<uint8_t*>(&output) + sizeof(T);
		for (size_t i = 0; i < sizeof(T); ++i)
		{
			*(--outputPointer) = *(basePointer++);
		}
#else
		T output = input;
#endif
		return output;
	}
} // namespace etherkitten::reader
