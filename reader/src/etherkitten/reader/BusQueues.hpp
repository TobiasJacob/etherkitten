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
 * \brief Defines the BusQueues, which are used for communication between two threads.
 */

#include <condition_variable>
#include <memory>
#include <optional>

#include <boost/lockfree/policies.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/errors.hpp>

#include "CoEUpdateRequest.hpp"
#include "DataView.hpp"
#include "MessageQueues.hpp"
#include "PDOWriteRequest.hpp"
#include "SearchList.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Implements MessageQueues and offers additional methods to be used by the
	 * opposite end of the queues.
	 *
	 * This class must only be used by one thread on either side of each queue,
	 * as its methods are not fully reentrant.
	 * If multiple threads call the same methods at the same time, the resulting behavior
	 * is undefined.
	 */
	class BusQueues : public MessageQueues
	{
	public:
		bool postCoEUpdateRequest(const datatypes::CoEObject& object,
		    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest) override;

		void postPDOWriteRequest(const datatypes::PDO& pdo,
		    std::unique_ptr<datatypes::AbstractDataPoint> value) override;

		std::shared_ptr<DataView<datatypes::ErrorMessage>> getErrors() override;

		void postErrorRegisterResetRequest(unsigned int slave) override;

		/*!
		 * \brief Return the oldest CoEReadRequest in the queue or nullptr if there is none.
		 * \return the oldest CoEReadRequest or nullptr if there is none
		 */
		std::shared_ptr<CoEUpdateRequest> getCoERequest();

		/*!
		 * \brief Return the oldest PDOWriteRequest in the queue or nullptr if there is none.
		 * \return the oldest PDOWriteRequest or nullptr if there is none
		 */
		std::shared_ptr<PDOWriteRequest> getPDORequest();

		/*!
		 * \brief Add an error message to the error queue.
		 * \param error the error to be added to the queue
		 */
		void postError(datatypes::ErrorMessage&& error);

		/*!
		 * \brief Notify the thread waiting for this CoE update request that it has been processed.
		 *
		 * Note that passing this method a request that has not been obtained from `getCoERequest()`
		 * has no effect.
		 * \param request that has been processed
		 */
		void postCoERequestReply(std::shared_ptr<CoEUpdateRequest>&& request);

		/*!
		 * \brief Return the oldest register reset request in the queue or nothing if there is none
		 * \return the oldest register reset request or nothing if there is none
		 */
		std::optional<unsigned int> getRegisterResetRequest();

	private:
		static constexpr size_t queueSize = 1000;
		boost::lockfree::spsc_queue<std::shared_ptr<CoEUpdateRequest>,
		    boost::lockfree::capacity<queueSize>>
		    coeQueue;
		boost::lockfree::spsc_queue<std::shared_ptr<PDOWriteRequest>,
		    boost::lockfree::capacity<queueSize>>
		    pdoQueue;

		SearchList<datatypes::ErrorMessage> errorList;
		boost::lockfree::spsc_queue<unsigned int, boost::lockfree::capacity<queueSize>> resetQueue;
		std::condition_variable coeCV;
	};
} // namespace etherkitten::reader
