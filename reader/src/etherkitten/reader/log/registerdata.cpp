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

#include "registerdata.hpp"

#include "../DatatypesSerializer.hpp"
#include <cstring>
#include <stdexcept>

namespace etherkitten::reader
{
	RegisterDataBlockSerializer RegisterDataBlock::serializer;

	/*
	 * block structure:
	 * *----------------------------------------*-----------*--------------------------------------*
	 * | id (containing slaveId and registerId) | timestamp |                data                  |
	 * *----------------------------------------*-----------*--------------------------------------*
	 * |                   32                   |     64    | depends on datatype of this register |
	 * *----------------------------------------*-----------*--------------------------------------*
	 * second line contains the length in bits
	 */

	void RegisterDataBlockSerializer::serialize(const RegisterDataBlock& obj, Serialized& ser)
	{
		uint32_t ident = obj.registerId << 16;
		ident |= obj.slaveId;
		ser.write(ident, 0);
		ser.write(obj.timestamp, 4);
		switch (
		    datatypes::getRegisterByteLength(static_cast<datatypes::RegisterEnum>(obj.registerId)))
		{
		case 1:
			ser.write(static_cast<uint8_t>(obj.data), 12);
			break;
		case 2:
			ser.write(static_cast<uint16_t>(obj.data), 12);
			break;
		case 4:
			ser.write(static_cast<uint32_t>(obj.data), 12);
			break;
		case 8:
			ser.write(static_cast<uint64_t>(obj.data), 12);
			break;
		default:
			throw std::runtime_error("unexpected register length");
		}
	}

	Serialized RegisterDataBlockSerializer::serialize(const RegisterDataBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	RegisterDataBlock RegisterDataBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		(void)context;
		uint32_t ident = ser.read<uint32_t>(0);
		uint16_t regId = ((ident & 0xFFFF0000) >> 16);
		uint16_t slaveId = (ident & 0xFFFF);
		uint64_t timestamp = ser.read<uint64_t>(4);
		uint64_t data;
		switch (datatypes::getRegisterByteLength(static_cast<datatypes::RegisterEnum>(regId)))
		{
		case 1:
			data = ser.read<uint8_t>(12);
			break;
		case 2:
			data = ser.read<uint16_t>(12);
			break;
		case 4:
			data = ser.read<uint32_t>(12);
			break;
		case 8:
			data = ser.read<uint64_t>(12);
			break;
		default:
			throw std::runtime_error("unexpected register length");
		}
		return RegisterDataBlock{ static_cast<uint8_t>(regId), slaveId, timestamp, data };
	}

	uint64_t RegisterDataBlock::getSerializedSize() const
	{
		return 12
		    + datatypes::getRegisterByteLength(static_cast<datatypes::RegisterEnum>(registerId));
	}
} // namespace etherkitten::reader
