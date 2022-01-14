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

#include "QueueCacheProxy.hpp"

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

#include "ReaderErrorIterator.hpp"
#include "queues-common.hpp"

namespace etherkitten::reader
{
	QueueCacheProxy::QueueCacheProxy(std::unique_ptr<MessageQueues>&& queues)
	    : queues(std::move(queues))
	{
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> QueueCacheProxy::getNewest(
	    const datatypes::CoEObject& object)
	{
		if (coeData.find(object) == coeData.end())
		{
			coeData.emplace(object,
			    std::make_shared<std::shared_ptr<datatypes::AbstractDataPoint>>(
			        std::shared_ptr<datatypes::AbstractDataPoint>(nullptr)));
		}
		return std::make_unique<CoENewestValueView>(CoENewestValueView(coeData[object]));
	}

	bool QueueCacheProxy::updateCoEObject(const datatypes::CoEObject& object,
	    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest)
	{
		bool newEntry = false;
		if (coeData.count(object) == 0)
		{
			newEntry = true;
			coeData[object] = std::make_shared<std::shared_ptr<datatypes::AbstractDataPoint>>(
			    datatypes::dataTypeMapWithStrings<AbstractDataPointCreator>.at(object.getType())());
		}
		// update the cached data if the read was successful
		if (queues->postCoEUpdateRequest(object, value, readRequest))
		{
			*coeData[object] = value;
			return true;
		}
		// delete entry in coeData if read fails and no valid CoE was read before
		if (newEntry)
		{
			coeData.erase(object);
		}
		return false;
	}

	void QueueCacheProxy::setPDOValue(
	    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint> value)
	{
		queues->postPDOWriteRequest(pdo, std::move(value));
	}

	std::shared_ptr<datatypes::ErrorIterator> QueueCacheProxy::getErrors()
	{
		return std::make_shared<ReaderErrorIterator>(ReaderErrorIterator{ queues->getErrors() });
	}

	void QueueCacheProxy::resetErrorRegisters(unsigned int slave)
	{
		queues->postErrorRegisterResetRequest(slave);
	}
} // namespace etherkitten::reader
