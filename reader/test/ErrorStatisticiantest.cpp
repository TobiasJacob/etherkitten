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

#include "etherkitten/datatypes/SlaveInfo.hpp"
#include "etherkitten/datatypes/dataobjects.hpp"
#include "etherkitten/datatypes/datapoints.hpp"
#include "etherkitten/datatypes/dataviews.hpp"
#include "etherkitten/datatypes/errorstatistic.hpp"
#include "etherkitten/datatypes/register.hpp"
#include "etherkitten/datatypes/time.hpp"
#include <catch2/catch.hpp>

#include <etherkitten/reader/ErrorStatistician.hpp>
#include <limits>
#include <memory>
#include <thread>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;
class ESTestSlaveInformant : public SlaveInformant
{
public:
	unsigned int getSlaveCount() const override { return 2; }

	const ekdatatypes::SlaveInfo& getSlaveInfo(unsigned int slaveIndex) const override
	{
		switch (slaveIndex)
		{
		case 1:
			return slave1;
		default: // 2
			return slave2;
		}
	}

	uint64_t getIOMapSize() const override { return 0; }

	const std::vector<ekdatatypes::ErrorMessage> getInitializationErrors() override
	{
		const std::vector<ekdatatypes::ErrorMessage> errorsTemp = initErrors;
		initErrors = std::vector<ekdatatypes::ErrorMessage>();
		return errorsTemp;
	}

private:
	unsigned int none = std::numeric_limits<unsigned int>::max();
	ekdatatypes::SlaveInfo slave1
	    = ekdatatypes::SlaveInfo(1, "Slave #1", {}, {}, { 0, 2, none, none });
	ekdatatypes::SlaveInfo slave2
	    = ekdatatypes::SlaveInfo(2, "Slave #2", {}, {}, { 1, none, none, none });
	std::vector<ekdatatypes::ErrorMessage> initErrors{ { "Testerror",
		ekdatatypes::ErrorSeverity::MEDIUM } };
};

class ESTestReader : public Reader
{
public:
	std::unique_ptr<ekdatatypes::AbstractNewestValueView> getNewest(
	    const ekdatatypes::PDO& /*pdo*/) override
	{
		return nullptr;
	}

	std::unique_ptr<ekdatatypes::AbstractNewestValueView> getNewest(
	    const ekdatatypes::Register& /*reg*/) override
	{
		return nullptr;
	}

	std::shared_ptr<ekdatatypes::AbstractDataView> getView(
	    const ekdatatypes::PDO& /*pdo*/, ekdatatypes::TimeSeries /*time*/) override
	{
		return nullptr;
	}

	std::shared_ptr<ekdatatypes::AbstractDataView> getView(
	    const ekdatatypes::Register& reg, ekdatatypes::TimeSeries /*time*/) override
	{
		if (registerMaps.count(reg.getRegister()) == 0)
		{
			registerMaps[reg.getRegister()] = std::vector<SearchList<uint8_t, nodeSize>>(2);
			if (reg.getRegister() == ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_0
			    || reg.getRegister() == ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_1
			    || reg.getRegister() == ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_2
			    || reg.getRegister() == ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_3)
			{
				registerMaps[reg.getRegister()].at(0).append(5, ekdatatypes::TimeStamp() + 10ms);
				registerMaps[reg.getRegister()].at(0).append(5, ekdatatypes::TimeStamp() + 20ms);
				registerMaps[reg.getRegister()].at(1).append(2, ekdatatypes::TimeStamp() + 10ms);
				registerMaps[reg.getRegister()].at(1).append(2, ekdatatypes::TimeStamp() + 20ms);
			}
			else if (reg.getRegister() == ekdatatypes::RegisterEnum::MALFORMAT_FRAME_COUNTER)
			{
				registerMaps[reg.getRegister()].at(0).append(13, ekdatatypes::TimeStamp() + 10ms);
				registerMaps[reg.getRegister()].at(0).append(13, ekdatatypes::TimeStamp() + 20ms);
				registerMaps[reg.getRegister()].at(1).append(0, ekdatatypes::TimeStamp() + 10ms);
				registerMaps[reg.getRegister()].at(1).append(0, ekdatatypes::TimeStamp() + 20ms);
			}
			else if (reg.getRegister() == ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_0
			    || reg.getRegister() == ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_1
			    || reg.getRegister() == ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_2
			    || reg.getRegister() == ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_3)
			{
				registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_0]
				    = std::vector<SearchList<uint8_t, nodeSize>>(2);
				registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_1]
				    = std::vector<SearchList<uint8_t, nodeSize>>(2);
				registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_2]
				    = std::vector<SearchList<uint8_t, nodeSize>>(2);
				registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_3]
				    = std::vector<SearchList<uint8_t, nodeSize>>(2);
				ekdatatypes::TimeStamp start = {};
				start += 10ms;
				uint8_t lastLLErrorCount = 0;
				for (ekdatatypes::TimeStamp time = start; time < start + 1s; time += 5ms)
				{
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_0].at(0).append(
					    ++lastLLErrorCount, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_0].at(1).append(
					    0, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_1].at(0).append(
					    0, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_1].at(1).append(
					    0, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_2].at(0).append(
					    0, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_2].at(1).append(
					    0, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_3].at(0).append(
					    0, time);
					registerMaps[ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_3].at(1).append(
					    0, time);
				}
			}
		}

		return registerMaps.at(reg.getRegister())
		    .at(reg.getSlaveID() - 1)
		    .getView({ {}, {} }, false);
	}

	std::shared_ptr<DataView<std::unique_ptr<IOMap>, nodeSize, IOMap*>> getIOMapView(
	    ekdatatypes::TimeStamp /*startTime*/) override
	{
		return nullptr;
	}

	std::shared_ptr<ekdatatypes::AbstractDataView> getRegisterRawDataView(
	    const uint16_t /*slaveId*/, const uint16_t /*regId*/,
	    ekdatatypes::TimeSeries /*time*/) override
	{
		return nullptr;
	}

	ekdatatypes::PDOInfo getAbsolutePDOInfo(const ekdatatypes::PDO& /*pdo*/) override { return {}; }

	double getPDOFrequency() override { return 1000; }

	double getRegisterFrequency() override { return 1000; }

	void changeRegisterSettings(
	    const std::unordered_map<ekdatatypes::RegisterEnum, bool>& toRead) override
	{
		(void)toRead;
	}

	void toggleBusSafeOp() override {}

	ekdatatypes::BusMode getBusMode() override { return ekdatatypes::BusMode::READ_WRITE_OP; }

	void setMaximumMemory(size_t size) override { (void)size; }

	void messageHalt() override {}

	virtual ekdatatypes::TimeStamp getStartTime() const override
	{
		return ekdatatypes::TimeStamp();
	}

private:
	std::unordered_map<ekdatatypes::RegisterEnum, std::vector<SearchList<uint8_t, nodeSize>>>
	    registerMaps;
};

SCENARIO("ErrorStatistician can report error statistics", "[ErrorStatistician]")
{
	GIVEN("SlaveInformant and Reader mocks and an ErrorStatistician")
	{
		ESTestSlaveInformant slaveInformant;
		ESTestReader reader;
		ErrorStatistician errorStatistician(slaveInformant, reader);
		WHEN("I request an ErrorStatistic")
		{
			ekdatatypes::ErrorStatisticType type
			    = ekdatatypes::ErrorStatisticType::FREQ_SLAVE_FRAME_ERROR;
			unsigned int slaveId = 1;
			ekdatatypes::ErrorStatistic& errorStat
			    = errorStatistician.getErrorStatistic(type, slaveId);
			THEN("I get a correct object")
			{
				REQUIRE(errorStat.getStatisticType() == type);
				REQUIRE(errorStat.getSlaveID() == slaveId);
			}
		}
		WHEN("I request a views for a single-register slave-specific error total")
		{
			ekdatatypes::ErrorStatisticType type
			    = ekdatatypes::ErrorStatisticType::TOTAL_SLAVE_MALFORMAT_FRAME_ERROR;
			unsigned int slaveId = 1;
			ekdatatypes::ErrorStatistic& errorStat
			    = errorStatistician.getErrorStatistic(type, slaveId);
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> nvv
			    = errorStatistician.getNewest(errorStat);
			std::shared_ptr<ekdatatypes::AbstractDataView> dv = errorStatistician.getView(
			    errorStat, { ekdatatypes::TimeStamp::min(), ekdatatypes::TimeStep::zero() });
			THEN("The views result in the same value the register has")
			{
				REQUIRE_FALSE(nvv->isEmpty());
				REQUIRE_THAT(
				    dynamic_cast<const ekdatatypes::DataPoint<double>*>(&**nvv)->getValue(),
				    Catch::Matchers::WithinRel(13.0, 0.00001));
				REQUIRE((!dv->isEmpty() || (dv->hasNext() && !(++*dv).isEmpty())));
				REQUIRE_THAT(dv->asDouble(), Catch::Matchers::WithinRel(13.0, 0.00001));
			}
		}
		WHEN("I request a NewestValueView for a global single-register error total that a single "
		     "slave had")
		{
			ekdatatypes::ErrorStatisticType type
			    = ekdatatypes::ErrorStatisticType::TOTAL_MALFORMAT_FRAME_ERROR;
			unsigned int slaveId = std::numeric_limits<unsigned int>::max();
			ekdatatypes::ErrorStatistic& errorStat
			    = errorStatistician.getErrorStatistic(type, slaveId);
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> nvv
			    = errorStatistician.getNewest(errorStat);
			THEN("The view results in the expected value")
			{
				REQUIRE_FALSE(nvv->isEmpty());
				INFO("Only one slave had this error, therefore it is equal to that one");
				REQUIRE_THAT(
				    dynamic_cast<const ekdatatypes::DataPoint<double>*>(&**nvv)->getValue(),
				    Catch::Matchers::WithinRel(13.0, 0.00001));
			}
		}
		WHEN("I request a NewestValueView for a global multiple-register error total that multiple "
		     "slaves had")
		{
			ekdatatypes::ErrorStatisticType type
			    = ekdatatypes::ErrorStatisticType::TOTAL_FRAME_ERROR;
			unsigned int slaveId = std::numeric_limits<unsigned int>::max();
			ekdatatypes::ErrorStatistic& errorStat
			    = errorStatistician.getErrorStatistic(type, slaveId);
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> nvv
			    = errorStatistician.getNewest(errorStat);
			THEN("The view results in the expected value")
			{
				REQUIRE_FALSE(nvv->isEmpty());
				INFO("There are four relevant registers with values 2 and 5 each. Since two "
				     "slaves have these registers, the total should be 4*(2+5) = 28.");
				REQUIRE_THAT(
				    dynamic_cast<const ekdatatypes::DataPoint<double>*>(&**nvv)->getValue(),
				    Catch::Matchers::WithinRel(28.0, 0.00001));
			}
		}
		WHEN("I request a NewestValueView for error frequencies and force some errors")
		{
			ekdatatypes::ErrorStatisticType type
			    = ekdatatypes::ErrorStatisticType::FREQ_LINK_LOST_ERROR;
			unsigned int slaveId = std::numeric_limits<unsigned int>::max();
			ekdatatypes::ErrorStatistic& errorStat
			    = errorStatistician.getErrorStatistic(type, slaveId);
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> globalNvv
			    = errorStatistician.getNewest(errorStat);

			THEN("The views result in the expected values")
			{
				REQUIRE_FALSE(globalNvv->isEmpty());
				INFO("Since the mocked reader generates link lost errors at 100Hz, that should "
				     "be reported.");
				REQUIRE_THAT(
				    dynamic_cast<const ekdatatypes::DataPoint<double>*>(&**globalNvv)->getValue(),
				    Catch::Matchers::WithinRel(100.0, 0.05));
			}
		}
		WHEN("I limit the ErrorStatistician's memory severely")
		{
			errorStatistician.setMaximumMemory(1);
			THEN("Nothing crashes")
			{
				std::this_thread::sleep_for(100ms);
				SUCCEED();
			}
		}
	}
}

SCENARIO("ErrorStatisticInfos can handle illegal input", "[ErrorStatistician]")
{
	GIVEN("An ErrorStatisticInfo")
	{
		ekdatatypes::ErrorStatisticInfo esInfo = ekdatatypes::errorStatisticInfos.back();
		WHEN("I supply illegal input to getType()")
		{
			THEN("An exception is thrown")
			{
				REQUIRE_THROWS(
				    esInfo.getType(static_cast<ekdatatypes::ErrorStatisticCategory>(16)));
			}
		}
		WHEN("I supply illegal input to getTotalCategory()")
		{
			THEN("I get my input back")
			{
				REQUIRE(esInfo.getTotalCategory(ekdatatypes::ErrorStatisticCategory::TOTAL_GLOBAL)
				    == ekdatatypes::ErrorStatisticCategory::TOTAL_GLOBAL);
			}
		}
	}
}
