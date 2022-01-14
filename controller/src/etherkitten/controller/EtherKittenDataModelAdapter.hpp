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

#include "visitors.hpp"
#include <etherkitten/gui/DataModelAdapter.hpp>
#include <etherkitten/reader/EtherKitten.hpp>

namespace etherkitten::controller
{

	/*!
	 * \brief Implements DataModelAdapter and forwards all method calls to EtherKitten.
	 */
	class EtherKittenDataModelAdapter : public gui::DataModelAdapter
	{
	public:
		/*!
		 * \brief Creates an EtherKittenDataModelAdapter that forwards to the given EtherKitten
		 * instance.
		 * \param etherKitten the EtherKitten instance to forward to
		 */
		EtherKittenDataModelAdapter(reader::EtherKitten& etherKitten);
		/*
		 * \copydoc gui::DataModelAdapter::getNewestValueView()
		 * \exception std::runtime_error if no AbstractNewestValueView could be acquired
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewestValueView(
		    const datatypes::DataObject& data) override;
		/*
		 * \copydoc gui::DataModelAdapter::getDataView()
		 * \exception std::runtime_error if no AbstractDataView could be acquired
		 */
		std::shared_ptr<datatypes::AbstractDataView> getDataView(
		    const datatypes::DataObject& data, datatypes::TimeSeries timeSeries) override;
		/*!
		 * \copydoc gui::DataModelAdapter::writeData()
		 * \exception ParseException if the given value is not parseable to the necessary type
		 */
		void writeData(const datatypes::DataObject& data, std::string value) override;
		//! \copydoc gui::DataModelAdapter::readCoEObject()
		void readCoEObject(const datatypes::CoEObject& obj) override;
		//! \copydoc gui::DataModelAdapter::resetAllErrorRegisters()
		void resetAllErrorRegisters() override;
		//! \copydoc gui::DataModelAdapter::resetErrorRegisters()
		void resetErrorRegisters(unsigned int slave) override;

	private:
		/*!
		 * \brief the EtherKitten instance to forward to
		 */
		reader::EtherKitten& etherKitten;
	};
} // namespace etherkitten::controller
