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
 * \brief Defines ESIBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <inttypes.h>
#include <vector>

namespace etherkitten::reader
{
	class ESIBlock;
	/*!
	 * \brief Serializes ESIBlock and parses serialized ESIBlock to vector<byte>
	 */
	class ESIBlockSerializer : public Serializer<ESIBlock, std::vector<std::byte>>
	{
	public:
		Serialized serialize(const ESIBlock& obj) override;
		void serialize(const ESIBlock& obj, Serialized& ser) override;
		std::vector<std::byte> parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containing ESI data of a slave
	 */
	class ESIBlock : public Block<ESIBlock, std::vector<std::byte>>
	{
	public:
		ESIBlock(const std::vector<std::byte>& esiBinary)
		    : esiBinary(esiBinary)
		{
		}
		friend ESIBlockSerializer;

		/*!
		 * \brief A serializer that can be used to serialize and parse ESIBlock
		 */
		static ESIBlockSerializer serializer;
		Serializer<ESIBlock, std::vector<std::byte>>& getSerializer() const override
		{
			return serializer;
		}
		uint64_t getSerializedSize() const override;

	private:
		uint8_t type = 2;
		const std::vector<std::byte>& esiBinary;
	};
} // namespace etherkitten::reader
