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

#include "coe.hpp"

#include "Serialized.hpp"
#include <cstring>

namespace etherkitten::reader
{
	CoEBlockSerializer CoEBlock::serializer;

	/*
	 * block structure:
	 * *----*-------*----------*----------*-----------------*
	 * | id | index | subindex | datatype |      name       |
	 * *----*-------*----------*----------*-----------------*
	 * |  8 |   16  |    8     |    16    | ended by 0-byte |
	 * *----*-------*----------*----------*-----------------*
	 * second line contains the length in bits
	 */

	void CoEBlockSerializer::serialize(const CoEBlock& obj, Serialized& ser)
	{
		ser.write(obj.type, 0);
		ser.write(obj.index, 1);
		ser.write(obj.subindex, 3);
		ser.write(obj.datatype, 4);
		ser.write(obj.name, 6);
	}

	Serialized CoEBlockSerializer::serialize(const CoEBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	datatypes::CoEObject CoEBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		if (ser.read<uint8_t>(0) != 1)
			throw std::runtime_error("type of block is not correct");
		uint16_t index = ser.read<uint16_t>(1);
		uint8_t subindex = ser.read<uint8_t>(3);
		uint16_t datatype = ser.read<uint16_t>(4);
		std::string name = ser.read<std::string>(6);
		return datatypes::CoEObject{ context.slaveId, std::move(name),
			static_cast<datatypes::EtherCATDataTypeEnum>(datatype), index, subindex, 0 };
	}

	uint64_t CoEBlock::getSerializedSize() const { return 6 + name.size() + 1; }
} // namespace etherkitten::reader
