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

#include "EtherKittenBusInfoSupplier.hpp"

#include <functional>
#include <memory>

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/reader/EtherKitten.hpp>

namespace etherkitten::controller
{
	EtherKittenBusInfoSupplier::EtherKittenBusInfoSupplier(
	    reader::EtherKitten& etherKitten, config::ConfigIO& config, InterfaceInfo& interfaceInfo)
	    : etherKitten(etherKitten)
	    , config(config)
	    , interfaceInfo(interfaceInfo)
	{
	}

	EtherKittenBusInfoSupplier::~EtherKittenBusInfoSupplier() {}

	int EtherKittenBusInfoSupplier::getPDOFramerate()
	{
		return static_cast<int>(etherKitten.getPDOFrequency());
	}

	int EtherKittenBusInfoSupplier::getRegisterFramerate()
	{
		return static_cast<int>(etherKitten.getRegisterFrequency());
	}

	std::shared_ptr<datatypes::ErrorIterator> EtherKittenBusInfoSupplier::getErrorLog()
	{
		return etherKitten.getErrors();
	}

	std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
	EtherKittenBusInfoSupplier::getErrorStatistics()
	{
		std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>> result{};
		// Global statistics
		for (datatypes::ErrorStatisticInfo error : datatypes::errorStatisticInfos)
		{
			result.emplace_back(etherKitten.getErrorStatistic(
			    error.getType(datatypes::ErrorStatisticCategory::TOTAL_GLOBAL),
			    std::numeric_limits<unsigned int>::max()));
			result.emplace_back(etherKitten.getErrorStatistic(
			    error.getType(datatypes::ErrorStatisticCategory::FREQ_GLOBAL),
			    std::numeric_limits<unsigned int>::max()));
		}
		// Slave-specific statistics
		for (unsigned int slaveId = 1; slaveId <= etherKitten.getSlaveCount(); slaveId++)
		{
			const std::vector<datatypes::ErrorStatistic>& slaveStats
			    = etherKitten.getSlaveInfo(slaveId).getErrorStatistics();
			for (const datatypes::ErrorStatistic& errorStat : slaveStats)
			{
				result.emplace_back(errorStat);
			}
		}
		return result;
	}

	unsigned int EtherKittenBusInfoSupplier::getSlaveCount() { return etherKitten.getSlaveCount(); }

	const datatypes::SlaveInfo& EtherKittenBusInfoSupplier::getSlaveInfo(unsigned int slaveID)
	{
		return etherKitten.getSlaveInfo(slaveID);
	}

	datatypes::BusMode EtherKittenBusInfoSupplier::getBusMode() { return etherKitten.getBusMode(); }

	datatypes::TimeStamp EtherKittenBusInfoSupplier::getStartTime() const
	{
		return etherKitten.getStartTime();
	}

	std::vector<std::string> EtherKittenBusInfoSupplier::getProfileNames()
	{
		return config.getConfiguredBusses();
	}

	std::vector<std::string> EtherKittenBusInfoSupplier::getInterfaceNames()
	{
		return interfaceInfo.getInterfaceNames();
	}

} // namespace etherkitten::controller
