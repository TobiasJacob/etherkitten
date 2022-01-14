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

#include "esi.hpp"

namespace etherkitten::reader
{
	ESIBlockSerializer ESIBlock::serializer;

	/*
	 * block structure:
	 * *----*-----------*----------------------*
	 * | id | blocksize |        data          |
	 * *----*-----------*----------------------*
	 * |  8 |     16    | depends on blocksize |
	 * *----*-----------*----------------------*
	 * second line contains the length in bits
	 */

	void ESIBlockSerializer::serialize(const ESIBlock& obj, Serialized& ser)
	{
		ser.write(obj.type, 0);
		ser.write<uint16_t>(obj.getSerializedSize(), 1);
		uint64_t i = 3;
		for (auto& b : obj.esiBinary)
			ser.write(static_cast<uint8_t>(b), i++);
	}

	Serialized ESIBlockSerializer::serialize(const ESIBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	std::vector<std::byte> ESIBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		(void)context;
		if (ser.read<uint8_t>(0) != 2)
			throw std::runtime_error("block type is not correct");
		uint16_t size = ser.read<uint16_t>(1);

		std::vector<std::byte> data;
		data.reserve(size);
		for (uint64_t i = 3; i < size; ++i)
			data.push_back(static_cast<std::byte>(ser.read<uint8_t>(i)));
		return data;
	}

	uint64_t ESIBlock::getSerializedSize() const { return 3 + esiBinary.size(); }
} // namespace etherkitten::reader
