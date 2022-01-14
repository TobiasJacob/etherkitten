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

#include "etherkitten/gui/DataModelAdapter.hpp"
#include <QMainWindow>
#include <QTimer>
#include <etherkitten/config/BusLayout.hpp>
#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/gui/BusInfoSupplier.hpp>
#include <vector>

namespace etherkitten::gui_test
{

	class DummyErrorIterator : public datatypes::ErrorIterator
	{
		DummyErrorIterator& operator++() override;
		bool hasNext() const override;
		bool isEmpty() const override;
		datatypes::DataPoint<datatypes::ErrorMessage> operator*() const override;
	};

	class DummyBusInfoSupplier : public gui::BusInfoSupplier
	{
	public:
		DummyBusInfoSupplier();
		int getPDOFramerate() override;
		int getRegisterFramerate() override;
		std::shared_ptr<datatypes::ErrorIterator> getErrorLog() override;
		std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
		getErrorStatistics() override;
		unsigned int getSlaveCount() override;
		datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveID) override;
		datatypes::BusMode getBusMode() override;
		datatypes::TimeStamp getStartTime() const override;
		std::vector<std::string> getProfileNames() override;
		std::vector<std::string> getInterfaceNames() override;

	private:
		std::vector<datatypes::SlaveInfo> slaves;
	};

	class DummyDataView : public datatypes::AbstractDataView
	{
	public:
		double asDouble() const override;
		datatypes::AbstractDataView& operator++() override;
		bool hasNext() const override;
		bool isEmpty() const override;
		datatypes::TimeStamp getTime() override;
	};

	class DummyNewestValueView : public datatypes::AbstractNewestValueView
	{
	public:
		bool isEmpty() const override;
		const datatypes::AbstractDataPoint& operator*() override;
	};

	class DummyDataPoint : public datatypes::AbstractDataPoint
	{
	public:
		DummyDataPoint();
		std::string asString(datatypes::NumberFormat base) const override;
		std::unique_ptr<AbstractDataPoint> clone() const override;

	private:
		datatypes::EtherCATDataType::UNSIGNED32 value;
	};

	class DummyAdapter : public gui::DataModelAdapter
	{
	public:
		DummyAdapter();
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewestValueView(
		    const datatypes::DataObject& data) override;
		std::shared_ptr<datatypes::AbstractDataView> getDataView(
		    const datatypes::DataObject& data, datatypes::TimeSeries time) override;
		void writeData(const datatypes::DataObject& data, std::string value) override;
		void readCoEObject(const datatypes::CoEObject& obj) override;
		void resetAllErrorRegisters() override;
		void resetErrorRegisters(unsigned int slave) override;
	};

	class MainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		explicit MainWindow(QWidget* parent = nullptr);
		~MainWindow();

	private:
		DummyAdapter adapter;
		DummyBusInfoSupplier busInfo;
		QTimer* timer;
		QTimer* timer2;
		config::BusLayout layout;
	};

} // namespace etherkitten::gui_test
