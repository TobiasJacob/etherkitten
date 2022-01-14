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

#include "CoEUpdateRequest.hpp"

namespace etherkitten::reader
{
	CoEUpdateRequest::CoEUpdateRequest(const datatypes::CoEObject& coeObject,
	    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest)
	    : coeObject(&coeObject)
	    , value(std::move(value))
	    , readRequest(readRequest)
	{
	}

	const datatypes::CoEObject* CoEUpdateRequest::getObject() const { return coeObject; }

	std::shared_ptr<datatypes::AbstractDataPoint> CoEUpdateRequest::getValue() const
	{
		return value;
	}

	bool CoEUpdateRequest::isProcessed() const { return requestProcessed; }

	void CoEUpdateRequest::setProcessed() { requestProcessed = true; }

	bool CoEUpdateRequest::hasFailed() const { return requestFailed; }

	void CoEUpdateRequest::setFailed() { requestFailed = true; }

	bool CoEUpdateRequest::isReadRequest() const { return readRequest; }
} // namespace etherkitten::reader
