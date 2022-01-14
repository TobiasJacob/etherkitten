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
 * \brief Defines RegisterDataBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <any>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <inttypes.h>
#include <vector>

namespace etherkitten::reader
{
	class RegisterDataBlock;
	/*!
	 * \brief Serializes RegisterDataBlock and parses serialized RegisterDataBlock
	 */
	class RegisterDataBlockSerializer : public Serializer<RegisterDataBlock>
	{
	public:
		Serialized serialize(const RegisterDataBlock& obj) override;
		void serialize(const RegisterDataBlock& obj, Serialized& ser) override;
		RegisterDataBlock parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containing data read from a register with timestamp
	 */
	class RegisterDataBlock : public Block<RegisterDataBlock>
	{
	public:
		RegisterDataBlock(uint16_t registerId, uint16_t slaveId, uint64_t timestamp, uint64_t data)
		    : registerId(registerId)
		    , slaveId(slaveId)
		    , timestamp(timestamp)
		    , data(data)
		{
		}
		friend RegisterDataBlockSerializer;

		/*!
		 * \brief registerId of this block
		 */
		uint16_t registerId;

		/*!
		 * \brief slaveId of this block
		 */
		uint16_t slaveId;

		/*!
		 * \brief timestamp of this block
		 */
		uint64_t timestamp;

		/*!
		 * \brief data in this block
		 */
		uint64_t data;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static RegisterDataBlockSerializer serializer;
		Serializer<RegisterDataBlock>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;
	};
} // namespace etherkitten::reader
