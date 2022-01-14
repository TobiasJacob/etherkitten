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

#pragma once
/*!
 * \file
 * \brief Defines NeighborsBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <array>
#include <inttypes.h>

namespace etherkitten::reader
{
	class NeighborsBlock;
	/*!
	 * \brief Serializes NeighborsBlock and parses serialized NeighborsBlock to
	 * array<unsigned int, 4>
	 *
	 * Neighbors block uses 2 bytes per neighbor. So a maximum of 2^16-1 slaves is supported.
	 */
	class NeighborsBlockSerializer : public Serializer<NeighborsBlock, std::array<unsigned int, 4>>
	{
	public:
		Serialized serialize(const NeighborsBlock& obj) override;
		void serialize(const NeighborsBlock& obj, Serialized& ser) override;
		std::array<unsigned int, 4> parseSerialized(
		    Serialized& data, ParsingContext& context) override;

	private:
		std::array<unsigned int, 4> convertNeighborsArray(std::array<uint16_t, 4>& array);
	};

	/*!
	 * \brief Block containing the ids of neighbor slaves
	 */
	class NeighborsBlock : public Block<NeighborsBlock, std::array<unsigned int, 4>>
	{
	public:
		NeighborsBlock(const std::array<unsigned int, 4>& neighborIds);
		friend NeighborsBlockSerializer;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static NeighborsBlockSerializer serializer;
		Serializer<NeighborsBlock, std::array<unsigned int, 4>>& getSerializer() const override
		{
			return serializer;
		}
		uint64_t getSerializedSize() const override;

	private:
		static uint8_t type;
		std::array<uint16_t, 4> neighbors;

		std::array<uint16_t, 4> convertNeighborsArray(const std::array<unsigned int, 4>& array);
	};
} // namespace etherkitten::reader
