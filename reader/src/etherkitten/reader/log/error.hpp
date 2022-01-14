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

#include "Block.hpp"
#include "Serialized.hpp"
#include "Serializer.hpp"
#include <etherkitten/datatypes/dataobjects.hpp>
#include <inttypes.h>
#include <string>

namespace etherkitten::reader
{
    class ErrorBlock;

	/*!
	 * \brief Serializes ErrorBlock and parses serialized ErrorBlocks
	 */
	class ErrorBlockSerializer : public Serializer<ErrorBlock>
	{
	public:
		Serialized serialize(const ErrorBlock& obj) override;
		void serialize(const ErrorBlock& obj, Serialized& ser) override;
		ErrorBlock parseSerialized(Serialized& data, ParsingContext& context) override;
	};

	/*!
	 * \brief A Block representing an error message
	 */
	class ErrorBlock : public Block<ErrorBlock>
	{
	public:
		ErrorBlock(uint8_t severity, std::string message, uint64_t time, unsigned int slave1,
		    unsigned int slave2)
		    : severity(severity)
		    , message(message)
		    , time(time)
		    , slave1(convertSlaveTo16(slave1))
		    , slave2(convertSlaveTo16(slave2))
		{
		}
		friend ErrorBlockSerializer;

		/*!
		 * \brief A serializer that can be used to serialize and parse this Block
		 */
		static ErrorBlockSerializer serializer;
		Serializer<ErrorBlock>& getSerializer() const override { return serializer; }
		uint64_t getSerializedSize() const override;

		uint8_t severity;
		std::string message;
		uint64_t time;

		unsigned long getSlave1();
		unsigned long getSlave2();

	private:
		uint32_t ident = 0xA0000000;
		uint16_t slave1;
		uint16_t slave2;

		/*!
		 * \brief Convert the slave id in unsigned long to uint16_t.
		 * Convert values not representable to max value of uint16_t.
		 * \param slaveId slave id
		 * \return converted slave id
		 */
		uint16_t convertSlaveTo16(unsigned int slaveId);

		/*!
		 * \brief Convert the slave id in 16 bit to unsigned long.
		 * Convert max value of uint16_t to max value of unsigned long.
		 * \param slaveId slave id
		 * \return converted slave id
		 */
		unsigned int convertSlaveFrom16(uint16_t slaveId);
	};

} // namespace etherkitten::reader
