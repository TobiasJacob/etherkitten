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
 * \brief Defines the CoEUpdateRequest, which can be used to communicate requests for both
 * reads from and writes to CoEObjects.
 */

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief Represents a request to update the value of a CoE Object.
	 *
	 * The update can take place either by reading the value into this request object or by
	 * writing the value from this request to the recipient.
	 */
	class CoEUpdateRequest
	{
	public:
		/*!
		 * \brief Unused, but required for use with certain data structures.
		 */
		CoEUpdateRequest() = default;

		/*!
		 * \brief Create a CoEUpdateRequest for updating the data of the specified CoEObject.
		 * \param coeObject the CoEObject to update the value of
		 * \param value an AbstractDataPoint to either read the value from or write the value to,
		 * depending on the type of the request
		 * \param readRequest true iff this is a request to read the value into the
		 * AbstractDataPoint, false iff this is a request to write the value from the
		 * AbstractDataPoint to the recipient
		 */
		CoEUpdateRequest(const datatypes::CoEObject& coeObject,
		    std::shared_ptr<datatypes::AbstractDataPoint> value, bool readRequest);

		/*!
		 * \brief Get the CoEObject this request is associated with.
		 * \return the associated CoEObject
		 */
		const datatypes::CoEObject* getObject() const;

		/*!
		 * \brief Get the AbstractDataPoint this request is associated with.
		 *
		 * If this is a read request and:
		 *  - `isProcessed() == true && hasFailed() == false`: the AbstractDataPoint will
		 *  contain the read value.
		 *  - `isProcessed() == true && hasFailed() == true`: the AbstractDataPoint will
		 *  be unchanged.
		 *  - `isProcessed() == false`: the AbstractDataPoint will be unchanged.
		 *
		 * If this is a write request, the AbstractDataPoint will never be changed.
		 * \return the AbstractDataPoint this request is associated with
		 */
		std::shared_ptr<datatypes::AbstractDataPoint> getValue() const;

		/*!
		 * \brief Check whether this request has been processed.
		 * \retval true iff this request has been processed
		 * \retval false iff this request is yet to be processed
		 */
		bool isProcessed() const;

		/*!
		 * \brief Signal that this request has been processed.
		 *
		 * To be used by the recipient of the CoEUpdateRequest.
		 */
		void setProcessed();

		/*!
		 * \brief Check whether this request has been processed.
		 * \retval true iff this request has been processed
		 * \retval false iff this request is yet to be processed
		 */
		bool hasFailed() const;

		/*!
		 * \brief Signal that this request has failed.
		 *
		 * To be used by the recipient of the CoEUpdateRequest.
		 */
		void setFailed();

		/*!
		 * \brief Check whether this is a read request.
		 * \retval true iff this is a read request
		 * \retval false iff this is a write request
		 */
		bool isReadRequest() const;

	private:
		const datatypes::CoEObject* coeObject = nullptr;
		std::shared_ptr<datatypes::AbstractDataPoint> value = nullptr;
		bool requestProcessed = false;
		bool requestFailed = false;
		bool readRequest = false;
	};
} // namespace etherkitten::reader
