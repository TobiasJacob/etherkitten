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

#include "coeentry.hpp"

#include "Serialized.hpp"
#include <cstring>
#include <stdexcept>

namespace etherkitten::reader
{
	CoEEntryBlockSerializer CoEEntryBlock::serializer;

	/*
	 * block structure:
	 * *----*-----------*-------*-------------*----------------------*
	 * | id | blocksize | index | object code |        name          |
	 * *----*-----------*-------*-------------*----------------------*
	 * |  8 |     16    |   16  |   8         | depends on blocksize |
	 * *----*-----------*-------*-------------*----------------------*
	 * second line contains the length in bits
	 */

	void CoEEntryBlockSerializer::serialize(const CoEEntryBlock& obj, Serialized& ser)
	{
		ser.write(obj.type, 0);
		ser.write(obj.getSerializedSize(), 1);
		ser.write(obj.index, 3);
		ser.write(obj.objectCode, 5);
		ser.write(obj.name, 6);
		uint64_t offset = 6 + obj.name.size() + 1;
		for (auto& block : obj.coes)
		{
			Serialized sub = ser.getAt(offset, block.getSerializedSize());
			block.getSerializer().serialize(block, sub);
			offset += sub.length;
		}
	}

	Serialized CoEEntryBlockSerializer::serialize(const CoEEntryBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	datatypes::CoEEntry CoEEntryBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		if (ser.read<uint8_t>(0) != 5)
			throw std::runtime_error("block type is not correct, expected 5");
		if (ser.read<uint16_t>(1) != ser.length)
			throw std::runtime_error("Serialized size does not equal block size");
		uint16_t index = ser.read<uint16_t>(3);
		datatypes::CoEObjectCode objectCode
		    = static_cast<datatypes::CoEObjectCode>(ser.read<uint8_t>(5));
		std::string name = ser.read<std::string>(6);
		std::vector<datatypes::CoEObject> coes;
		uint64_t offset = 6 + name.size() + 1;
		while (offset + 2 < ser.length)
		{
			Serialized sub(ser.getAt(offset, 7 + strlen(ser.data + offset + 6)));
			coes.push_back(CoEBlock::serializer.parseSerialized(sub, context));
			offset += sub.length;
		}

		return datatypes::CoEEntry{ context.slaveId, index, objectCode, std::move(name),
			std::move(coes) };
	}

	uint64_t CoEEntryBlock::getSerializedSize() const
	{
		uint64_t size = 6 + name.size() + 1;
		for (auto& block : coes)
			size += block.getSerializedSize();
		return size;
	}
} // namespace etherkitten::reader
