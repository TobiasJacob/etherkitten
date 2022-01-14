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

#include "EtherKittenDataModelAdapter.hpp"

namespace etherkitten::controller
{

	EtherKittenDataModelAdapter::EtherKittenDataModelAdapter(reader::EtherKitten& etherKitten)
	    : etherKitten(etherKitten)
	{
	}

	std::unique_ptr<datatypes::AbstractNewestValueView>
	EtherKittenDataModelAdapter::getNewestValueView(const datatypes::DataObject& data)
	{
		NewestValueViewVisitor visitor{ etherKitten };
		data.acceptVisitor(visitor);
		return visitor.getNewestValueView();
	}
	std::shared_ptr<datatypes::AbstractDataView> EtherKittenDataModelAdapter::getDataView(
	    const datatypes::DataObject& data, datatypes::TimeSeries timeSeries)
	{
		DataViewVisitor visitor{ etherKitten, timeSeries };
		data.acceptVisitor(visitor);
		return visitor.getDataView();
	}
	void EtherKittenDataModelAdapter::writeData(
	    const datatypes::DataObject& data, std::string value)
	{
		WriteVisitor visitor{ etherKitten, value };
		data.acceptVisitor(visitor);
	}
	void EtherKittenDataModelAdapter::readCoEObject(const datatypes::CoEObject& obj)
	{
		// Does not need to be returned, will be read via a NewestValueView.
		etherKitten.readCoEObject(obj);
	}
	void EtherKittenDataModelAdapter::resetAllErrorRegisters()
	{
		unsigned int slaveCount = etherKitten.getSlaveCount();
		for (unsigned int slaveIndex = 1; slaveIndex <= slaveCount; slaveIndex++)
		{
			etherKitten.resetErrorRegisters(slaveIndex);
		}
	}
	void EtherKittenDataModelAdapter::resetErrorRegisters(unsigned int slave)
	{
		etherKitten.resetErrorRegisters(slave);
	}
} // namespace etherkitten::controller
