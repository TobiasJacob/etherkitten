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


#include "CoEUpdate.hpp"

#include <functional>

#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "Serializer.hpp"
#include "coedata.hpp"

namespace etherkitten::reader
{

	CoEUpdate::CoEUpdate(
	    const datatypes::CoEObject& obj, std::shared_ptr<datatypes::AbstractDataPoint>&& dataPoint)
	    : obj(obj)
	    , dataPoint(dataPoint)
	{
	}

	/*!
	 * \brief Converts an AbstractDataPoint to a std::any containing the value.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type contained in the AbstractDataPoint
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class AbstractToAnyPoint
	{
	public:
		using product_t = std::function<std::any(datatypes::AbstractDataPoint&)>;

		static product_t eval()
		{
			return [](datatypes::AbstractDataPoint& dataPoint) -> std::any {
				using Type = typename datatypes::TypeMap<E>::type;
				return std::make_any<Type>(
				    dynamic_cast<datatypes::DataPoint<Type>&>(dataPoint).getValue());
			};
		}
	};

	Serialized CoEUpdate::get()
	{
		datatypes::TimeStamp timeStamp(dataPoint->getTime());
		auto ms = datatypes::timeStampToInt(timeStamp);
		std::any any
		    = datatypes::dataTypeMapWithStrings<AbstractToAnyPoint>.at(obj.getType())(*dataPoint);
		CoEDataBlock block{ static_cast<uint16_t>(obj.getSlaveID()), static_cast<uint64_t>(ms),
			static_cast<uint16_t>(obj.getIndex()), static_cast<uint8_t>(obj.getSubIndex()),
			std::move(any), obj.getType() };
		return block.getSerializer().serialize(block);
	}
} // namespace etherkitten::reader
