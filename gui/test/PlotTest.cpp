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

#include "PlotTest.hpp"
#include <QApplication>
#include <QTabWidget>
#include <chrono>

namespace etherkitten::gui_test
{

	DummyDataView::DummyDataView(
	    std::vector<double>& data, datatypes::TimeSeries series, datatypes::TimeStamp start)
	    : data(data)
	    , series(series)
	    , start(start)
	{
		step = series.microStep.count() / 1000;
		pos = std::chrono::duration_cast<std::chrono::milliseconds>(series.startTime - start)
		          .count();
		if (pos < 0)
			pos = 0;
		if (pos >= static_cast<long>(data.size() * 100))
			pos = (static_cast<long>(data.size()) - 1) * 100;
	}

	double DummyDataView::asDouble() const
	{
		if (static_cast<size_t>(pos / 100) >= data.size())
			return 0;
		return data[static_cast<size_t>(pos / 100)];
	}

	datatypes::AbstractDataView& DummyDataView::operator++()
	{
		long oldIdx = pos / 100;
		while (oldIdx == pos / 100)
			pos += step > 0 ? step : 1;
		if (pos >= static_cast<long>(data.size() * 100))
			pos = static_cast<long>(data.size() - 1) * 100;
		return *this;
	}

	bool DummyDataView::hasNext() const
	{
		if (step == 0)
		{
			if (pos < static_cast<long>(data.size() - 1) * 100)
				return true;
			else
				return false;
		}
		else
		{
			return (pos + step) <= static_cast<long>(data.size() - 1) * 100;
		}
	}

	bool DummyDataView::isEmpty() const { return false; }

	datatypes::TimeStamp DummyDataView::getTime()
	{
		return start + std::chrono::milliseconds((pos / 100) * 100);
	}

	const datatypes::AbstractDataPoint& DummyNewestValueView::operator*()
	{
		DummyDataPoint* point = new DummyDataPoint();
		return *point;
	}

	bool DummyNewestValueView::isEmpty() const { return false; }

	DummyDataPoint::DummyDataPoint()
	    : datatypes::AbstractDataPoint(datatypes::now())
	{
	}

	std::string DummyDataPoint::asString(datatypes::NumberFormat base) const
	{
		(void)base;
		return "";
	}

	std::unique_ptr<datatypes::AbstractDataPoint> DummyDataPoint::clone() const
	{
		return std::make_unique<DummyDataPoint>(*this);
	}

	DummyAdapter::DummyAdapter(std::vector<double>& data1, std::vector<double>& data2,
	    std::vector<double>& data3, datatypes::TimeStamp start)
	    : data1(data1)
	    , data2(data2)
	    , data3(data3)
	    , start(start)
	{
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> DummyAdapter::getNewestValueView(
	    const datatypes::DataObject& data)
	{
		(void)data;
		return std::make_unique<DummyNewestValueView>();
	}

	std::shared_ptr<datatypes::AbstractDataView> DummyAdapter::getDataView(
	    const datatypes::DataObject& data, datatypes::TimeSeries time)
	{
		switch (data.getSlaveID())
		{
		case 0:
			return std::make_shared<DummyDataView>(data1, time, start);
		case 1:
			return std::make_shared<DummyDataView>(data2, time, start);
		default:
			return std::make_shared<DummyDataView>(data3, time, start);
		}
	}

	void DummyAdapter::writeData(const datatypes::DataObject& data, std::string value)
	{
		(void)data;
		(void)value;
	}

	void DummyAdapter::readCoEObject(const datatypes::CoEObject& obj) { (void)obj; }

	void DummyAdapter::resetAllErrorRegisters() {}

	void DummyAdapter::resetErrorRegisters(unsigned int slave) { (void)slave; }

	DummyBusInfoSupplier::DummyBusInfoSupplier(datatypes::TimeStamp time)
	    : time(time)
	{
	}

	int DummyBusInfoSupplier::getPDOFramerate() { return 0; }

	int DummyBusInfoSupplier::getRegisterFramerate() { return 0; }

	std::shared_ptr<datatypes::ErrorIterator> DummyBusInfoSupplier::getErrorLog()
	{
		return nullptr;
	}

	std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
	DummyBusInfoSupplier::getErrorStatistics()
	{
		return std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>();
	}

	unsigned int DummyBusInfoSupplier::getSlaveCount() { return 0; }

	const datatypes::SlaveInfo& DummyBusInfoSupplier::getSlaveInfo(unsigned int slaveID)
	{
		(void)slaveID;
		static datatypes::SlaveInfo info = datatypes::SlaveInfo(0, "",
		    std::vector<datatypes::PDO>(), std::vector<datatypes::CoEEntry>(), datatypes::ESIData(),
		    std::vector<std::byte>(), std::array<unsigned int, 4>());
		return info;
	}

	datatypes::BusMode DummyBusInfoSupplier::getBusMode() { return datatypes::BusMode::READ_ONLY; }

	datatypes::TimeStamp DummyBusInfoSupplier::getStartTime() const { return time; }

	std::vector<std::string> DummyBusInfoSupplier::getProfileNames()
	{
		return std::vector<std::string>();
	}

	std::vector<std::string> DummyBusInfoSupplier::getInterfaceNames()
	{
		return std::vector<std::string>();
	}

	MainWindow::MainWindow(QWidget* parent)
	    : QMainWindow(parent)
	    , adapter(data1, data2, data3, datatypes::now())
	    , busInfo(datatypes::now())
	    , obj1(0, "PDO 1", datatypes::EtherCATDataTypeEnum::REAL64, 0,
	          datatypes::PDODirection::OUTPUT)
	    , obj2(1, "PDO 2", datatypes::EtherCATDataTypeEnum::REAL64, 0,
	          datatypes::PDODirection::OUTPUT)
	    , obj3(2, "PDO 3", datatypes::EtherCATDataTypeEnum::REAL64, 0,
	          datatypes::PDODirection::OUTPUT)
	{
		mtEngine = std::mt19937(rd());
		dist = std::uniform_int_distribution(-11, 11);
		data1.emplace_back(dist(mtEngine));
		data2.emplace_back(dist(mtEngine));
		data3.emplace_back(dist(mtEngine));
		QTabWidget* w = new QTabWidget(this);
		setCentralWidget(w);
		QLabel* l = new QLabel("bla");
		w->addTab(l, "Hi");
		plotList = new gui::PlotList(this, busInfo, adapter);
		QFrame* frm = new QFrame(w);
		QVBoxLayout* layout = new QVBoxLayout(frm);
		w->addTab(frm, "Plots");
		layout->addWidget(plotList);
		timer = new QTimer(this);
		connect(timer, &QTimer::timeout, [this]() {
			data1.emplace_back(dist(mtEngine));
			data3.emplace_back(dist(mtEngine));
			plotList->updateData();
		});
		connect(timer, &QTimer::timeout, [this]() { data2.emplace_back(dist(mtEngine)); });
		timer->start(100);
		plotList->addPlot(obj1);
		plotList->addPlot(obj2);
		plotList->addPlot(obj3);
		plotList->addToPlot(1, obj1);
		plotList->addToPlot(1, obj3);
		QPushButton* btn = new QPushButton("Add graph");
		layout->addWidget(btn);
		connect(btn, &QPushButton::clicked, [this]() { plotList->addPlot(obj1); });
	}

	MainWindow::~MainWindow() { delete timer; }

} // namespace etherkitten::gui_test

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	etherkitten::gui_test::MainWindow w;
	w.show();

	return a.exec();
}
