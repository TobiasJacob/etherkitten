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

#include "Serialized.hpp"

#include "../endianness.hpp"
#include <inttypes.h>
#include <memory.h>
#include <sstream>

namespace etherkitten::reader
{
	Serialized::Serialized(Serialized& ser)
	{
		data = new char[ser.length];
		length = ser.length;
		memcpy(data, ser.data, length);
	}

	Serialized::Serialized(uint64_t length)
	{
		data = new char[length];
		this->length = length;
	}

	Serialized::Serialized(char* data, uint64_t length)
	{
		this->data = data;
		this->length = length;
		this->shouldFree = false;
	}

	Serialized::~Serialized()
	{
		if (shouldFree)
			delete[] data;
	}

	Serialized Serialized::getAt(uint64_t offset, uint64_t length)
	{
		if (this->length >= offset + length)
			return Serialized(data + offset, length);
		std::stringstream s;
		s << "space is not sufficient, size: " << this->length
		  << " , requested: " << (offset + length);
		throw std::runtime_error(s.str());
	}

} // namespace etherkitten::reader
