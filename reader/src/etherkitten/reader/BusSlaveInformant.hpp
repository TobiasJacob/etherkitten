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
 * \brief Defines the BusSlaveInformant, which initializes and gathers data on an EtherCAT bus.
 */

#include <array>
#include <functional>
#include <future>
#include <stdexcept>
#include <string>
#include <vector>

#include "SlaveInformant.hpp"

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/errors.hpp>

namespace etherkitten::reader
{
	static const size_t ioMapSize = 32768;

	/*!
	 * \brief The BusInfo struct contains general information needed to control an EtherCAT bus.
	 */
	struct BusInfo
	{
		/*!
		 * \brief The array SOEM will use to read and write its IOMap.
		 *
		 * SOEM will always write to and read from the beginning of this array,
		 * though not necessarily to its end (see ioMapUsedSize).
		 *
		 * Note that the contents of this array do not necessarily adhere to the byte order
		 * of the host system.
		 */
		std::array<uint8_t, ioMapSize> ioMap{};

		/*!
		 * \brief The actual size of the IOMap as used by SOEM.
		 *
		 * Everything beyond the index ioMapUsedSize - 1 is not read or written to,
		 * and the contents are undefined.
		 */
		size_t ioMapUsedSize;

		/*!
		 * \brief Contains the offsets for PDO objects relative to their slave starting point in the
		 * ioMap.
		 *
		 * SOEM automatically maps each slave's PDOs into the IOMap and tells us the starting point
		 * for each slave's inputs and outputs, but not which PDO is at which offset.
		 *
		 * This map contains exactly that information - the offset of an input / output PDO relative
		 * to its slave's input / output starting point.
		 */
		std::unordered_map<datatypes::PDO, datatypes::PDOInfo, datatypes::PDOHash,
		    datatypes::PDOEqual>
		    pdoOffsets;

		std::unordered_map<datatypes::CoEObject, datatypes::CoEInfo, datatypes::CoEObjectHash,
		    datatypes::CoEObjectEqual>
		    coeInfos;

		/*!
		 * \brief The status of the slaves on the bus after the BusSlaveInformant initialized them.
		 */
		datatypes::BusStatus statusAfterInit;
	};

	/*!
	 * \brief The BusSlaveInformant class is a SlaveInformant that initializes an EtherCAT bus
	 * and reads slave and bus information from it.
	 *
	 * Multiple BusSlaveInformants MUST NOT exist at the same time.
	 * This classes' interaction with the SOEM backend works via global variables, which would
	 * lead to inconsistent states if used for multiple busses simultaneously.
	 */
	class BusSlaveInformant : public SlaveInformant
	{
	public:
		/*!
		 * \brief Construct a new BusSlaveInformant that accesses an EtherCAT bus on the given
		 * network interface.
		 *
		 * When this constructor exits, all slaves on the EtherCAT bus on the interface will be
		 * either in the operational state or in the safe operational state.
		 * \param interface the network interface to connect to (e.g. "enp2s0")
		 * \exception BusSlaveInformantError if an unrecoverable error occured while
		 * initializing the bus.
		 */
		BusSlaveInformant(std::string interface);

		/*!
		 * \brief Construct a new BusSlaveInformant that accesses an EtherCAT bus on the given
		 * network interface.
		 *
		 * When this constructor exits, all slaves on the EtherCAT bus on the interface will be
		 * either in the operational state or in the safe operational state.
		 * This constructor will additionally report its progress by calling the progressFunction
		 * regularly with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param interface the network interface to connect to (e.g. "enp2s0")
		 * \param progressFunction a function that is called to report initialization progress
		 * \exception BusSlaveInformantError if an unrecoverable error occured while
		 * initializing the bus.
		 */
		BusSlaveInformant(
		    std::string interface, std::function<void(int, std::string)> progressFunction);

		/*!
		 * \brief Construct a new BusSlaveInformant that accesses an EtherCAT bus on the given
		 * socket.
		 *
		 * When this constructor exits, all slaves on the EtherCAT bus on the interface will be
		 * either in the operational state or in the safe operational state.
		 * \param socket the network socket to connect to
		 * \exception BusSlaveInformantError if an unrecoverable error occured while
		 * initializing the bus.
		 */
		BusSlaveInformant(int socket);

		/*!
		 * \brief Construct a new BusSlaveInformant that accesses an EtherCAT bus on the given
		 * socket.
		 *
		 * When this constructor exits, all slaves on the EtherCAT bus on the interface will be
		 * either in the operational state or in the safe operational state.
		 * This constructor will additionally report its progress by calling the progressFunction
		 * regularly with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param socket the network socket to connect to
		 * \param progressFunction a function that is called to report initialization progress
		 * \exception BusSlaveInformantError if an unrecoverable error occured while
		 * initializing the bus.
		 */
		BusSlaveInformant(int socket, std::function<void(int, std::string)> progressFunction);

		unsigned int getSlaveCount() const override;

		const datatypes::SlaveInfo& getSlaveInfo(unsigned int index) const override;

		uint64_t getIOMapSize() const override;

		/*!
		 * \brief Get the information on the EtherCAT bus that is required to operate it.
		 * \return the BusInfo for the bus this BusSlaveInformant was constructed with
		 */
		BusInfo& getBusInfo();

		/*!
		 * \brief Get all error messages that accumulated during the initialization process.
		 *
		 * Errors with a LOW severity only affect singular DataObjects, which may be
		 * unavailable. Errors with a MEDIUM severity affect larger parts of the SlaveInfo for
		 * one slave - for example, one SlaveInfo might be lacking PDOs. Errors with a FATAL
		 * severity will not appear in this vector, as these would have caused an exception to
		 * be thrown in this BusSlaveInformant's constructor.
		 * \return all the accumulated error messages.
		 */
		const std::vector<datatypes::ErrorMessage> getInitializationErrors() override;

	private:
		std::vector<datatypes::SlaveInfo> slaveInfos;
		BusInfo busInfo;
		std::vector<datatypes::ErrorMessage> initializationErrors;

		void performSlaveInformantInit(const std::function<int(void)>& initFunc,
		    std::function<void(int, std::string)>& progressFunction);
		std::vector<datatypes::ErrorMessage> initializeBus(const std::function<int(void)>& initFunc,
		    std::function<void(int, std::string)>& progressFunction);
		std::vector<datatypes::ErrorMessage> readSlaveInfosFromBus(
		    std::function<void(int, std::string)>& progressFunction);
		std::vector<datatypes::ErrorMessage> setSlavesIntoOP(
		    std::function<void(int, std::string)>& progressFunction);

		static constexpr int initBusProgress = 0;
		static constexpr int pdoMappingProgress = 5;
		static constexpr int safeOPProgress = 10;
		static constexpr int beginSlaveInfoProgress = 15;
		static constexpr int opProgress = 95;
		static constexpr int finishedProgress = 100;
	};

} // namespace etherkitten::reader
