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
 * \brief Defines CoEBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <etherkitten/datatypes/dataobjects.hpp>
#include <inttypes.h>
#include <string>

namespace etherkitten::reader
{
	class CoEBlock;

	/*!
	 * \brief Serializes CoEBlock and parses serialized CoEBlocks into CoEObjects
	 */
	class CoEBlockSerializer : public Serializer<CoEBlock, datatypes::CoEObject>
	{
	public:
		Serialized serialize(const CoEBlock& obj) override;
		void serialize(const CoEBlock& obj, Serialized& ser) override;
		datatypes::CoEObject parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief A Block representing the data of a CoEObject
	 */
	class CoEBlock : public Block<CoEBlock, datatypes::CoEObject>
	{
	public:
		CoEBlock(uint16_t index, uint8_t subindex, uint16_t datatype, std::string name)
		    : index(index)
		    , subindex(subindex)
		    , datatype(datatype)
		    , name(name)
		{
		}
		friend CoEBlockSerializer;

		/*!
		 * \brief A serializer that can be used to serialize and parse this Block
		 */
		static CoEBlockSerializer serializer;
		Serializer<CoEBlock, datatypes::CoEObject>& getSerializer() const override
		{
			return serializer;
		}
		uint64_t getSerializedSize() const override;

	private:
		uint8_t type = 1;
		uint16_t index;
		uint8_t subindex;
		uint16_t datatype;
		std::string name;
	};
} // namespace etherkitten::reader
