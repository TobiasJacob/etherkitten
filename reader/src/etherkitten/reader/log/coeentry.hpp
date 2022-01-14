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
 * \brief Defines CoEEntryBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include "coe.hpp"
#include <etherkitten/datatypes/dataobjects.hpp>
#include <inttypes.h>
#include <string>

namespace etherkitten::reader
{
	class CoEEntryBlock;
	/*!
	 * \brief Serializes CoEEntryBlock and parses serialized CoEEntryBlock to CoEEntry
	 */
	class CoEEntryBlockSerializer : public Serializer<CoEEntryBlock, datatypes::CoEEntry>
	{
	public:
		Serialized serialize(const CoEEntryBlock& obj) override;
		void serialize(const CoEEntryBlock& obj, Serialized& ser) override;
		datatypes::CoEEntry parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containing the data of a CoEEntry
	 */
	class CoEEntryBlock : public Block<CoEEntryBlock, datatypes::CoEEntry>
	{
	public:
		CoEEntryBlock(uint16_t index, uint8_t objectCode, const std::string&& name,
		    std::vector<CoEBlock>&& coes)
		    : index(index)
		    , objectCode(objectCode)
		    , name(name)
		    , coes(coes)
		{
		}
		friend CoEEntryBlockSerializer;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static CoEEntryBlockSerializer serializer;
		Serializer<CoEEntryBlock, datatypes::CoEEntry>& getSerializer() const override
		{
			return serializer;
		}
		uint64_t getSerializedSize() const override;

	private:
		uint8_t type = 5;
		uint16_t index;
		uint8_t objectCode;
		const std::string name;
		std::vector<CoEBlock> coes;
	};
} // namespace etherkitten::reader
