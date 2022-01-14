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

#include "DataReaderMock.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
#include <memory>

#include <etherkitten/reader/LogReader.hpp>
#include <etherkitten/reader/logger.hpp>

using namespace etherkitten::reader;
using namespace etherkitten::datatypes;

class MyErrorIterator : public ErrorIterator
{
public:
	MyErrorIterator(std::vector<std::pair<ErrorMessage, TimeStamp>> messages)
	    : messages(messages)
	{
	}

	bool hasNext() const override { return messages.size() > i + 1; }

	bool isEmpty() const override { return messages.size() <= i; }

	DataPoint<ErrorMessage> operator*() const override
	{
		return DataPoint{ messages.at(i).first, messages.at(i).second };
	}

	MyErrorIterator& operator++() override
	{
		i++;
		return *this;
	}

private:
	size_t i = 0;
	std::vector<std::pair<ErrorMessage, TimeStamp>> messages;
};

SCENARIO("Logger starts log, LogReder reads the logfile", "[Logger]")
{
	GIVEN("A logger and a LogReader")
	{
		DataReaderMock reader{ SlaveInformantMock{ 2, 3 } };
		PDO pdo1 = PDO(1, "PDO1", EtherCATDataTypeEnum::INTEGER16, 0, PDODirection::INPUT);
		PDO pdo2 = PDO(1, "test2", EtherCATDataTypeEnum::UNSIGNED8, 1, PDODirection::INPUT);
		CoEObject coe1(1, "CoE1", EtherCATDataTypeEnum::BIT6, 0, 0, 1);
		CoEObject coe2(2, "CoE1 of Slave2", EtherCATDataTypeEnum::INTEGER16, 0, 0, 1);
		reader.slaveInformant.feedSlaveInfo(1,
		    SlaveInfo(1, "Slave1", std::vector<PDO>{ pdo1, pdo2 },
		        std::vector<CoEEntry>{
		            CoEEntry(1, 0, CoEObjectCode::RECORD, "name", std::vector<CoEObject>{ coe1 }) },
		        ESIData{},
		        std::vector<std::byte>{ std::byte{ 1 }, std::byte{ 2 }, std::byte{ 3 },
		            std::byte{ 4 }, std::byte{ 5 } },
		        std::array<unsigned int, 4>{ 2, 0, 1, 6 }));
		reader.slaveInformant.feedSlaveInfo(2,
		    SlaveInfo(2, "Slave2",
		        std::vector<PDO>{ PDO(1, "PDO1 of Slave2", EtherCATDataTypeEnum::UNSIGNED32, 0,
		            PDODirection::INPUT) },
		        std::vector<CoEEntry>{
		            CoEEntry(2, 0, CoEObjectCode::RECORD, "name", std::vector<CoEObject>{ coe2 }) },
		        ESIData{},
		        std::vector<std::byte>{
		            std::byte{ 4 }, std::byte{ 5 }, std::byte{ 1 }, std::byte{ 4 } },
		        std::array<unsigned int, 4>{ 5, 0, std::numeric_limits<unsigned int>::max(), 2 }));

		WHEN("The reader has no data other than slaveInfo")
		{
			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{ {} }), "Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				logger.stopLog();
			}
			THEN("The log file is created") { REQUIRE(std::filesystem::exists("Testlog.ekl")); }
			THEN("The data is correct")
			{
				LogSlaveInformant logSlaveInformant{ "Testlog.ekl" };
				REQUIRE(logSlaveInformant.getIOMapSize() == 3);
				REQUIRE(logSlaveInformant.getSlaveCount() == 2);

				{
					const SlaveInfo slaveInfo = logSlaveInformant.getSlaveInfo(1);
					REQUIRE(slaveInfo.getID() == 1);
					REQUIRE(slaveInfo.getName() == "Slave1");
					REQUIRE(slaveInfo.getPDOs().size() == 2);
					REQUIRE(slaveInfo.getPDOs().at(0).getIndex() == 0);
					REQUIRE(slaveInfo.getPDOs().at(0).getName() == "PDO1");
					REQUIRE(slaveInfo.getPDOs().at(0).getSlaveID() == 1);
					// PDO direction is not written to log, thus not tested here
					REQUIRE(slaveInfo.getPDOs().at(0).getType() == EtherCATDataTypeEnum::INTEGER16);
					REQUIRE(slaveInfo.getCoEs().size() == 1);
					REQUIRE(slaveInfo.getCoEs().at(0).getIndex() == 0);
					REQUIRE(slaveInfo.getCoEs().at(0).getName() == "name");
					REQUIRE(slaveInfo.getCoEs().at(0).getObjectCode() == CoEObjectCode::RECORD);
					REQUIRE(slaveInfo.getCoEs().at(0).getSlaveID() == 1);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().size() == 1);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getSlaveID() == 1);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getIndex() == 0);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getSubIndex() == 0);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getName() == "CoE1");
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getType()
					    == EtherCATDataTypeEnum::BIT6);
					REQUIRE(slaveInfo.getESIBinary().size() == 5);
					REQUIRE(slaveInfo.getESIBinary().at(0) == std::byte{ 1 });
					REQUIRE(slaveInfo.getESIBinary().at(1) == std::byte{ 2 });
					REQUIRE(slaveInfo.getESIBinary().at(2) == std::byte{ 3 });
					REQUIRE(slaveInfo.getESIBinary().at(3) == std::byte{ 4 });
					REQUIRE(slaveInfo.getESIBinary().at(4) == std::byte{ 5 });
					REQUIRE(slaveInfo.getNeighbors().at(0) == 2);
					REQUIRE(slaveInfo.getNeighbors().at(1) == 0);
					REQUIRE(slaveInfo.getNeighbors().at(2) == 1);
					REQUIRE(slaveInfo.getNeighbors().at(3) == 6);
				}

				{
					const SlaveInfo slaveInfo = logSlaveInformant.getSlaveInfo(2);
					REQUIRE(slaveInfo.getID() == 2);
					REQUIRE(slaveInfo.getName() == "Slave2");
					REQUIRE(slaveInfo.getPDOs().size() == 1);
					REQUIRE(slaveInfo.getPDOs().at(0).getIndex() == 0);
					REQUIRE(slaveInfo.getPDOs().at(0).getName() == "PDO1 of Slave2");
					REQUIRE(slaveInfo.getPDOs().at(0).getSlaveID() == 2);
					// PDO direction is not written to log, thus not tested here
					REQUIRE(
					    slaveInfo.getPDOs().at(0).getType() == EtherCATDataTypeEnum::UNSIGNED32);
					REQUIRE(slaveInfo.getCoEs().size() == 1);
					REQUIRE(slaveInfo.getCoEs().at(0).getIndex() == 0);
					REQUIRE(slaveInfo.getCoEs().at(0).getName() == "name");
					REQUIRE(slaveInfo.getCoEs().at(0).getObjectCode() == CoEObjectCode::RECORD);
					REQUIRE(slaveInfo.getCoEs().at(0).getSlaveID() == 2);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().size() == 1);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getSlaveID() == 2);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getIndex() == 0);
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getSubIndex() == 0);
					REQUIRE(
					    slaveInfo.getCoEs().at(0).getObjects().at(0).getName() == "CoE1 of Slave2");
					REQUIRE(slaveInfo.getCoEs().at(0).getObjects().at(0).getType()
					    == EtherCATDataTypeEnum::INTEGER16);
					REQUIRE(slaveInfo.getESIBinary().size() == 4);
					REQUIRE(slaveInfo.getESIBinary().at(0) == std::byte{ 4 });
					REQUIRE(slaveInfo.getESIBinary().at(1) == std::byte{ 5 });
					REQUIRE(slaveInfo.getESIBinary().at(2) == std::byte{ 1 });
					REQUIRE(slaveInfo.getESIBinary().at(3) == std::byte{ 4 });
					REQUIRE(slaveInfo.getNeighbors().at(0) == 5);
					REQUIRE(slaveInfo.getNeighbors().at(1) == 0);
					REQUIRE(
					    slaveInfo.getNeighbors().at(2) == std::numeric_limits<unsigned int>::max());
					REQUIRE(slaveInfo.getNeighbors().at(3) == 2);
				}
			}
		}

		WHEN("The reader has some register updates")
		{
			Register reg = Register(1, RegisterEnum::BUILD);
			reader.feedRegister(reg, intToTimeStamp(100000), 0xAF78);
			reader.feedRegister(reg, intToTimeStamp(200000), 0x7F03);
			reader.feedRegister(reg, intToTimeStamp(300000), 0x1234);
			reader.feedRegister(reg, intToTimeStamp(400000), 0x0014);
			Register reg2 = Register(2, RegisterEnum::DC_RANGE);
			reader.feedRegister(reg2, intToTimeStamp(200000), 0x56);
			Register reg3 = Register(1, RegisterEnum::DLS_USER_R8);
			reader.feedRegister(reg3, intToTimeStamp(250000), 0x12345678);
			Register reg4 = Register(2, RegisterEnum::SYSTEM_TIME);
			reader.feedRegister(reg4, intToTimeStamp(150000), 0x9876987698769876ull);
			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{ {} }), "Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				logger.stopLog();
			}

			LogCache cache;
			LogSlaveInformant slaveInformant{ "Testlog.ekl" };
			LogReader reader{ "Testlog.ekl", slaveInformant, cache };
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			THEN("The LogReader creates NewestValueViews with the correct register data")
			{
				auto view = reader.getNewest(reg);
				REQUIRE(view->isEmpty() == false);
				REQUIRE((**view).asString(NumberFormat::HEXADECIMAL) == "0x14");
				REQUIRE(timeStampToInt((**view).getTime()) == 400000);
				auto view3 = reader.getNewest(reg3);
				REQUIRE(view3->isEmpty() == false);
				REQUIRE((**view3).asString(NumberFormat::HEXADECIMAL) == "0x12345678");
				REQUIRE(timeStampToInt((**view3).getTime()) == 250000);
				auto view4 = reader.getNewest(reg4);
				REQUIRE(view4->isEmpty() == false);
				REQUIRE((**view4).asString(NumberFormat::HEXADECIMAL) == "0x9876987698769876");
				REQUIRE(timeStampToInt((**view4).getTime()) == 150000);
			}

			THEN("The LogReader creates correct DataViews of register raw data")
			{
				auto view = reader.getRegisterRawDataView(reg2.getSlaveID(),
				    static_cast<uint16_t>(reg2.getRegister()), TimeSeries{ intToTimeStamp(0), 0s });
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == false);
				REQUIRE(view->asDouble() == 0x56);
				REQUIRE(view->getTime() == intToTimeStamp(200000));
			}

			THEN("The LogReader creates DataViews with the correct register data")
			{
				auto view = reader.getView(reg, TimeSeries{ intToTimeStamp(0), 0s });
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == true);
				REQUIRE(view->asDouble() == 0xAF78);
				REQUIRE(view->getTime() == intToTimeStamp(100000));
				++(*view);
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == true);
				REQUIRE(view->asDouble() == 0x7F03);
				REQUIRE(view->getTime() == intToTimeStamp(200000));
				++(*view);
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == true);
				REQUIRE(view->asDouble() == 0x1234);
				REQUIRE(view->getTime() == intToTimeStamp(300000));
				++(*view);
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == false);
				REQUIRE(view->asDouble() == 0x0014);
				REQUIRE(view->getTime() == intToTimeStamp(400000));
			}

			THEN("The LogReader calculates sensible frequencies for PDOs")
			{
				// Note that the LogReader doesn't actually calculate the register
				// framerate because it can't - it would require too much effort
				// to piece that information back together.
				REQUIRE(reader.getRegisterFrequency() == 0);
				REQUIRE(reader.getPDOFrequency() == 0);
			}
		}

		WHEN("The reader has some pdo data")
		{
			reader.appendPDOToIOMap(pdo1);
			reader.appendPDOToIOMap(pdo2);
			reader.feedPDOData(
			    std::vector<std::pair<PDO, uint64_t>>{ std::pair<PDO, uint64_t>{ pdo1, 0x4567 },
			        std::pair<PDO, uint64_t>{ pdo2, 0x89 } },
			    intToTimeStamp(112340));
			reader.feedPDOData(
			    std::vector<std::pair<PDO, uint64_t>>{ std::pair<PDO, uint64_t>{ pdo1, 0x4598 },
			        std::pair<PDO, uint64_t>{ pdo2, 0x65 } },
			    intToTimeStamp(134340));
			reader.feedPDOData(
			    std::vector<std::pair<PDO, uint64_t>>{ std::pair<PDO, uint64_t>{ pdo1, 0x4796 },
			        std::pair<PDO, uint64_t>{ pdo2, 0x95 } },
			    intToTimeStamp(150987));

			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{ {} }), "Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				logger.stopLog();
			}

			LogCache cache;
			LogSlaveInformant slaveInformant{ "Testlog.ekl" };
			LogReader reader{ "Testlog.ekl", slaveInformant, cache };
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			THEN("The LogReader can retrieve DataViews for the pdos")
			{
				auto view = reader.getView(pdo1, TimeSeries{ intToTimeStamp(0), 0s });
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == true);
				REQUIRE(view->asDouble() == 0x4567);
				REQUIRE(view->getTime() == intToTimeStamp(112340));
				++(*view);
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == true);
				REQUIRE(view->asDouble() == 0x4598);
				REQUIRE(view->getTime() == intToTimeStamp(134340));
				++(*view);
				REQUIRE(view->isEmpty() == false);
				REQUIRE(view->hasNext() == false);
				REQUIRE(view->asDouble() == 0x4796);
				REQUIRE(view->getTime() == intToTimeStamp(150987));
			}

			THEN("The LogReader can retrieve NewestValueViews for the pdos")
			{
				auto view = reader.getNewest(pdo1);
				REQUIRE(view->isEmpty() == false);
				REQUIRE((**view).asString(NumberFormat::HEXADECIMAL) == "0x4796");
				REQUIRE((**view).getTime() == intToTimeStamp(150987));
				auto view2 = reader.getNewest(pdo2);
				REQUIRE(view2->isEmpty() == false);
				REQUIRE((**view2).asString(NumberFormat::HEXADECIMAL) == "0x95");
				REQUIRE((**view2).getTime() == intToTimeStamp(150987));
			}

			THEN("The LogReader calculates sensible frequencies for registers and PDOs")
			{
				REQUIRE(reader.getRegisterFrequency() == 0);
				REQUIRE(reader.getPDOFrequency() > 0);
				REQUIRE(reader.getPDOFrequency() < 1000000);
			}
		}

		WHEN("The logger has some CoE data")
		{
			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{ {} }), "Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				logger.updateCoE(coe2,
				    std::make_shared<DataPoint<EtherCATDataType::INTEGER16>>(
				        -147, intToTimeStamp(1234567)));
				logger.updateCoE(coe2,
				    std::make_shared<DataPoint<EtherCATDataType::INTEGER16>>(
				        578, intToTimeStamp(6544567)));

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				logger.stopLog();
			}

			THEN("The newest value and time can be read via LogCache")
			{
				LogCache cache;
				LogSlaveInformant slaveInformant{ "Testlog.ekl" };
				LogReader reader{ "Testlog.ekl", slaveInformant, cache };
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				std::unique_ptr<AbstractNewestValueView> abstractView
				    = cache.getNewest(CoEObject(coe2));
				CoENewestValueView& view = dynamic_cast<CoENewestValueView&>(*abstractView);
				const DataPoint<EtherCATDataType::INTEGER16>& point
				    = dynamic_cast<const DataPoint<EtherCATDataType::INTEGER16>&>(*view);

				REQUIRE(point.getValue() == 578);
				REQUIRE(point.getTime() == intToTimeStamp(6544567));
			}
		}

		WHEN("The logger has one CoE data")
		{
			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{ {} }), "Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				logger.updateCoE(coe2,
				    std::make_shared<DataPoint<EtherCATDataType::INTEGER16>>(
				        -147, intToTimeStamp(1234567)));

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				logger.stopLog();
			}

			THEN("The value and time can be read via LogQueues")
			{
				LogCache cache;
				LogSlaveInformant slaveInformant{ "Testlog.ekl" };
				LogReader reader{ "Testlog.ekl", slaveInformant, cache };
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				std::unique_ptr<AbstractNewestValueView> abstractView
				    = cache.getNewest(CoEObject(coe2));
				CoENewestValueView& view = dynamic_cast<CoENewestValueView&>(*abstractView);
				const DataPoint<EtherCATDataType::INTEGER16>& point
				    = dynamic_cast<const DataPoint<EtherCATDataType::INTEGER16>&>(*view);

				REQUIRE(point.getValue() == -147);
				REQUIRE(point.getTime() == intToTimeStamp(1234567));
			}
		}

		WHEN("The reader has a lot of register data and we set a memory limit")
		{
			Register reg = Register(1, RegisterEnum::BUILD);
			for (int i = 0; i < 2000; ++i)
			{
				reader.feedRegister(reg, intToTimeStamp(i * 100000), 0);
			}

			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{ {} }), "Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				logger.stopLog();
			}

			LogCache cache;
			LogSlaveInformant slaveInformant{ "Testlog.ekl" };
			LogReader reader{ "Testlog.ekl", slaveInformant, cache };
			reader.setMaximumMemory(1);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			THEN("We will end up with the register data starting at a later TimeStamp")
			{
				auto view = reader.getView(reg, { intToTimeStamp(0), 0s });
				REQUIRE(view->getTime() > intToTimeStamp(0));
			}
		}

		WHEN("The logger has some error messages")
		{
			{
				Logger logger{ reader.slaveInformant, reader,
					std::make_shared<MyErrorIterator>(MyErrorIterator{
					    { { ErrorMessage{ "test 1", ErrorSeverity::LOW }, intToTimeStamp(100000) },
					        { ErrorMessage{ "test 2", ErrorSeverity::MEDIUM },
					            intToTimeStamp(200000) },
					        { ErrorMessage{ "test 3", { 2767, 35 }, ErrorSeverity::FATAL },
					            intToTimeStamp(300000) } } }),
					"Testlog.ekl" };
				logger.startLog(etherkitten::datatypes::intToTimeStamp(0));

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				logger.stopLog();
			}

			LogCache cache;
			LogSlaveInformant slaveInformant{ "Testlog.ekl" };
			LogReader reader{ "Testlog.ekl", slaveInformant, cache };
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			THEN("We get the errors again from LogQueues")
			{
				auto errors = cache.getErrors();
				if (errors->isEmpty())
				{
					REQUIRE(errors->hasNext());
					++(*errors);
				}
				REQUIRE(timeStampToInt((**errors).getTime()) == 100000);
				REQUIRE((**errors).getValue().getSeverity() == ErrorSeverity::LOW);
				REQUIRE((**errors).getValue().getMessage() == "test 1");
				REQUIRE((**errors).getValue().getAssociatedSlaves().first
				    == std::numeric_limits<unsigned int>::max());
				++(*errors);
				REQUIRE((**errors).getValue().getMessage() == "test 2");
				++(*errors);
				REQUIRE((**errors).getValue().getMessage() == "test 3");
				REQUIRE((**errors).getValue().getAssociatedSlaves().first == 2767);
				REQUIRE((**errors).getValue().getAssociatedSlaves().second == 35);
			}
		}
	}

	std::filesystem::remove("Testlog.ekl");
}
