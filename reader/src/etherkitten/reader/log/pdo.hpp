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
 * \brief Defines PDOBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <etherkitten/datatypes/dataobjects.hpp>
#include <inttypes.h>
#include <string>

namespace etherkitten::reader
{
	class PDOBlock;
	/*!
	 * \brief Serializes PDOBlock and parses serialized PDOBlock to PDO object
	 */
	class PDOBlockSerializer : public Serializer<PDOBlock, datatypes::PDO>
	{
	public:
		Serialized serialize(const PDOBlock& obj) override;
		void serialize(const PDOBlock& obj, Serialized& ser) override;
		datatypes::PDO parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block representing a PDO object
	 */
	class PDOBlock : public Block<PDOBlock, datatypes::PDO>
	{
	public:
		PDOBlock(uint16_t index, uint16_t offset, uint16_t datatype, std::string name)
		    : index(index)
		    , offset(offset)
		    , datatype(datatype)
		    , name(name)
		{
		}
		friend PDOBlockSerializer;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static PDOBlockSerializer serializer;
		Serializer<PDOBlock, datatypes::PDO>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;

	private:
		uint8_t type = 0;
		uint16_t index;
		uint16_t offset;
		uint16_t datatype;
		std::string name;
	};
} // namespace etherkitten::reader
