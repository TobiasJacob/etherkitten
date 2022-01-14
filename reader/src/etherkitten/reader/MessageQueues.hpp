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
 * \brief Defines the MessageQueues, an interface for passing messages and requests between two
 * threads.
 */

#include <memory>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

#include "DataView.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Defines a message queue-based interface between two threads.
	 *
	 * Every individual queue defined by this class is unidirectional, but there are
	 * queues in both directions available.
	 */
	class MessageQueues // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		virtual ~MessageQueues() = default;

		/*!
		 * \brief Post an update request for a CoEObject.
		 *
		 * Both read and write requests are handled by this method.
		 * For read requests, the given value must be a valid value for the type
		 * of the CoEObject. For write requests, it must only be able to contain such a value.
		 * This call blocks until the request has been processed.
		 *
		 * \param object the CoEObject to request the update for
		 * \param value the value to write to or read from
		 * \param readRequest true iff this is a read request
		 * \retval true iff the request was processed successfully
		 * \retval false iff the request was not processed successfully
		 */
		virtual bool postCoEUpdateRequest(const datatypes::CoEObject& object,
		    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest)
		    = 0;

		/*!
		 * \brief Post a request to write the given value for the given PDO.
		 * \param pdo the PDO to change the value of
		 * \param value the value to write
		 */
		virtual void postPDOWriteRequest(
		    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint> value)
		    = 0;

		/*!
		 * \brief Return all the errors that occurred on the opposite side of the queues.
		 * \return all the errors that occurred in the other thread
		 */
		virtual std::shared_ptr<DataView<datatypes::ErrorMessage>> getErrors() = 0;

		/*!
		 * \brief Post a request to reset all the error-counting registers of the slave with the
		 * given index.
		 * \param slave the index of the slave whose registers will be reset
		 */
		virtual void postErrorRegisterResetRequest(unsigned int slave) = 0;
	};
} // namespace etherkitten::reader
