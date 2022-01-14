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

#include <functional>
#include <memory>
#include <vector>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::gui
{
	/*!
	 * \brief Provides an interface for the GUI to acquire information about the
	 * bus.
	 */
	class BusInfoSupplier
	{
	public:
		/*!
		 * \brief Default destructor - does nothing.
		 */
		virtual ~BusInfoSupplier() {}

		/*!
		 * \brief Return the rate at which PDOs are updated.
		 * \return the rate at which PDOs are updated
		 */
		virtual int getPDOFramerate() = 0;

		/*!
		 * \brief Return the rate at which registers are updated.
		 * \return the rate at which registers are updated.
		 */
		virtual int getRegisterFramerate() = 0;

		/*!
		 * \brief Return the error messages for the currently attached bus.
		 * \return the error messages for the currently attached bus
		 */
		virtual std::shared_ptr<datatypes::ErrorIterator> getErrorLog() = 0;

		/*!
		 * \brief Return the error statistics for the currently attached bus.
		 * \return the error statistics for the currently attached bus
		 */
		virtual std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
		getErrorStatistics() = 0;

		/*!
		 * \brief Return the number of slaves for the currently attached bus.
		 * \return the number of slaves for the currently attached bus.
		 */
		virtual unsigned int getSlaveCount() = 0;

		/*!
		 * \brief Return the SlaveInfo with the given ID.
		 * \param slaveID The ID of the SlaveInfo to return.
		 * \return the SlaveInfo with the given ID.
		 */
		virtual const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveID) = 0;

		/*!
		 * \brief Return the mode of the currently attached bus.
		 * \return the mode of the currently attached bus
		 */
		virtual datatypes::BusMode getBusMode() = 0;

		/*!
		 * \brief Get a TimeStamp that is earlier than all DataPoints offered by this Reader.
		 *
		 * This TimeStamp can be used as the base time to calculate time offsets
		 * of DataPoints.
		 * \return a TimeStamp that is earlier than all DataPoints
		 */
		virtual datatypes::TimeStamp getStartTime() const = 0;

		/*!
		 * \brief Return a list of currently available profile names.
		 * \return a list of currently available profile names.
		 */
		virtual std::vector<std::string> getProfileNames() = 0;

		/*!
		 * \brief Return a list of currently available network interfaces.
		 * \return a list of currently available network interfaces.
		 */
		virtual std::vector<std::string> getInterfaceNames() = 0;
	};
} // namespace etherkitten::gui
