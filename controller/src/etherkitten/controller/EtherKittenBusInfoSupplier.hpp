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

#include <memory>

#include <etherkitten/config/ConfigIO.hpp>
#include <etherkitten/datatypes/time.hpp>
#include <etherkitten/gui/BusInfoSupplier.hpp>
#include <etherkitten/reader/EtherKitten.hpp>

#include "InterfaceInfo.hpp"

namespace etherkitten::controller
{

	/*!
	 * \brief Implements BusInfoSupplier and forwards most method calls to EtherKitten.
	 * Some calls are forwarded to ConfigIO, too.
	 */
	class EtherKittenBusInfoSupplier : public gui::BusInfoSupplier
	{
	public:
		/*!
		 * \brief Creates an EtherKittenBusInfoSupplier that forwards to the given EtherKitten
		 * and ConfigIO instances.
		 * \param etherKitten the EtherKitten instance to forward to
		 * \param config the ConfigIO instance to forward to
		 * \param interfaceInfo information about where to access the bus
		 */
		EtherKittenBusInfoSupplier(reader::EtherKitten& etherKitten, config::ConfigIO& config,
		    InterfaceInfo& interfaceInfo);

		~EtherKittenBusInfoSupplier() override;
		//! \copydoc gui::BusInfoSupplier::getPDOFramerate()
		int getPDOFramerate() override;
		//! \copydoc gui::BusInfoSupplier::getRegisterFramerate()
		int getRegisterFramerate() override;
		//! \copydoc gui::BusInfoSupplier::getErrorLog()
		std::shared_ptr<datatypes::ErrorIterator> getErrorLog() override;
		//! \copydoc gui::BusInfoSupplier::getErrorStatistics()
		std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
		getErrorStatistics() override;
		//! \copydoc gui::BusInfoSupplier::getSlaveCount()
		unsigned int getSlaveCount() override;
		//! \copydoc gui::BusInfoSupplier::getSlaveInfo()
		const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveID) override;
		//! \copydoc gui::BusInfoSupplier::getBusMode()
		datatypes::BusMode getBusMode() override;
		//! \copydoc gui::BusInfoSupplier::getStartTime()
		datatypes::TimeStamp getStartTime() const override;
		//! \copydoc gui::BusInfoSupplier::getProfileNames()
		std::vector<std::string> getProfileNames() override;
		//! \copydoc gui::BusInfoSupplier::getInterfaceNames()
		std::vector<std::string> getInterfaceNames() override;

	private:
		/*!
		 * \brief the EtherKitten instance to forward to
		 */
		reader::EtherKitten& etherKitten;
		/*!
		 * \brief the ConfigIO instance to get information from
		 */
		config::ConfigIO& config;
		/*!
		 * \brief the network interface information to supply to the GUI
		 */
		InterfaceInfo interfaceInfo;
	};
} // namespace etherkitten::controller
