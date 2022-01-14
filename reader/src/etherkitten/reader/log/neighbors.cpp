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

#include "neighbors.hpp"

namespace etherkitten::reader
{
	uint8_t NeighborsBlock::type = 3;
	NeighborsBlockSerializer NeighborsBlock::serializer;

	/*
	 * block structure:
	 * *----*------------*------------*------------*------------*
	 * | id | neighbor 1 | neighbor 2 | neighbor 3 | neighbor 4 |
	 * *----*------------*------------*------------*------------*
	 * |  8 |    16      |    16      |    16      |    16      |
	 * *----*------------*------------*------------*------------*
	 * second line contains the length in bits
	 */

	NeighborsBlock::NeighborsBlock(const std::array<unsigned int, 4>& neighborIds)
	    : neighbors(convertNeighborsArray(neighborIds))
	{
	}

	void NeighborsBlockSerializer::serialize(const NeighborsBlock& obj, Serialized& ser)
	{
		ser.write(obj.type, 0);
		uint64_t i = 1;
		for (size_t j = 0; j < 4; ++j)
		{
			ser.write(obj.neighbors.at(j), i);
			i += 2;
		}
	}

	Serialized NeighborsBlockSerializer::serialize(const NeighborsBlock& obj)
	{
		Serialized ser(obj.getSerializedSize());
		serialize(obj, ser);
		return ser;
	}

	std::array<unsigned int, 4> NeighborsBlockSerializer::parseSerialized(
	    Serialized& ser, ParsingContext& context)
	{
		(void)context;
		if (ser.read<uint8_t>(0) != NeighborsBlock::type)
			throw std::runtime_error("block type is incorrect");
		std::array<uint16_t, 4> data;
		for (size_t i = 0; i < 4; ++i)
			data[i] = ser.read<uint16_t>((2 * i) + 1);
		return convertNeighborsArray(data);
	}

	uint64_t NeighborsBlock::getSerializedSize() const { return 9; }

	std::array<uint16_t, 4> NeighborsBlock::convertNeighborsArray(
	    const std::array<unsigned int, 4>& array)
	{
		std::array<uint16_t, 4> data;
		for (size_t i = 0; i < 4; ++i)
		{
			if (array[i] == std::numeric_limits<unsigned int>::max())
				data[i] = 0xFFFF;
			else
				data[i] = array[i];
		}
		return data;
	}

	std::array<unsigned int, 4> NeighborsBlockSerializer::convertNeighborsArray(
	    std::array<uint16_t, 4>& array)
	{
		std::array<unsigned int, 4> data;
		for (size_t i = 0; i < 4; ++i)
		{
			if (array[i] == 0xFFFF)
				data[i] = std::numeric_limits<unsigned int>::max();
			else
				data[i] = array[i];
		}
		return data;
	}
} // namespace etherkitten::reader
