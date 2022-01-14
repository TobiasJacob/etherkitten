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
 * \brief Defines CoEUpdate, a pair-like datastructure that combines a coe object
 * and AbstractDataPoint.
 */

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>

#include "Serialized.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Holds CoEObject with corresponding AbstractDataPoint.
	 */
	class CoEUpdate
	{
	public:
		/*!
		 * \brief Create new CoEUpdate
		 * \param obj the CoEObject
		 * \param dataPoint the AbstractDataPoint
		 */
		CoEUpdate(const datatypes::CoEObject& obj,
		    std::shared_ptr<datatypes::AbstractDataPoint>&& dataPoint);

		/*!
		 * \brief Serialize the data for the log
		 * \return the buffer containing the data for the log
		 */
		Serialized get();

	private:
		const datatypes::CoEObject& obj;
		std::shared_ptr<datatypes::AbstractDataPoint> dataPoint;
	};
} // namespace etherkitten::reader
