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
 * \brief Defines methods for reading PDO information from an EtherCAT bus.
 */

#include "../endianness.hpp"
#include "impl-common.hpp"

#include <etherkitten/datatypes/esidata.hpp>

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief The SyncMTypes enum encodes the different types a Sync Manager can have,
	 * as encoded in the CoE object dictionary of a slave.
	 */
	enum class SyncMTypes
	{
		UNUSED = 0,
		MAILBOX_RECEIVE = 1,
		MAILBOX_SEND = 2,
		PDO_MASTER_TO_SLAVE = 3,
		PDO_SLAVE_TO_MASTER = 4,
	};

	/*!
	 * \brief Perform an SDO read from the given slave's object dictionary at the given
	 * index and subindex.
	 *
	 * The type parameter should have at minimum the width of the data at that index
	 * and subindex.
	 * \tparam T the type of the result
	 * \param slave the slave to read from
	 * \param index the index in the object dictionary to read from
	 * \param subindex the subindex of the index to read from
	 * \return the value if successful, nothing if not
	 */
	template<typename T>
	std::optional<T> performSDOread(unsigned int slave, uint16_t index, uint8_t subindex)
	{
		T result = 0;
		int tSize = sizeof(T);
		int workingCounter
		    = ec_SDOread(slave, index, subindex, (boolean) false, &tSize, &result, EC_TIMEOUTRXM);
		if (workingCounter == 0)
		{
			return {};
		}
		return flipBytesIfBigEndianHost(result);
	}

	/*!
	 * \brief Read the PDOs of one slave via the CoE object dictionary.
	 *
	 * If the returned errors contain a MEDIUM severity error, a PDO mapping
	 * could not be read and PDO reading for this slave must be stopped to
	 * keep the PDOOffsets consistent.
	 * If the coes do not contain the object a PDO refers to, that PDO will
	 * not be made available in the PDOList, but its offset will be counted
	 * and reflected in the offsets of the other PDOs.
	 * \param slave the slave to read the PDOs of
	 * \param coes CoE entries to read the name and type of PDOs from
	 * \return the PDOs in a list and their respective offsets in a map
	 * along with any errors that occurred during reading
	 */
	MayError<PDOResult> readPDOsViaCoE(
	    unsigned int slave, const std::vector<datatypes::CoEEntry>& coes);

	/*!
	 * \brief Read the PDOs of one slave via the ESI data of that slave.
	 * \param slave the slave to read the PDOs of
	 * \param esiData the ESI data to read the PDOs from
	 * \return the PDOs in a list and their offsets in a map, along with any errors that occurred
	 * during reading
	 */
	MayError<PDOResult> readPDOsViaESI(unsigned int slave, const datatypes::ESIData& esiData);

} // namespace etherkitten::reader::bSInformant
