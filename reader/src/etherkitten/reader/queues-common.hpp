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
 * \brief Defines methods used by the various implementations of the MessageQueues interface.
 */

#include <functional>
#include <memory>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief Create an empty AbstractDataPoint of any type.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type to create in the AbstractDataPoint
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class AbstractDataPointCreator
	{
	public:
		using product_t = std::function<std::shared_ptr<datatypes::AbstractDataPoint>()>;

		static product_t eval()
		{
			return []() {
				return std::make_shared<
				    datatypes::DataPoint<typename datatypes::TypeMap<E>::type>>();
			};
		}
	};
} // namespace etherkitten::reader
