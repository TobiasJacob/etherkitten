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
 * \brief Defines PDODetailsBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <inttypes.h>
#include <string>

namespace etherkitten::reader
{
	class PDODetailsBlock;
	/*!
	 * \brief Serializes PDODetailsBlock and parses serialized PDODetailsBlock
	 */
	class PDODetailsBlockSerializer : public Serializer<PDODetailsBlock>
	{
	public:
		Serialized serialize(const PDODetailsBlock& obj) override;
		void serialize(const PDODetailsBlock& obj, Serialized& ser) override;
		PDODetailsBlock parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containg data on how to find PDO in IOMap
	 */
	class PDODetailsBlock : public Block<PDODetailsBlock>
	{
	public:
		PDODetailsBlock(uint16_t index, uint16_t offset, uint8_t bitLength, uint16_t datatype)
		    : index(index)
		    , offset(offset)
		    , bitLength(bitLength)
		    , datatype(datatype)
		{
		}
		friend PDODetailsBlockSerializer;

		/*!
		 * \brief pdo index
		 */
		uint16_t index;

		/*!
		 * \brief pdo offset
		 */
		uint16_t offset;

		/*!
		 * \brief bitLength of the pdo
		 */
		uint8_t bitLength;

		/*!
		 * \brief datatype of the pdo
		 */
		uint16_t datatype;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static PDODetailsBlockSerializer serializer;
		Serializer<PDODetailsBlock>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;
	};
} // namespace etherkitten::reader
