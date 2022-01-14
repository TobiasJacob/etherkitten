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
 * \brief Defines a class to hold information on EtherCAT slaves as well as the EtherCAT
 * state machine.
 */

#include <array>
#include <optional>
#include <type_traits>
#include <vector>

#include "dataobjects.hpp"
#include "esidata.hpp"

namespace etherkitten::datatypes
{

	/*!
	 * \brief The SlaveInfo class holds information and DataObjects from one EtherCAT slave.
	 */
	class SlaveInfo
	{
	public:
		/*!
		 * \brief Construct a new SlaveInfo with the given parameters.
		 *
		 * Unused ports should be marked by storing `std::numeric_limits<unsigned int>::max()`
		 * at that index in the array.
		 * \param id the ID of the slave on the bus (index in the logical line)
		 * \param name the name of the slave
		 * \param pdos the PDOs of the slave
		 * \param coes the CoE objects of the slave
		 * \param esiData the ESI data of the slave in a parsed struct format
		 * \param esiBinary the ESI of the slave, encoded in the binary SII format
		 * \param neighbors the neighbors of this slave ordered by port
		 */
		SlaveInfo(unsigned int id, std::string&& name, std::vector<PDO>&& pdos,
		    std::vector<CoEEntry>&& coes, ESIData&& esiData, std::vector<std::byte>&& esiBinary,
		    std::array<unsigned int, 4>&& neighbors);

		/*!
		 * \brief Construct a new SlaveInfo with the given parameters.
		 *
		 * Unused ports should be marked by storing `std::numeric_limits<unsigned int>::max()`
		 * at that index in the array.
		 * \param id the ID of the slave on the bus (index in the logical line)
		 * \param name the name of the slave
		 * \param pdos the PDOs of the slave
		 * \param coes the CoE objects of the slave
		 * \param neighbors the neighbors of this slave ordered by port
		 */
		SlaveInfo(unsigned int id, std::string&& name, std::vector<PDO>&& pdos,
		    std::vector<CoEEntry>&& coes, std::array<unsigned int, 4>&& neighbors);

		/*!
		 * \brief Get the ID of the slave on the bus.
		 * \return the ID of the slave
		 */
		unsigned int getID() const;

		/*!
		 * \brief Get the Name of the slave.
		 * \return the name of the slave
		 */
		const std::string& getName() const;

		/*!
		 * \brief Get a list of all the PDOs of the slave.
		 * \return a list of the PDOs
		 */
		const std::vector<PDO>& getPDOs() const;

		/*!
		 * \brief Get a list of all the CoE objects of the slave.
		 * \return a list of the CoE objects
		 */
		const std::vector<CoEEntry>& getCoEs() const;

		/*!
		 * \brief Get a list of all the registers of the slave.
		 * \return a list of the registers
		 */
		const std::vector<Register>& getRegisters() const;

		/*!
		 * \brief Get a list of all the error statistics of the slave.
		 * \return a list of the error statistics
		 */
		const std::vector<ErrorStatistic>& getErrorStatistics() const;

		/*!
		 * \brief Get the ESI data of the slave in a parsed format.
		 * \return the ESI data of the slave
		 */
		const std::optional<ESIData>& getESI() const;

		/*!
		 * \brief Get the binary encoded ESI data of the slave.
		 * \return the binary ESI data of the slave
		 */
		const std::vector<std::byte>& getESIBinary() const;

		/*!
		 * \brief Get the neighbors of this slave.
		 *
		 * Unused ports are marked by `std::numeric_limits<unsigned int>::max()`
		 * at that index in the array.
		 * \return the neighbors of this slave
		 */
		const std::array<unsigned int, 4>& getNeighbors() const;

	private:
		static std::vector<Register> makeRegisterVector(unsigned int id);
		static std::vector<ErrorStatistic> makeErrorStatisticVector(unsigned int id);

		unsigned int id;
		std::string name;
		std::vector<PDO> pdos;
		std::vector<CoEEntry> coes;
		std::vector<Register> registers;
		std::vector<ErrorStatistic> statistics;
		std::vector<std::byte> esiBinary;
		std::optional<ESIData> esiData;
		std::array<unsigned int, 4> neighbors;
	};

	/*!
	 * \brief Represents the state of an EtherCAT bus as a whole.
	 *
	 * OP means all slaves are in the operational state,
	 * while SAFE_OP means they are all in the safe-operational state.
	 */
	enum class BusStatus
	{
		OP,
		SAFE_OP
	};

	/*!
	 * \brief Represents the available capabilities of a bus.
	 *
	 * READ_WRITE_SAFE_OP is equivalent to SAFE_OP in BusStatus.
	 * READ_WRITE_OP is equivalent to OP in BusStatus.
	 */
	enum class BusMode
	{
		NOT_AVAILABLE,
		READ_ONLY,
		READ_WRITE_SAFE_OP,
		READ_WRITE_OP
	};

} // namespace etherkitten::datatypes
