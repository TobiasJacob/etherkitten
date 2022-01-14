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

#include <QMainWindow>
#include <QTimer>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/gui/DataModelAdapter.hpp>
#include <etherkitten/gui/Plot.hpp>
#include <etherkitten/gui/PlotList.hpp>
#include <random>
#include <vector>

namespace etherkitten::gui_test
{

	class DummyDataView : public datatypes::AbstractDataView
	{
	public:
		DummyDataView(
		    std::vector<double>& data, datatypes::TimeSeries series, datatypes::TimeStamp start);
		double asDouble() const override;
		datatypes::AbstractDataView& operator++() override;
		bool hasNext() const override;
		bool isEmpty() const override;
		datatypes::TimeStamp getTime() override;

	private:
		std::vector<double>& data;
		datatypes::TimeSeries series;
		datatypes::TimeStamp start;
		long pos;
		long step;
	};

	class DummyNewestValueView : public datatypes::AbstractNewestValueView
	{
	public:
		bool isEmpty() const override;
		const datatypes::AbstractDataPoint& operator*();
	};

	class DummyDataPoint : public datatypes::AbstractDataPoint
	{
	public:
		DummyDataPoint();
		std::string asString(datatypes::NumberFormat base) const override;
		std::unique_ptr<AbstractDataPoint> clone() const override;
	};

	class DummyAdapter : public gui::DataModelAdapter
	{
	public:
		DummyAdapter(std::vector<double>& data1, std::vector<double>& data2,
		    std::vector<double>& data3, datatypes::TimeStamp start);
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewestValueView(
		    const datatypes::DataObject& data) override;
		std::shared_ptr<datatypes::AbstractDataView> getDataView(
		    const datatypes::DataObject& data, datatypes::TimeSeries time) override;
		void writeData(const datatypes::DataObject& data, std::string value) override;
		void readCoEObject(const datatypes::CoEObject& obj) override;
		void resetAllErrorRegisters() override;
		void resetErrorRegisters(unsigned int slave) override;

	private:
		std::vector<double>& data1;
		std::vector<double>& data2;
		std::vector<double>& data3;
		datatypes::TimeStamp start;
	};

	class DummyBusInfoSupplier : public gui::BusInfoSupplier
	{
	public:
		DummyBusInfoSupplier(datatypes::TimeStamp time);
		int getPDOFramerate() override;
		int getRegisterFramerate() override;
		std::shared_ptr<datatypes::ErrorIterator> getErrorLog() override;
		std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
		getErrorStatistics() override;
		unsigned int getSlaveCount() override;
		const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveID) override;
		datatypes::BusMode getBusMode() override;
		datatypes::TimeStamp getStartTime() const override;
		std::vector<std::string> getProfileNames() override;
		std::vector<std::string> getInterfaceNames() override;

	private:
		datatypes::TimeStamp time;
	};

	class MainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		explicit MainWindow(QWidget* parent = nullptr);
		~MainWindow();

	private:
		gui::PlotList* plotList;
		DummyAdapter adapter;
		DummyBusInfoSupplier busInfo;
		std::vector<double> data1;
		std::vector<double> data2;
		std::vector<double> data3;
		datatypes::PDO obj1;
		datatypes::PDO obj2;
		datatypes::PDO obj3;
		std::random_device rd;
		std::mt19937 mtEngine;
		std::uniform_int_distribution<int> dist;
		QTimer* timer;
	};

} // namespace etherkitten::gui_test
