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

#include "pdo.hpp"

#include <cstring>
#include <stdexcept>

namespace etherkitten::reader
{
	PDOBlockSerializer PDOBlock::serializer;

	/*
	 * block structure:
	 * *----*-------*--------*----------*-----------------*
	 * | id | index | offset | datatype |      name       |
	 * *----*-------*--------*----------*-----------------*
	 * |  8 |   16  |   16   |    16    | ended by 0-byte |
	 * *----*-------*--------*----------*-----------------*
	 * second line contains the length in bits
	 */

	void PDOBlockSerializer::serialize(const PDOBlock& obj, Serialized& ser)
	{
		ser.write(obj.type, 0);
		ser.write(obj.index, 1);
		ser.write(obj.offset, 3);
		ser.write(obj.datatype, 5);
		ser.write(obj.name, 7);
	}

	Serialized PDOBlockSerializer::serialize(const PDOBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	datatypes::PDO PDOBlockSerializer::parseSerialized(Serialized& ser, ParsingContext& context)
	{
		if (ser.read<uint8_t>(0) != 0)
			throw std::runtime_error("trying to parse PDO but block type is wrong");
		uint16_t index = ser.read<uint16_t>(1);
		// this is the offset for busInfo which is not updated here
		ser.read<uint16_t>(3);
		uint16_t datatype = ser.read<uint16_t>(5);
		std::string name = ser.read<std::string>(7);
		return datatypes::PDO{ context.slaveId, std::move(name),
			static_cast<datatypes::EtherCATDataTypeEnum>(datatype), index,
			datatypes::PDODirection::OUTPUT };
	}

	uint64_t PDOBlock::getSerializedSize() const { return 7 + name.size() + 1; }
} // namespace etherkitten::reader
