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
 * \brief Defines SlaveBlock and the corresponding Serializer.
 */

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include "coe.hpp"
#include "coeentry.hpp"
#include "esi.hpp"
#include "neighbors.hpp"
#include "pdo.hpp"
#include <etherkitten/datatypes/SlaveInfo.hpp>

namespace etherkitten::reader
{
	class SlaveBlock;
	/*!
	 * \brief Serializes SlaveBlock and parses serialized SlaveBlock to SlaveInfo
	 */
	class SlaveBlockSerializer : public Serializer<SlaveBlock, datatypes::SlaveInfo>
	{
	public:
		Serialized serialize(const SlaveBlock& obj) override;
		void serialize(const SlaveBlock& obj, Serialized& ser) override;
		datatypes::SlaveInfo parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief Block containing data of SlaveInfo
	 */
	class SlaveBlock : public Block<SlaveBlock, datatypes::SlaveInfo>
	{
	public:
		SlaveBlock(uint16_t& id, std::string& name, std::vector<PDOBlock>& pdos,
		    std::vector<CoEEntryBlock>& coes, ESIBlock& esiBlock, NeighborsBlock& neighborsBlock)
		    : id(id)
		    , name(name)
		    , pdos(pdos)
		    , coes(coes)
		    , esiBlock(esiBlock)
		    , neighborsBlock(neighborsBlock)
		{
		}
		friend SlaveBlockSerializer;

		/*!
		 * \brief Serializer that can be used for SlaveBlock
		 */
		static SlaveBlockSerializer serializer;
		Serializer<SlaveBlock, datatypes::SlaveInfo>& getSerializer() const override
		{
			return serializer;
		}
		uint64_t getSerializedSize() const override;

	private:
		uint16_t& id;
		std::string& name;
		std::vector<PDOBlock>& pdos;
		std::vector<CoEEntryBlock>& coes;
		ESIBlock& esiBlock;
		NeighborsBlock& neighborsBlock;
	};
} // namespace etherkitten::reader
