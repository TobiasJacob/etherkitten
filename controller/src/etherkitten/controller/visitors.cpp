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

#include "visitors.hpp"
#include "etherkitten/datatypes/datapoints.hpp"
#include "etherkitten/datatypes/ethercatdatatypes.hpp"
#include <etherkitten/datatypes/EtherCATTypeParser.hpp>

namespace etherkitten::controller
{
	// NewestValueViewVisitor

	NewestValueViewVisitor::NewestValueViewVisitor(reader::EtherKitten& etherKitten)
	    : etherKitten(etherKitten)
	    , newestValueView(nullptr)
	{
	}
	NewestValueViewVisitor::~NewestValueViewVisitor() {}
	void NewestValueViewVisitor::handleCoE(const datatypes::CoEObject& object)
	{
		newestValueView = etherKitten.getNewest(object);
	}
	void NewestValueViewVisitor::handlePDO(const datatypes::PDO& object)
	{
		newestValueView = etherKitten.getNewest(object);
	}
	void NewestValueViewVisitor::handleErrorStatistic(const datatypes::ErrorStatistic& statistic)
	{
		newestValueView = etherKitten.getNewest(statistic);
	}
	void NewestValueViewVisitor::handleRegister(const datatypes::Register& reg)
	{
		newestValueView = etherKitten.getNewest(reg);
	}
	bool NewestValueViewVisitor::hasNewestValueView() { return newestValueView != nullptr; }
	std::unique_ptr<datatypes::AbstractNewestValueView> NewestValueViewVisitor::getNewestValueView()
	{
		if (!hasNewestValueView())
			throw std::runtime_error("no NewestValueView available");
		return std::move(newestValueView);
	}

	// DataViewVisitor

	DataViewVisitor::DataViewVisitor(
	    reader::EtherKitten& etherKitten, datatypes::TimeSeries timeSeries)
	    : etherKitten(etherKitten)
	    , timeSeries(timeSeries)
	    , dataView(nullptr)
	{
	}
	DataViewVisitor::~DataViewVisitor() {}
	void DataViewVisitor::handlePDO(const datatypes::PDO& object)
	{
		dataView = etherKitten.getView(object, timeSeries);
	}
	void DataViewVisitor::handleCoE(const datatypes::CoEObject& object)
	{
		(void)object;
		dataView = nullptr;
	}
	void DataViewVisitor::handleErrorStatistic(const datatypes::ErrorStatistic& statistic)
	{
		dataView = etherKitten.getView(statistic, timeSeries);
	}
	void DataViewVisitor::handleRegister(const datatypes::Register& reg)
	{
		dataView = etherKitten.getView(reg, timeSeries);
	}
	bool DataViewVisitor::hasDataView() { return dataView != nullptr; }
	std::shared_ptr<datatypes::AbstractDataView> DataViewVisitor::getDataView()
	{
		if (!hasDataView())
			throw std::runtime_error("no DataView available");
		return dataView;
	}

	// WriteVisitor

	WriteVisitor::WriteVisitor(reader::EtherKitten& etherKitten, std::string value)
	    : etherKitten(etherKitten)
	    , value(value)
	{
	}
	WriteVisitor::~WriteVisitor() {}

	void WriteVisitor::handleCoE(const datatypes::CoEObject& object)
	{
		convertValueFor(object);
		etherKitten.writeCoEObject(object, std::move(dataPoint));
	}
	void WriteVisitor::handlePDO(const datatypes::PDO& object)
	{
		convertValueFor(object);
		etherKitten.setPDOValue(object, std::move(dataPoint));
	}
	void WriteVisitor::handleErrorStatistic(const datatypes::ErrorStatistic& stat) { (void)stat; }
	void WriteVisitor::handleRegister(const datatypes::Register& reg) { (void)reg; }

	/*!
	 * \brief Inserts a generic value into an AbstractDataPoint using the DataPointInserter.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type of the value to insert
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class GenericDataPointInserter
	{
	public:
		/**
		 * \brief A function that takes a unique_ptr to an AbstractDataPoint and a string reference
		 */
		using product_t = std::function<void(
		    std::unique_ptr<datatypes::AbstractDataPoint>&, const std::string&)>;

		/**
		 * \brief Create a function to insert a generic value into an AbstractDataPoint
		 * \return the created function
		 */
		static product_t eval()
		{
			return
			    [](std::unique_ptr<datatypes::AbstractDataPoint>& point, const std::string& value) {
				    WriteVisitor::DataPointInserter<typename datatypes::TypeMap<E>::type>::insert(
				        point, value);
			    };
		}
	};

	void WriteVisitor::convertValueFor(const datatypes::DataObject& object)
	{
		using namespace etherkitten::datatypes;
		datatypes::dataTypeMapWithStrings<GenericDataPointInserter>.at(object.getType())(
		    dataPoint, value);
	}
} // namespace etherkitten::controller
