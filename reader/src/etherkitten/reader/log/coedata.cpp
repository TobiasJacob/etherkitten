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

#include "coedata.hpp"

#include "../DatatypesSerializer.hpp"
#include <cstring>
#include <functional>
#include <stdexcept>

namespace etherkitten::reader
{
	CoEDataBlockSerializer CoEDataBlock::serializer;

	/*
	 * block structure:
	 * *-------------------------*-----------*-----------*-------*----------*----------------------*
	 * | id (containing slaveId) | timestamp | blocksize | index | subindex |        data          |
	 * *-------------------------*-----------*-----------*-------*----------*----------------------*
	 * |           32            |     64    |     64    |   16  |    8     | depends on blocksize |
	 * *-------------------------*-----------*-----------*-------*----------*----------------------*
	 * second line contains the length in bits
	 */

	void CoEDataBlockSerializer::serialize(const CoEDataBlock& obj, Serialized& ser)
	{
		uint32_t ident = obj.ident << 16;
		ident |= obj.slaveId;
		ser.write(ident, 0);
		ser.write(obj.timestamp, 4);
		ser.write(obj.getSerializedSize(), 12);
		ser.write(obj.index, 20);
		ser.write(obj.subIndex, 22);
		Serialized sub = ser.getAt(23, DatatypesSerializer::getLength(obj.dataType, obj.data));
		DatatypesSerializer::serialize(obj.dataType, obj.data, sub);
	}

	Serialized CoEDataBlockSerializer::serialize(const CoEDataBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	CoEDataBlock CoEDataBlockSerializer::parseSerialized(Serialized& ser, ParsingContext& context)
	{
		uint32_t ident = ser.read<uint32_t>(0);
		context.slaveId = ident & 0xFFFF;
		uint64_t timestamp = ser.read<uint64_t>(4);
		uint16_t index = ser.read<uint16_t>(20);
		uint8_t subindex = ser.read<uint8_t>(22);
		Serialized sub = ser.getAt(23, ser.length - 23);
		std::any data = DatatypesSerializer::parse(context.getCoEType(index, subindex), sub);
		return CoEDataBlock{ context.slaveId, timestamp, index, subindex, std::move(data),
			context.getCoEType(index, subindex) };
	}

	uint64_t CoEDataBlock::getSerializedSize() const
	{
		return 23 + DatatypesSerializer::getLength(dataType, data);
	}
} // namespace etherkitten::reader
