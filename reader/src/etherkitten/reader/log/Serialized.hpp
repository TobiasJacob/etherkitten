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
 * \brief Defines Serialized, a buffer that has methods to write data to it
 * and parse containing data.
 */

#include "../endianness.hpp"
#include <cstring>
#include <inttypes.h>
#include <stdexcept>

namespace etherkitten::reader
{
	/*!
	 * \brief Is a buffer of fixed length to write to a file.
	 */
	struct Serialized
	{
		/*!
		 * \brief Create a Serialized object of given length.
		 * \param length the length in bytes
		 */
		Serialized(uint64_t length);

		Serialized(Serialized&& ser) = default;

		/*!
		 * \brief Create a copy of a Serialized object.
		 * This should not be used because the buffer is copied.
		 * \param ser the object to copy
		 */
		Serialized(Serialized& ser);

		~Serialized();

		/*!
		 * \brief Create a Serialized object representing a part of the buffer of this object.
		 * The created Serialized object does not have ownership of the buffer!!!
		 * \param offset the offset at which the buffer of the object should begin.
		 * 0 means same beginning.
		 * \param length the length of the buffer of the object
		 * \return
		 * \exception std::runtime_error if the requested region is not entirely included in this
		 * buffer.
		 */
		Serialized getAt(uint64_t offset, uint64_t length);

		/*!
		 * \brief data the buffer of length length
		 */
		char* data;

		/*!
		 * \brief length the length of the buffer
		 */
		uint64_t length;

		/*!
		 * \brief Write data to the buffer at given offset.
		 * This method changes the byte order if neccessary.
		 * \param data the data to write
		 * \param offset the offset
		 * \tparam T the type of the data to write into the buffer
		 * \exception std::runtime_error when trying to write out of bounds
		 */
		template<typename T>
		void write(const T& data, const uint64_t offset)
		{
			if (sizeof(data) + offset > length)
				throw std::runtime_error(
				    "trying to write data to Serialized that is longer than the buffer");
			T flipped = flipBytesIfBigEndianHost(data);
			memcpy(this->data + offset, &flipped, sizeof(data));
		}

		/*!
		 * \brief Write data to the buffer at given offset.
		 * This method changes the byte order if neccessary.
		 * \param data the data to write
		 * \param offset the offset
		 * \tparam T the type of the data to write into the buffer
		 * \exception std::runtime_error when trying to write out of bounds
		 */
		template<typename T>
		void write(const T&& data, const uint64_t offset)
		{
			if (sizeof(data) + offset > length)
				throw std::runtime_error(
				    "trying to write data to Serialized that is longer than the buffer");
			T flipped = flipBytesIfBigEndianHost(data);
			memcpy(this->data + offset, &flipped, sizeof(data));
		}

		/*!
		 * \brief Read data from the buffer at given offset.
		 * This method changes the byte order if neccessary.
		 * \param data the data to write
		 * \param offset the offset
		 * \tparam T the type in which the data should be interpreted
		 * \exception std::runtime_error when trying to read out of bounds
		 */
		template<typename T>
		T read(const uint64_t offset) const
		{
			if (sizeof(T) + offset > length)
				throw std::runtime_error(
				    "trying to read data from serialized that exceeds the length of the buffer");
			T data;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
			memcpy(&data, this->data + offset, sizeof(T));
#pragma GCC diagnostic pop
			return flipBytesIfBigEndianHost(data);
		}

	private:
		/*!
		 * \brief Create Serialized object with an already initialized buffer
		 * \param data the buffer
		 * \param length the length of the buffer
		 */
		Serialized(char* data, uint64_t length);

		/*!
		 * \brief shouldFree Indicates whether the buffer should be freed if the object is
		 * destroyed.
		 */
		bool shouldFree = true;
	};

	template<>
	inline void Serialized::write<std::string>(const std::string& data, const uint64_t offset)
	{
		uint64_t strlen = data.size();
		if (strlen + offset + 1 > length)
			throw std::runtime_error(
			    "trying to write string to Serialized that exceeds buffer capacity");
		memcpy(this->data + offset, data.c_str(), strlen + 1);
	}

	template<>
	inline void Serialized::write<std::string>(const std::string&& data, const uint64_t offset)
	{
		uint64_t strlen = data.size();
		if (strlen + offset + 1 > length)
			throw std::runtime_error(
			    "trying to write string to Serialized that exceeds buffer capacity");
		strncpy(this->data + offset, data.c_str(), strlen + 1);
	}

	template<>
	inline std::string Serialized::read<std::string>(const uint64_t offset) const
	{
		if (offset >= length || strnlen(data + offset, length - offset) + offset == length)
			throw std::runtime_error("string length exceeds buffer length");
		std::string str{ data + offset };
		return str;
	}

} // namespace etherkitten::reader
