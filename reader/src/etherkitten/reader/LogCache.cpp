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

#include "LogCache.hpp"

namespace etherkitten::reader
{
	std::unique_ptr<datatypes::AbstractNewestValueView> LogCache::getNewest(
	    const datatypes::CoEObject& object)
	{
		std::lock_guard<std::mutex> lg(coeMutex);
		if (coeData.find(object) == coeData.end())
		{
			coeData.emplace(object,
			    std::make_shared<std::shared_ptr<datatypes::AbstractDataPoint>>(
			        std::shared_ptr<datatypes::AbstractDataPoint>(nullptr)));
		}
		return std::make_unique<CoENewestValueView>(CoENewestValueView(coeData[object]));
	}

	std::shared_ptr<datatypes::ErrorIterator> LogCache::getErrors()
	{
		return std::make_shared<ReaderErrorIterator>(ReaderErrorIterator{ errorList.getView(
		    datatypes::TimeSeries{ datatypes::intToTimeStamp(0), datatypes::TimeStep(0) },
		    false) });
	}

	void LogCache::postError(datatypes::ErrorMessage&& error, datatypes::TimeStamp time)
	{
		errorList.append(error, time);
	}

	void LogCache::setCoEValue(
	    const datatypes::CoEObject& object, std::unique_ptr<datatypes::AbstractDataPoint> datapoint)
	{
		std::lock_guard<std::mutex> lg(coeMutex);
		if (coeData.count(object) == 0)
		{
			coeData[object] = std::make_shared<std::shared_ptr<datatypes::AbstractDataPoint>>(
			    datatypes::dataTypeMapWithStrings<AbstractDataPointCreator>.at(object.getType())());
		}
		coeData[object]->reset(datapoint.release());
	}

} // namespace etherkitten::reader
