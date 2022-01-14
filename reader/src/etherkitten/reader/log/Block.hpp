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
 * \brief Defines the Block class which is used for reading and writing a logfile
 */

#include "Serialized.hpp"
#include "Serializer.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Represents the data of a logical block of bytes in the log file. Can be serialized and
	 * written to the log file.
	 * \tparam T the Block type that the corresponding serializer takes as input
	 * \tparam R the return type of the parsing function of the serializer
	 */
	template<class T, class R = T>
	class Block
	{
	public:
		/*!
		 * \brief Get the Serializer for this Block
		 * \return the Serializer for this block
		 */
		virtual Serializer<T, R>& getSerializer() const = 0;

		/*!
		 * \brief Get the size this block needs in the log file in bytes.
		 * \return the serialized size
		 */
		virtual uint64_t getSerializedSize() const = 0;
	};
} // namespace etherkitten::reader
