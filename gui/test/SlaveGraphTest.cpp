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

#include "SlaveGraphTest.hpp"
#include <QApplication>
#include <array>
#include <etherkitten/gui/SlaveGraph.hpp>
#include <random>

namespace etherkitten::gui_test
{
	// 20 Slaves + 1 Master
	const unsigned int numSlaves = 20;
	DummyErrorIterator& DummyErrorIterator::operator++() { return *this; }

	bool DummyErrorIterator::hasNext() const
	{
		static std::random_device rd;
		static std::mt19937 mtEngine(rd());
		static std::uniform_int_distribution dist(0, 10);
		return dist(mtEngine) == 0;
	}

	bool DummyErrorIterator::isEmpty() const { return false; }

	datatypes::DataPoint<datatypes::ErrorMessage> DummyErrorIterator::operator*() const
	{
		static std::random_device rd;
		static std::mt19937 mtEngine(rd());
		static std::uniform_int_distribution dist(0, static_cast<int>(numSlaves - 1));
		static std::uniform_int_distribution dist2(0, 100);
		/* a bit hacky - when the second slave is greater than the number of slaves, it
		 * is ignored by the SlaveGraph, so the error is shown as only affecting slave s1 */
		unsigned int s1 = static_cast<unsigned int>(dist(mtEngine));
		unsigned int s2 = static_cast<unsigned int>(dist2(mtEngine));
		datatypes::ErrorSeverity severity = (s2 < 10 || s2 > 80) ? datatypes::ErrorSeverity::LOW
		                                                         : datatypes::ErrorSeverity::MEDIUM;
		return datatypes::DataPoint(
		    datatypes::ErrorMessage("Error message", std::make_pair(s1, s2), severity),
		    datatypes::TimeStamp());
	}

	DummyBusInfoSupplier::DummyBusInfoSupplier()
	{
		unsigned int max = std::numeric_limits<unsigned int>::max();
		slaves.emplace_back(1, "Hi", std::vector<datatypes::PDO>(),
		    std::vector<datatypes::CoEEntry>(), datatypes::ESIData(), std::vector<std::byte>(),
		    std::array<unsigned int, 4>({ 2, 0, max, max }));
		for (unsigned int i = 1; i <= numSlaves; i++)
		{
			datatypes::ESIData esi;
			esi.strings = { "bla" };
			esi.general = datatypes::ESIGeneral();
			esi.general->nameIdx = 0;
			slaves.emplace_back(i + 1, "Hi", std::vector<datatypes::PDO>(),
			    std::vector<datatypes::CoEEntry>(), std::move(esi), std::vector<std::byte>(),
			    std::array<unsigned int, 4>({ i + 2, max, max, max }));
		}
	}

	int DummyBusInfoSupplier::getPDOFramerate() { return 0; }

	int DummyBusInfoSupplier::getRegisterFramerate() { return 0; }

	std::shared_ptr<datatypes::ErrorIterator> DummyBusInfoSupplier::getErrorLog()
	{
		return std::make_shared<DummyErrorIterator>();
	}

	std::vector<std::reference_wrapper<const datatypes::ErrorStatistic>>
	DummyBusInfoSupplier::getErrorStatistics()
	{
		/* just ignore this for now... */
		throw std::runtime_error("not implemented");
	}

	unsigned int DummyBusInfoSupplier::getSlaveCount() { return numSlaves; }

	datatypes::SlaveInfo& DummyBusInfoSupplier::getSlaveInfo(unsigned int slaveID)
	{
		return slaves.at(slaveID - 1);
	}

	datatypes::BusMode DummyBusInfoSupplier::getBusMode()
	{
		return datatypes::BusMode::READ_WRITE_OP;
	}

	datatypes::TimeStamp DummyBusInfoSupplier::getStartTime() const { return datatypes::now(); }

	std::vector<std::string> DummyBusInfoSupplier::getProfileNames() { return {}; }

	std::vector<std::string> DummyBusInfoSupplier::getInterfaceNames() { return {}; }

	double DummyDataView::asDouble() const
	{
		static std::random_device rd;
		static std::mt19937 mtEngine(rd());
		static std::uniform_real_distribution<double> dist(0.0, 0.00003);
		return dist(mtEngine);
	}

	datatypes::AbstractDataView& DummyDataView::operator++() { return *this; }

	bool DummyDataView::hasNext() const { return false; }

	bool DummyDataView::isEmpty() const { return false; }

	datatypes::TimeStamp DummyDataView::getTime() { throw std::runtime_error("not implemented"); }

	const datatypes::AbstractDataPoint& DummyNewestValueView::operator*()
	{
		DummyDataPoint* point = new DummyDataPoint();
		return *point;
	}

	bool DummyNewestValueView::isEmpty() const { return false; }

	DummyDataPoint::DummyDataPoint()
	    : datatypes::AbstractDataPoint(datatypes::now())
	{
		static std::random_device rd;
		static std::mt19937 mtEngine(rd());
		static std::uniform_int_distribution<unsigned int> dist(0, 9);
		value = dist(mtEngine);
	}

	std::string DummyDataPoint::asString(datatypes::NumberFormat base) const
	{
		return datatypes::EtherCATTypeStringFormatter<
		    datatypes::EtherCATDataType::UNSIGNED32>::asString(value, base);
	}

	std::unique_ptr<datatypes::AbstractDataPoint> DummyDataPoint::clone() const
	{
		return std::make_unique<DummyDataPoint>(*this);
	}

	DummyAdapter::DummyAdapter() {}

	std::unique_ptr<datatypes::AbstractNewestValueView> DummyAdapter::getNewestValueView(
	    const datatypes::DataObject& data)
	{
		(void)data;
		return std::make_unique<DummyNewestValueView>();
	}

	std::shared_ptr<datatypes::AbstractDataView> DummyAdapter::getDataView(
	    const datatypes::DataObject& data, datatypes::TimeSeries time)
	{
		(void)data;
		(void)time;
		return std::make_shared<DummyDataView>();
	}

	void DummyAdapter::writeData(const datatypes::DataObject& data, std::string value)
	{
		std::cout << "Write data for object " << data.getName() << ": " << value << std::endl;
	}

	void DummyAdapter::readCoEObject(const datatypes::CoEObject& obj)
	{
		(void)obj;
		std::cout << "Read CoE object: " << obj.getName() << std::endl;
	}

	void DummyAdapter::resetAllErrorRegisters()
	{
		std::cout << "Reset all error registers" << std::endl;
	}

	void DummyAdapter::resetErrorRegisters(unsigned int slave)
	{
		std::cout << "Reset error registers for slave " << slave << std::endl;
	}

	MainWindow::MainWindow(QWidget* parent)
	    : QMainWindow(parent)
	{
		gui::SlaveGraph* graph = new gui::SlaveGraph(this, adapter, busInfo, layout);
		setCentralWidget(graph);
		graph->setupData();
		graph->writeBusLayout();
		config::Position pos(100, 50);
		layout.setPosition(0, pos);
		graph->applyBusLayout();
		timer = new QTimer();
		connect(timer, &QTimer::timeout, [graph]() { graph->updateData(); });
		timer->start(200);
		timer2 = new QTimer();
		connect(timer2, &QTimer::timeout, [graph]() { graph->clearErrors(); });
		timer2->start(30000);
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
