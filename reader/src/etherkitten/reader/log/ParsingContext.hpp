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
 * \brief Defines ParsingContext, a class that contains metadata while parsing a log.
 */

#include "../SlaveInformant.hpp"
#include "LogBusInfo.hpp"
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <sstream>
#include <stdexcept>

namespace etherkitten::reader
{
	/*!
	 * \brief The ParsingContext class contains metadata that is needed whilst parsing the log file.
	 * It is intended to set temporary values on a place where they are read from the log
	 * and retrieve the latest value where needed.
	 */
	class ParsingContext
	{
	public:
		/*!
		 * \brief Create a ParsingContext that has access to a SlaveInformant and LogBusInfo
		 * \param slaveInformant the SlaveInformant to use
		 * \param busInfo the LogBusInfo to use
		 */
		ParsingContext(SlaveInformant& slaveInformant, const LogBusInfo& busInfo)
		    : busInfo(busInfo)
		    , slaveInformant(slaveInformant)
		{
		}

		/*!
		 * \brief slaveId last slave id (temporary)
		 */
		uint16_t slaveId;

		/*!
		 * \brief Get the type of the CoE object of the slave with id slaveId
		 * and given index and subindex.
		 * \param index the index of the CoE object
		 * \param subindex the subindex of the CoE object
		 * \return the datatype of this object
		 * \exception std::runtime_error if the CoE object cannot be found
		 */
		datatypes::EtherCATDataTypeEnum getCoEType(uint16_t index, uint8_t subindex)
		{
			auto& vec = slaveInformant.getSlaveInfo(slaveId).getCoEs();
			for (auto& entry : vec)
			{
				for (auto& obj : entry.getObjects())
				{
					if (obj.getIndex() == index && obj.getSubIndex() == subindex)
						return obj.getType();
				}
			}
			std::stringstream sstr;
			sstr << "No CoE object found with index " << index << " and subindex " << subindex
			     << std::endl;
			throw std::runtime_error(sstr.str());
		}

		/*!
		 * \brief Get the size of the ioMap
		 * \return size of the ioMap
		 */
		uint64_t getIOMapSize() { return busInfo.ioMapUsedSize; }

		/*!
		 * \brief the LogBusInfo
		 */
		const LogBusInfo& busInfo;

	private:
		SlaveInformant& slaveInformant;
	};
} // namespace etherkitten::reader
