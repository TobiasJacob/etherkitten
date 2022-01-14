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
 * \brief Defines ProcessDataBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <inttypes.h>
#include <vector>

namespace etherkitten::reader
{
	class ProcessDataBlock;
	/*!
	 * \brief Serializes ProcessDataBlock and parses serialized ProcessDataBlock
	 */
	class ProcessDataBlockSerializer : public Serializer<ProcessDataBlock>
	{
	public:
		Serialized serialize(const ProcessDataBlock& obj) override;
		void serialize(const ProcessDataBlock& obj, Serialized& ser) override;
		ProcessDataBlock parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containing IOMap with data
	 */
	class ProcessDataBlock : public Block<ProcessDataBlock>
	{
	public:
		ProcessDataBlock(uint64_t timestamp, Serialized&& data)
		    : timestamp(timestamp)
		    , data(data)
		{
		}
		friend ProcessDataBlockSerializer;

		/*!
		 * \brief timestamp for direct access
		 */
		uint64_t timestamp;

		/*!
		 * \brief data for direct access
		 */
		Serialized data;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static ProcessDataBlockSerializer serializer;
		Serializer<ProcessDataBlock>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;

	private:
		uint32_t ident = 0x80000000;
	};
} // namespace etherkitten::reader
