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

#include "SlaveTreeTest.hpp"
#include <QApplication>
#include <QTabWidget>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

namespace etherkitten::slave_tree_test
{

	// 20 Slaves + 1 Master
	const unsigned int numSlaves = 20;

	DummyBusInfoSupplier::DummyBusInfoSupplier()
	    : safeOP(false)
	{
		unsigned int max = std::numeric_limits<unsigned int>::max();
		for (unsigned int i = 1; i <= numSlaves; i++)
		{
			datatypes::ESIData esi;
			esi.strings = { "bla" };
			esi.general = datatypes::ESIGeneral();
			esi.general->nameIdx = 0;
			std::vector<datatypes::PDO> pdos = { datatypes::PDO(0, "PDO1",
				datatypes::EtherCATDataTypeEnum::UNSIGNED16, 0, datatypes::PDODirection::INPUT) };
			std::vector<datatypes::CoEObject> coes = { datatypes::CoEObject(
				0, "CoE1", datatypes::EtherCATDataTypeEnum::UNSIGNED16, 0, 0, 4) };
			std::vector<datatypes::CoEEntry> coeEntries = { datatypes::CoEEntry(
				0, 0, datatypes::CoEObjectCode::VAR, "CoE Entry 1", std::move(coes)) };
			slaves.emplace_back(i, "Hi", std::move(pdos), std::move(coeEntries), std::move(esi),
			    std::vector<std::byte>(), std::array<unsigned int, 4>({ i + 2, max, max, max }));
		}
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

	unsigned int DummyBusInfoSupplier::getSlaveCount() { return numSlaves; }

	datatypes::SlaveInfo& DummyBusInfoSupplier::getSlaveInfo(unsigned int slaveID)
	{
		return slaves.at(slaveID - 1);
	}

	datatypes::BusMode DummyBusInfoSupplier::getBusMode()
	{
		return safeOP ? datatypes::BusMode::READ_WRITE_SAFE_OP : datatypes::BusMode::READ_WRITE_OP;
	}

	datatypes::TimeStamp DummyBusInfoSupplier::getStartTime() const { return datatypes::now(); }

	std::vector<std::string> DummyBusInfoSupplier::getProfileNames() { return {}; }

	std::vector<std::string> DummyBusInfoSupplier::getInterfaceNames() { return {}; }

	void DummyBusInfoSupplier::toggleSafeOP() { safeOP = !safeOP; }

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
		static std::uniform_int_distribution<unsigned int> dist(0, 100);
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
		throw std::runtime_error("not implemented");
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

	SlaveTreeTest::SlaveTreeTest(QWidget* parent)
	    : QMainWindow(parent)
	{
		QFrame* frame = new QFrame(this);
		setCentralWidget(frame);
		QVBoxLayout* layout = new QVBoxLayout(frame);
		QPushButton* btn = new QPushButton("Toggle SafeOP", frame);
		layout->addWidget(btn);
		gui::SlaveTree* slavetree = new gui::SlaveTree(this, adapter, busInfo, tooltipFormatter);
		registers[datatypes::RegisterEnum::TYPE] = true;
		registers[datatypes::RegisterEnum::BUILD] = true;
		registers[datatypes::RegisterEnum::SII_BUSY] = true;
		slavetree->setupData(registers);
		slavetree->jumpToSlave(5);
		connect(btn, &QPushButton::clicked, [this]() { busInfo.toggleSafeOP(); });
		QPushButton* btn2 = new QPushButton("Change registers", frame);
		layout->addWidget(btn2);
		layout->addWidget(slavetree);
		connect(btn2, &QPushButton::clicked, [this, slavetree]() {
			registers[datatypes::RegisterEnum::SII_BUSY]
			    = !registers[datatypes::RegisterEnum::SII_BUSY];
			slavetree->rebuildRegisters(registers);
		});
		slavetree->setBusLive(true);
		timer = new QTimer();
		connect(timer, &QTimer::timeout, [slavetree]() { slavetree->updateData(); });
		timer->start(100);
	}

} // namespace etherkitten::slave_tree_test

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	etherkitten::slave_tree_test::SlaveTreeTest w;
	w.show();

	return a.exec();
}
