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

#include "slave.hpp"

#include "../DatatypesSerializer.hpp"
#include <cstring>
#include <etherkitten/datatypes/esiparser.hpp>
#include <stdexcept>

namespace etherkitten::reader
{
	SlaveBlockSerializer SlaveBlock::serializer;

	/*
	 * block structure:
	 * *----------*-----------*-----------------*------*------*-----------*-----------------*
	 * | slave id | blocksize |      name       | pdos | coes | esi block | neighbors block |
	 * *----------*-----------*-----------------*------*------*-----------*-----------------*
	 * |    16    |     64    | ended by 0-byte |      |      |           |                 |
	 * *----------*-----------*-----------------*------*------*-----------*-----------------*
	 * second line contains the length in bits
	 */

	void SlaveBlockSerializer::serialize(const SlaveBlock& obj, Serialized& ser)
	{
		ser.write(obj.id, 0);
		ser.write(obj.getSerializedSize(), 2);
		ser.write(obj.name, 10);
		uint64_t offset = 10 + obj.name.size() + 1;
		for (auto& block : obj.pdos)
		{
			Serialized sub = ser.getAt(offset, block.getSerializedSize());
			block.getSerializer().serialize(block, sub);
			offset += sub.length;
		}
		for (auto& block : obj.coes)
		{
			Serialized sub = ser.getAt(offset, block.getSerializedSize());
			block.getSerializer().serialize(block, sub);
			offset += sub.length;
		}
		Serialized sub = ser.getAt(offset, obj.esiBlock.getSerializedSize());
		obj.esiBlock.getSerializer().serialize(obj.esiBlock, sub);
		offset += sub.length;
		Serialized serN = ser.getAt(offset, obj.neighborsBlock.getSerializedSize());
		obj.neighborsBlock.getSerializer().serialize(obj.neighborsBlock, serN);
	}

	Serialized SlaveBlockSerializer::serialize(const SlaveBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	datatypes::SlaveInfo SlaveBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		std::vector<datatypes::PDO> pdos;
		std::vector<datatypes::CoEEntry> coes;
		datatypes::ESIData esiData;
		std::vector<std::byte> esiBin;
		std::array<unsigned int, 4> neighbors;
		uint16_t id = ser.read<uint16_t>(0);
		context.slaveId = id;
		// this is the size which is not required here
		ser.read<uint64_t>(2);
		std::string name = ser.read<std::string>(10);
		uint64_t off = 10 + name.size() + 1;
		while (off + 2 < ser.length)
		{
			uint8_t type = ser.data[off];
			switch (type)
			{
			case 0: // PDO
			{
				uint64_t length = 8 + strlen(ser.data + off + 7);
				Serialized sub(ser.getAt(off, length));
				pdos.push_back(PDOBlock::serializer.parseSerialized(sub, context));
				off += sub.length;
				break;
			}
			case 1: // CoE
			{
				throw std::runtime_error(
				    "SlaveBlockSerializer should not be called to parse CoEBlock, instead it "
				    "should be parsed in CoEEntryBlockSerializer");
			}
			case 2: // ESI
			{
				uint16_t length = *reinterpret_cast<uint16_t*>(ser.data + off + 1);
				Serialized sub(ser.getAt(off, length));
				esiBin = ESIBlock::serializer.parseSerialized(sub, context);
				try
				{
					esiData = datatypes::esiparser::parseESI(esiBin);
				}
				catch (const datatypes::esiparser::ParseException& e)
				{
					// ignore it, data is simply not displayed in gui
				}
				off += sub.length;
				break;
			}
			case 3: // Neighbors
			{
				Serialized sub(ser.getAt(off, 9));
				neighbors = NeighborsBlock::serializer.parseSerialized(sub, context);
				off += sub.length;
				break;
			}
			case 5: // CoE Entry
			{
				uint16_t length = *reinterpret_cast<uint16_t*>(ser.data + off + 1);
				Serialized sub(ser.getAt(off, length));
				coes.push_back(CoEEntryBlock::serializer.parseSerialized(sub, context));
				off += sub.length;
				break;
			}
			}
		}

		return datatypes::SlaveInfo{ id, std::move(name), std::move(pdos), std::move(coes),
			std::move(esiData), std::move(esiBin), std::move(neighbors) };
	}

	uint64_t SlaveBlock::getSerializedSize() const
	{
		uint64_t size = 10;
		size += name.size() + 1;
		for (auto& block : pdos)
		{
			size += block.getSerializedSize();
		}
		for (auto& block : coes)
		{
			size += block.getSerializedSize();
		}
		size += esiBlock.getSerializedSize();
		size += neighborsBlock.getSerializedSize();
		return size;
	}
} // namespace etherkitten::reader
