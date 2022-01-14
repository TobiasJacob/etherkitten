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
 * \brief Defines CoEDataBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <any>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <inttypes.h>

namespace etherkitten::reader
{
	class CoEDataBlock;

	/*!
	 * \brief Serializes CoEDataBlock and parses serialized CoEDataBlock
	 */
	class CoEDataBlockSerializer : public Serializer<CoEDataBlock>
	{
	public:
		Serialized serialize(const CoEDataBlock& obj) override;
		void serialize(const CoEDataBlock& obj, Serialized& ser) override;
		CoEDataBlock parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block of CoE object data
	 */
	class CoEDataBlock : public Block<CoEDataBlock>
	{
	public:
		CoEDataBlock(uint16_t slaveId, uint64_t timestamp, uint16_t index, uint8_t subIndex,
		    std::any&& data, datatypes::EtherCATDataTypeEnum dataType)
		    : slaveId(slaveId)
		    , timestamp(timestamp)
		    , index(index)
		    , subIndex(subIndex)
		    , data(data)
		    , dataType(dataType)
		{
		}
		friend CoEDataBlockSerializer;

		/*!
		 * \brief slaveId of this block
		 */
		uint16_t slaveId;

		/*!
		 * \brief timestamp of this block
		 */
		uint64_t timestamp;

		/*!
		 * \brief coe index
		 */
		uint16_t index;

		/*!
		 * \brief coe subindex
		 */
		uint16_t subIndex;

		/*!
		 * \brief data in this block
		 */
		std::any data;

		/*!
		 * \brief dataType of the data in this block
		 */
		datatypes::EtherCATDataTypeEnum dataType;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static CoEDataBlockSerializer serializer;
		Serializer<CoEDataBlock>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;

	private:
		uint16_t ident = 0x9000;
	};
} // namespace etherkitten::reader
