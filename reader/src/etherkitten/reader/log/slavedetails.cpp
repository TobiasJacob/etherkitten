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

#include "slavedetails.hpp"

#include <cstring>

namespace etherkitten::reader
{
	SlaveDetailsBlockSerializer SlaveDetailsBlock::serializer;

	/*
	 * block structure:
	 * *----------*-----------*--------------------*
	 * | slave id | blocksize | pdo details blocks |
	 * *----------*-----------*--------------------*
	 * |   16     |     16    |                    |
	 * *----------*-----------*--------------------*
	 * second line contains the length in bits
	 */

	void SlaveDetailsBlockSerializer::serialize(const SlaveDetailsBlock& obj, Serialized& ser)
	{
		ser.write(obj.slaveId, 0);
		ser.write<uint16_t>(obj.getSerializedSize(), 2);
		uint64_t offset = 4;
		for (auto& block : obj.pdoBlocks)
		{
			Serialized sub = ser.getAt(offset, block.getSerializedSize());
			block.getSerializer().serialize(block, sub);
			offset += sub.length;
		}
	}

	Serialized SlaveDetailsBlockSerializer::serialize(const SlaveDetailsBlock& obj)
	{
		uint64_t size(obj.getSerializedSize());
		Serialized ser(size);
		serialize(obj, ser);
		return ser;
	}

	SlaveDetailsBlock SlaveDetailsBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		uint16_t slaveid = ser.read<uint16_t>(0);
		uint16_t size = ser.read<uint16_t>(2);
		std::vector<PDODetailsBlock> pdoDetails;
		uint64_t offset = 4;
		while (offset + 1 < size)
		{
			Serialized sub = ser.getAt(offset, ser.length - offset);
			pdoDetails.push_back(PDODetailsBlock::serializer.parseSerialized(sub, context));
			offset += pdoDetails.back().getSerializedSize();
		}
		return SlaveDetailsBlock{ slaveid, pdoDetails };
	}

	uint64_t SlaveDetailsBlock::getSerializedSize() const
	{
		uint64_t size = 4;
		for (auto& block : pdoBlocks)
		{
			size += block.getSerializedSize();
		}
		return size;
	}
} // namespace etherkitten::reader
