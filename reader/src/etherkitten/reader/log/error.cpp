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

#include "error.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

namespace etherkitten::reader
{
    ErrorBlockSerializer ErrorBlock::serializer;

	/*
	 * block structure:
	 * *----*-----------*-----------*---------*---------*----------*--------------------------*
	 * | id | timestamp | blocksize | slave 1 | slave 2 | severity |         message          |
	 * *----*-----------*-----------*---------*---------*----------*--------------------------*
	 * | 32 |     64    |     64    |    16   |    16   |    8     | depends on string length |
	 * *----*-----------*-----------*---------*---------*----------*--------------------------*
	 * second line contains the length in bits
	 */

	void ErrorBlockSerializer::serialize(const ErrorBlock& obj, Serialized& ser)
	{
		ser.write(obj.ident, 0);
		ser.write(obj.time, 4);
		ser.write(obj.getSerializedSize(), 12);
		ser.write(obj.slave1, 20);
		ser.write(obj.slave2, 22);
		ser.write(obj.severity, 24);
		ser.write(obj.message, 25);
	}

	Serialized ErrorBlockSerializer::serialize(const ErrorBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	ErrorBlock ErrorBlockSerializer::parseSerialized(Serialized& ser, ParsingContext& context)
	{
		uint64_t timestamp = ser.read<uint64_t>(4);
		uint8_t severity = ser.read<uint8_t>(24);
		uint16_t slave1 = ser.read<uint16_t>(20);
		uint16_t slave2 = ser.read<uint16_t>(22);
		return ErrorBlock{ severity, ser.read<std::string>(25), timestamp, slave1, slave2 };
	}

	uint64_t ErrorBlock::getSerializedSize() const { return 25 + message.size() + 1; }

	unsigned int ErrorBlock::convertSlaveFrom16(uint16_t slaveId)
	{
		if (slaveId == std::numeric_limits<uint16_t>::max())
			return std::numeric_limits<unsigned int>::max();
		return slaveId;
	}

	unsigned long ErrorBlock::getSlave1() { return convertSlaveFrom16(slave1); }
	unsigned long ErrorBlock::getSlave2() { return convertSlaveFrom16(slave2); }

	uint16_t ErrorBlock::convertSlaveTo16(unsigned int slaveId)
	{
		if (slaveId >= std::numeric_limits<uint16_t>::max())
			return std::numeric_limits<uint16_t>::max();
		return slaveId;
	}
} // namespace etherkitten::reader
