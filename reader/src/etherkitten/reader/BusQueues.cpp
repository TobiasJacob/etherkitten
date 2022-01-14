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

#include "BusQueues.hpp"

#include <functional>

#include "queues-common.hpp"

namespace etherkitten::reader
{
	bool BusQueues::postCoEUpdateRequest(const datatypes::CoEObject& object,
	    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest)
	{
		static std::mutex mtx;
		std::unique_lock<std::mutex> lck(mtx);

		std::shared_ptr<CoEUpdateRequest> request
		    = std::make_shared<CoEUpdateRequest>(object, value, readRequest);
		coeQueue.push(request);
		coeCV.wait(lck, [request]() { return request->isProcessed() || request->hasFailed(); });

		return !request->hasFailed();
	}

	void BusQueues::postPDOWriteRequest(
	    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint> value)
	{
		pdoQueue.push(std::make_shared<PDOWriteRequest>(pdo, std::move(value)));
	}

	std::shared_ptr<DataView<datatypes::ErrorMessage>> BusQueues::getErrors()
	{
		return errorList.getView(
		    datatypes::TimeSeries{ datatypes::TimeStamp(), datatypes::TimeStep(0) }, false);
	}

	void BusQueues::postErrorRegisterResetRequest(unsigned int slave) { resetQueue.push(slave); }

	std::shared_ptr<CoEUpdateRequest> BusQueues::getCoERequest()
	{
		std::shared_ptr<CoEUpdateRequest> request;
		if (!coeQueue.pop(request))
		{
			return nullptr;
		}
		return request;
	}

	std::shared_ptr<PDOWriteRequest> BusQueues::getPDORequest()
	{
		std::shared_ptr<PDOWriteRequest> request;
		if (!pdoQueue.pop(request))
		{
			return nullptr;
		}
		return request;
	}

	void BusQueues::postError(datatypes::ErrorMessage&& error)
	{
		datatypes::TimeStamp ts = datatypes::now();
		errorList.append(error, ts);
	}

	void BusQueues::postCoERequestReply(std::shared_ptr<CoEUpdateRequest>&& request)
	{
		request->setProcessed();
		coeCV.notify_all();
	}

	std::optional<unsigned int> BusQueues::getRegisterResetRequest()
	{
		static unsigned int slave;
		if (!resetQueue.pop(slave))
		{
			return {};
		}
		return slave;
	}

} // namespace etherkitten::reader
