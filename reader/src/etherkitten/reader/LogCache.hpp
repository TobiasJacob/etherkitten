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
#include <mutex>

#include "CoENewestValueView.hpp"
#include "ReaderErrorIterator.hpp"
#include "SearchList.hpp"
#include "queues-common.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The LogCache caches the read CoE-Objects and errors. They are accessible via
	 * AbstarctNewestValueViews which this class provides.
	 */
	class LogCache
	{
	public:
		/*!
		 * \brief returns a view to the last know value of a CoE-Object
		 * \param object is the CoE-Object for which the view is requested
		 * \return a view to the last know value of a CoE-Object
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::CoEObject& object);

		/*!
		 * \brief returns all errors of the realtime part of the BusReader
		 * \return all errors of the realtime part of the BusReader formatted in ErrorMessage
		 */
		std::shared_ptr<datatypes::ErrorIterator> getErrors();

		/*!
		 * \brief adds an error message to the LogCache
		 * \param error which is added
		 * \param time timestamp when the error occured
		 */
		void postError(
		    datatypes::ErrorMessage&& error, datatypes::TimeStamp time = datatypes::now());

		/*!
		 * \brief Set a new value for a coe object
		 * \param object the coe object which has a new value
		 * \param datapoint a datapoint containing the new value of the coe object
		 */
		void setCoEValue(const datatypes::CoEObject& object,
		    std::unique_ptr<datatypes::AbstractDataPoint> datapoint);

	private:
		SearchList<datatypes::ErrorMessage> errorList;
		std::unordered_map<datatypes::CoEObject,
		    std::shared_ptr<std::shared_ptr<datatypes::AbstractDataPoint>>,
		    datatypes::CoEObjectHash, datatypes::CoEObjectEqual>
		    coeData;
		std::mutex coeMutex;
	};
} // namespace etherkitten::reader
