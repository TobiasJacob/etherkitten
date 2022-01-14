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
 * \brief Defines the QueueCacheProxy, a proxy that caches results from a MessageQueues instance
 * and otherwise offers the same interface.
 */

#include <memory>
#include <unordered_map>
#include <utility>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

#include "CoENewestValueView.hpp"
#include "MessageQueues.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The QueueCacheProxy class works as proxy between EtherKitten and the MessageQueues.
	 *
	 * The most recently read and written CoEObject values are stored here. They are accessible via
	 * the AbstractNewestValueViews that this class provides.
	 */
	class QueueCacheProxy
	{
	public:
		/*!
		 * \brief Construct a QueueCacheProxy that caches results from the given MessageQueues.
		 * \param queues the MessageQueues to cache the results of
		 */
		QueueCacheProxy(std::unique_ptr<MessageQueues>&& queues);

		/*!
		 * \brief Get an AbstractNewestValueView over the most recent values of the given CoEObject.
		 * \param object the CoEObject for which the view is requested
		 * \return a view over the most recent value of the CoEObject
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::CoEObject& object);

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
		bool updateCoEObject(const datatypes::CoEObject& object,
		    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest);

		/*!
		 * \brief Post a request to write the given value for the given PDO.
		 * \param pdo the PDO to change the value of
		 * \param value the value to write
		 */
		void setPDOValue(
		    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint> value);

		/*!
		 * \brief Return all the errors that occurred on the opposite side of the queues.
		 * \return all the errors that occurred in the other thread
		 */
		std::shared_ptr<datatypes::ErrorIterator> getErrors();

		/*!
		 * \brief Post a request to reset all the error-counting registers of the slave with the
		 * given index.
		 * \param slave the index of the slave whose registers will be reset
		 */
		void resetErrorRegisters(unsigned int slave);

	private:
		std::unique_ptr<MessageQueues> queues;
		std::unordered_map<datatypes::CoEObject,
		    std::shared_ptr<std::shared_ptr<datatypes::AbstractDataPoint>>,
		    datatypes::CoEObjectHash, datatypes::CoEObjectEqual>
		    coeData;
	};
} // namespace etherkitten::reader
