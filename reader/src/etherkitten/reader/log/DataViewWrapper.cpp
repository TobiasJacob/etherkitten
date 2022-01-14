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

#include "DataViewWrapper.hpp"

#include "../DataView.hpp"
#include "Serialized.hpp"
#include "registerdata.hpp"
#include <any>
#include <etherkitten/datatypes/datapoints.hpp>

namespace etherkitten::reader
{
	RegisterDataViewWrapper::RegisterDataViewWrapper(
	    uint16_t regAddr, uint16_t slaveId, std::shared_ptr<datatypes::AbstractDataView> dataView)
	    : regAddr(regAddr)
	    , slaveId(slaveId)
	    , dataView(dataView)
	{
		firstIsValid = !dataView->isEmpty();
	}

	bool RegisterDataViewWrapper::hasNext() { return dataView->hasNext() || firstIsValid; }

	void RegisterDataViewWrapper::next()
	{
		if (firstIsValid)
			firstIsValid = false;
		else
			++(*dataView);
	}

	Serialized RegisterDataViewWrapper::get()
	{
		using namespace datatypes::EtherCATDataType;
		datatypes::TimeStamp timeStamp(dataView->getTime());
		auto ms = datatypes::timeStampToInt(timeStamp);

		uint64_t data;
		switch (datatypes::getRegisterByteLength(static_cast<datatypes::RegisterEnum>(regAddr)))
		{
		case 1:
			data = static_cast<uint64_t>(
			    *dynamic_cast<DataView<UNSIGNED8, Reader::nodeSize>&>(*dataView));
			break;
		case 2:
			data = static_cast<uint64_t>(
			    *dynamic_cast<DataView<UNSIGNED16, Reader::nodeSize>&>(*dataView));
			break;
		case 4:
			data = static_cast<uint64_t>(
			    *dynamic_cast<DataView<UNSIGNED32, Reader::nodeSize>&>(*dataView));
			break;
		case 8:
			data = static_cast<uint64_t>(
			    *dynamic_cast<DataView<UNSIGNED64, Reader::nodeSize>&>(*dataView));
			break;
		default:
			throw std::runtime_error("unexpected register length");
		}

		RegisterDataBlock block{ regAddr, slaveId, static_cast<uint64_t>(ms), data };
		return block.getSerializer().serialize(block);
	}

	bool RegisterDataViewWrapper::isEmpty() { return dataView->isEmpty(); }

	datatypes::TimeStamp RegisterDataViewWrapper::getTime() { return dataView->getTime(); }

	IOMapDataViewWrapper::IOMapDataViewWrapper(
	    std::shared_ptr<DataView<std::unique_ptr<IOMap>, Reader::nodeSize, IOMap*>> dataView)
	    : dataView(dataView)
	{
		firstIsValid = !dataView->isEmpty();
	}

	bool IOMapDataViewWrapper::hasNext() { return dataView->hasNext() || firstIsValid; }

	void IOMapDataViewWrapper::next()
	{
		if (firstIsValid)
			firstIsValid = false;
		else
			++(*dataView);
	}

	datatypes::TimeStamp IOMapDataViewWrapper::getTime() { return dataView->getTime(); }

	IOMap* IOMapDataViewWrapper::get() { return **dataView.get(); }
} // namespace etherkitten::reader
