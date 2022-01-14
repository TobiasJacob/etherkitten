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
 * \brief Defines SlaveDetailsBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include "pdodetails.hpp"
#include <inttypes.h>
#include <vector>

namespace etherkitten::reader
{
	class SlaveDetailsBlock;
	/*!
	 * \brief Serializes SlaveDetailsBlock and parses serialized SlaveDetailsBlock
	 */
	class SlaveDetailsBlockSerializer : public Serializer<SlaveDetailsBlock>
	{
	public:
		Serialized serialize(const SlaveDetailsBlock& obj) override;
		void serialize(const SlaveDetailsBlock& obj, Serialized& ser) override;
		SlaveDetailsBlock parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containing the data to find all PDO in IOMap
	 */
	class SlaveDetailsBlock : public Block<SlaveDetailsBlock>
	{
	public:
		SlaveDetailsBlock(uint16_t slaveId, std::vector<PDODetailsBlock>& pdoBlocks)
		    : slaveId(slaveId)
		    , pdoBlocks(pdoBlocks)
		{
		}
		friend SlaveDetailsBlockSerializer;

		/*!
		 * \brief slaveId of this block
		 */
		uint16_t slaveId;

		/*!
		 * \brief pdoBlocks in this block
		 */
		std::vector<PDODetailsBlock> pdoBlocks;

		/*!
		 * \brief Serializer that can be used for this block
		 */
		static SlaveDetailsBlockSerializer serializer;
		Serializer<SlaveDetailsBlock>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;
	};
} // namespace etherkitten::reader
