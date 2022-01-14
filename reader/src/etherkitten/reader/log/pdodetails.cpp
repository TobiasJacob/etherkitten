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

#include "pdodetails.hpp"

#include <cstring>

namespace etherkitten::reader
{
	PDODetailsBlockSerializer PDODetailsBlock::serializer;

	/*
	 * block structure:
	 * *-------*--------*-----------*----------*
	 * | index | offset | bitlength | datatype |
	 * *-------*--------*-----------*----------*
	 * |   16  |   16   |     8     |    16    |
	 * *-------*--------*-----------*----------*
	 * second line contains the length in bits
	 */

	void PDODetailsBlockSerializer::serialize(const PDODetailsBlock& obj, Serialized& ser)
	{
		ser.write(obj.index, 0);
		ser.write(obj.offset, 2);
		ser.write(obj.bitLength, 4);
		ser.write(obj.datatype, 5);
	}

	Serialized PDODetailsBlockSerializer::serialize(const PDODetailsBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	PDODetailsBlock PDODetailsBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		(void)context;
		uint16_t index = ser.read<uint16_t>(0);
		uint16_t offset = ser.read<uint16_t>(2);
		uint8_t bitLength = ser.read<uint8_t>(4);
		uint16_t datatype = ser.read<uint16_t>(5);

		/* PDO offsets are not set in context object.
		 * LogSlaveInformant is responsible for handling them.*/

		return PDODetailsBlock{ index, offset, bitLength, datatype };
	}

	uint64_t PDODetailsBlock::getSerializedSize() const { return 7; }
} // namespace etherkitten::reader
