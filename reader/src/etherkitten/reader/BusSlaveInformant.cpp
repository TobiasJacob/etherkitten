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


#include "BusSlaveInformant.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/esidata.hpp>
#include <etherkitten/datatypes/esiparser.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

#include "BusSlaveInformant-impl/coe.hpp"
#include "BusSlaveInformant-impl/esi.hpp"
#include "BusSlaveInformant-impl/pdo.hpp"
#include "BusSlaveInformant-impl/slavename.hpp"
#include "BusSlaveInformant-impl/topology.hpp"

namespace etherkitten::reader
{

	using namespace bSInformant;

	/*!
	 * \brief Use SOEM to start communicating on the EtherCAT bus.
	 *
	 * This method creates the IO mapping for the PDOs and attempts to get all slaves
	 * into the Safe-Operational state. The returned error messages will contain
	 * fatal errors iff this method failed.
	 * \param initFunc which function to call to initialize the bus
	 * \return errors that occured during initialization
	 */
	std::vector<ErrorMessage> BusSlaveInformant::initializeBus(
	    const std::function<int(void)>& initFunc,
	    std::function<void(int, std::string)>& progressFunction)
	{
		std::vector<ErrorMessage> errors;

		progressFunction(initBusProgress, "Initializing EtherCAT bus...");

		// Start EtherCAT bus
		int success = initFunc();
		if (success == 0)
		{
			errors.emplace_back("Could not initialize EtherCAT bus.", ErrorSeverity::FATAL);
			addSOEMErrorsToVector(errors, ErrorSeverity::FATAL);
			return errors;
		}

		progressFunction(pdoMappingProgress, "Creating PDO mapping...");

		// Create PDO mapping
		ec_config(0, busInfo.ioMap.data());

		if (ec_slavecount == 0)
		{
			errors.emplace_back("No slaves were found on this interface", ErrorSeverity::FATAL);
			addSOEMErrorsToVector(errors, ErrorSeverity::FATAL);
			return errors;
		}

		// Find out how large the IO map actually needs to be
		busInfo.ioMapUsedSize = ec_group[0].Ibytes + ec_group[0].Obytes;

		// Configure distributed clock (not completely sure what this is good for)
		ec_configdc();

		progressFunction(safeOPProgress, "Setting slaves into SAFE-OP...");

		// Check if all slaves (0 == all) are in SAFE-OP
		ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 3);
		if (ec_slave[0].state != EC_STATE_SAFE_OP)
		{
			ec_close();
			errors.emplace_back("Could not get all slaves into SAFE-OP.", ErrorSeverity::FATAL);
			addSOEMErrorsToVector(errors, ErrorSeverity::FATAL);
			return errors;
		}

		// At this point, all slaves should be in SAFE-OP
		addSOEMErrorsToVector(errors, ErrorSeverity::MEDIUM);
		return errors;
	}

	/*!
	 * \brief Read all the slave information available from the EtherCAT bus.
	 *
	 * This method will generate all the SlaveInfo entries and calculate
	 * the PDOOffsets for the BusInfo struct.
	 * \return errors that occured while reading the slave information
	 */
	std::vector<ErrorMessage> BusSlaveInformant::readSlaveInfosFromBus(
	    std::function<void(int, std::string)>& progressFunction)
	{
		std::vector<ErrorMessage> accumulatedErrors;

		auto topology = readBusTopology();

		for (int slave = 1; slave <= ec_slavecount; ++slave)
		{
			int progress = std::floor(
			    (opProgress - beginSlaveInfoProgress) * ((double)(slave - 1) / ec_slavecount)
			    + beginSlaveInfoProgress);
			progressFunction(progress,
			    "Reading information for slave " + std::to_string(slave) + " / "
			        + std::to_string(ec_slavecount));

			auto esiBinary = readESIBinary(slave);
			accumulatedErrors.insert(
			    accumulatedErrors.end(), esiBinary.second.begin(), esiBinary.second.end());
			std::optional<datatypes::ESIData> esiData;
			if (containsOnlyLowSeverityErrors(esiBinary.second))
			{
				try
				{
					esiData = datatypes::esiparser::parseESI(esiBinary.first);
				}
				catch (datatypes::esiparser::ParseException& e)
				{
					accumulatedErrors.emplace_back("Could not parse this slave's ESI."
					                               " ESI will not be available for this slave,"
					                               " and PDOs may also be unavailable.",
					    slave, ErrorSeverity::MEDIUM);
				}
			}

			MayError<CoEResult> coeEntries;
			MayError<PDOResult> pdos;
			PDOList pdoList;

			std::string slaveName = getSlaveName(slave);

			// If the slave supports CANopen over EtherCAT
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
			if ((ec_slave[slave].mbx_proto & ECT_MBXPROT_COE) != 0)
			{
				coeEntries = readObjectDictionary(slave);
				if (containsOnlyLowSeverityErrors(coeEntries.second))
				{
					pdos = readPDOsViaCoE(slave, coeEntries.first.first);
				}
				accumulatedErrors.insert(
				    accumulatedErrors.end(), coeEntries.second.begin(), coeEntries.second.end());
				slaveName = getSlaveName(slave, coeEntries.first);
			}
			else if (esiData.has_value())
			{
				pdos = readPDOsViaESI(slave, esiData.value());
			}

			accumulatedErrors.insert(
			    accumulatedErrors.end(), pdos.second.begin(), pdos.second.end());
			pdoList = pdos.first.first;
			busInfo.pdoOffsets.insert(pdos.first.second.begin(), pdos.first.second.end());
			busInfo.coeInfos.insert(coeEntries.first.second.begin(), coeEntries.first.second.end());
			if (esiData.has_value())
			{
				slaveInfos.emplace_back(slave, std::move(slaveName), std::move(pdoList),
				    std::move(coeEntries.first.first), std::move(esiData.value()),
				    std::move(esiBinary.first), std::array<unsigned int, 4>(topology[slave]));
			}
			else
			{
				slaveInfos.emplace_back(slave, std::move(slaveName), std::move(pdoList),
				    std::move(coeEntries.first.first),
				    std::array<unsigned int, 4>(topology[slave]));
			}
		}

		addSOEMErrorsToVector(accumulatedErrors, ErrorSeverity::LOW);
		return accumulatedErrors;
	}

	/*!
	 * \brief Try to set all slaves into the Operational state.
	 *
	 * Will return at least one FATAL error iff the attempt failed.
	 * \return errors that occured while setting slave states
	 */
	// Making this method static is possible, but semantically a little strange.
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	std::vector<ErrorMessage> BusSlaveInformant::setSlavesIntoOP(
	    std::function<void(int, std::string)>& progressFunction)
	{
		progressFunction(opProgress, "Setting slaves into OP...");
		// Request OP state for all slaves
		ec_slave[0].state = EC_STATE_OPERATIONAL;
		// SOEM claims this makes some slaves happy
		ec_send_processdata();
		ec_receive_processdata(EC_TIMEOUTRET);

		ec_writestate(0);

		// Wait for all slaves to reach OP. This is taken from SOEM's "simpletest".
		static const int maxWaitCycles = 40;
		static const int stateCheckTimeout = 50000;
		for (int waitCycles = 0;
		     waitCycles < maxWaitCycles && ec_slave[0].state != EC_STATE_OPERATIONAL; ++waitCycles)
		{
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			ec_statecheck(0, EC_STATE_OPERATIONAL, stateCheckTimeout);
		}

		std::vector<ErrorMessage> errors;
		if (ec_slave[0].state != EC_STATE_OPERATIONAL)
		{
			errors.emplace_back("Could not get all slaves into OP.", ErrorSeverity::MEDIUM);
			addSOEMErrorsToVector(errors, ErrorSeverity::MEDIUM);
			return errors;
		}

		// At this point, we should have all slaves in OP.
		addSOEMErrorsToVector(errors, ErrorSeverity::LOW);
		return errors;
	}

	BusSlaveInformant::BusSlaveInformant(std::string interface)
	    : BusSlaveInformant(interface, [](int, std::string) {}) // NOLINT
	{
	}

	BusSlaveInformant::BusSlaveInformant(
	    std::string interface, std::function<void(int, std::string)> progressFunction)
	{
		performSlaveInformantInit(
		    [&interface]() { return ec_init(interface.c_str()); }, progressFunction);
	}

	BusSlaveInformant::BusSlaveInformant(int socket)
	    : BusSlaveInformant(socket, [](int, std::string) {}) // NOLINT
	{
	}

	BusSlaveInformant::BusSlaveInformant(
	    int socket, std::function<void(int, std::string)> progressFunction)
	{
		performSlaveInformantInit([socket]() { return ec_init_wsock(socket); }, progressFunction);
	}

	/*!
	 * \brief Initializes the EtherCAT bus and the SlaveInfos with the correct information.
	 * \param initFunc the function to initialize the bus with
	 * \exception BusSlaveInformantError if a fatal error occured during initialization
	 */
	void BusSlaveInformant::performSlaveInformantInit(const std::function<int(void)>& initFunc,
	    std::function<void(int, std::string)>& progressFunction)
	{
		std::vector<ErrorMessage> accumulatedErrors;
		auto errors = initializeBus(initFunc, progressFunction);
		accumulatedErrors.insert(accumulatedErrors.end(), errors.begin(), errors.end());
		if (containsFatalSeverityError(errors))
		{
			throw SlaveInformantError("Failed to initialize bus.", std::move(accumulatedErrors));
		}

		errors = readSlaveInfosFromBus(progressFunction);
		accumulatedErrors.insert(accumulatedErrors.end(), errors.begin(), errors.end());

		errors = setSlavesIntoOP(progressFunction);
		accumulatedErrors.insert(accumulatedErrors.end(), errors.begin(), errors.end());
		if (containsOnlyLowSeverityErrors(errors))
		{
			busInfo.statusAfterInit = datatypes::BusStatus::OP;
		}
		else
		{
			busInfo.statusAfterInit = datatypes::BusStatus::SAFE_OP;
		}

		progressFunction(finishedProgress, "Initialization finished.");
		initializationErrors = accumulatedErrors;
	}

	unsigned int BusSlaveInformant::getSlaveCount() const { return slaveInfos.size(); }

	const datatypes::SlaveInfo& BusSlaveInformant::getSlaveInfo(unsigned int index) const
	{
		if (index < 1 || index > slaveInfos.size())
		{
			throw std::runtime_error("Illegal slaveInfo " + std::to_string(index) + " requested.");
		}
		return slaveInfos.at(index - 1);
	}

	BusInfo& BusSlaveInformant::getBusInfo() { return busInfo; }

	uint64_t BusSlaveInformant::getIOMapSize() const { return busInfo.ioMapUsedSize; }

	const std::vector<ErrorMessage> BusSlaveInformant::getInitializationErrors()
	{
		const std::vector<datatypes::ErrorMessage> errorsTemp = initializationErrors;
		initializationErrors = std::vector<datatypes::ErrorMessage>();
		return errorsTemp;
	}

} // namespace etherkitten::reader
