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
 * \brief Defines the interface for SlaveInformants, classes that offer information on
 * EtherCAT slaves.
 */

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/errors.hpp>

namespace etherkitten::reader
{

	/*!
	 * \brief The SlaveInformantError class is an exception which carries error messages
	 * that occured during SlaveInformant initialization.
	 */
	class SlaveInformantError : public std::runtime_error
	{
	public:
		/*!
		 * \brief Construct a new SlaveInformantError with the given information.
		 * \param whatArg the message to store in this error
		 * \param errors the additional error messages to store in this error
		 */
		SlaveInformantError(const char* whatArg, std::vector<datatypes::ErrorMessage>&& errors);

		/*!
		 * \brief Get the additional error messages stored in this error.
		 * \return the additional error messages
		 */
		const std::vector<datatypes::ErrorMessage>& getErrorList() const;

	private:
		std::vector<datatypes::ErrorMessage> errors;
	};

	/*!
	 * \brief The SlaveInformant class defines the interface for a class offering information
	 * on EtherCAT slaves on an EtherCAT bus.
	 *
	 * When an implementing class is destroyed, all references to SlaveInfo objects acquired
	 * via that class become invalid.
	 */
	class SlaveInformant
	{
	public:
		/*!
		 * \brief Get the number of slaves on an EtherCAT bus.
		 * \return the number of slaves
		 */
		virtual unsigned int getSlaveCount() const = 0;

		/*!
		 * \brief Get the SlaveInfo object for the slave at the corresponding location on the bus.
		 *
		 * slaveIndex must be in the range `[1, getSlaveCount()]`.
		 * \param slaveIndex the index of the slave to get
		 * \return the SlaveInfo object for the slave
		 * \exception std::runtime_error if the slaveIndex is not valid
		 */
		virtual const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveIndex) const = 0;

		/*!
		 * \brief Get the size of the IOMap
		 * \return the size of the IOMap
		 */
		virtual uint64_t getIOMapSize() const = 0;

		/*!
		 * \brief Get all error messages that accumulated during the initialization process. All
		 * errors are cleared once this method is called.
		 *
		 * Errors with a LOW severity only affect singular DataObjects, which may be
		 * unavailable. Errors with a MEDIUM severity affect larger parts of the SlaveInfo for
		 * one slave - for example, one SlaveInfo might be lacking PDOs. Errors with a FATAL
		 * severity will not appear in this vector, as these would have caused an exception to
		 * be thrown in this BusSlaveInformant's constructor. \return all the accumulated error
		 * messages.
		 */
		virtual const std::vector<datatypes::ErrorMessage> getInitializationErrors() = 0;
	};
} // namespace etherkitten::reader
