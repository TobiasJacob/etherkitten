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

#include "processdata.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

namespace etherkitten::reader
{
	ProcessDataBlockSerializer ProcessDataBlock::serializer;

	/*
	 * block structure:
	 * *----*-----------*-----------------------*
	 * | id | timestamp |         data          |
	 * *----*-----------*-----------------------*
	 * | 32 |     64    | depends on iomap size |
	 * *----*-----------*-----------------------*
	 * second line contains the length in bits
	 */

	void ProcessDataBlockSerializer::serialize(const ProcessDataBlock& obj, Serialized& ser)
	{
		ser.write(obj.ident, 0);
		ser.write(obj.timestamp, 4);
		memcpy(ser.data + 12, obj.data.data, obj.data.length);
	}

	Serialized ProcessDataBlockSerializer::serialize(const ProcessDataBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	ProcessDataBlock ProcessDataBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		uint64_t timestamp = ser.read<uint64_t>(4);
		Serialized data(context.getIOMapSize());
		memcpy(data.data, ser.data + 12, context.getIOMapSize());
		return ProcessDataBlock{ timestamp, std::move(data) };
	}

	uint64_t ProcessDataBlock::getSerializedSize() const { return 12 + data.length; }
} // namespace etherkitten::reader
