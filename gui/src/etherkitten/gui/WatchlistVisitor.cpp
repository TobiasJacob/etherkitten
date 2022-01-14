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

#include "WatchlistVisitor.hpp"
#include <QMessageBox>
#include <stdexcept>

namespace etherkitten::gui
{

	WatchlistVisitor::WatchlistVisitor(DataModelAdapter& adapter)
	    : safeOP(false)
	    , adapter(adapter)
	{
	}

	WatchlistVisitor::~WatchlistVisitor() {}

	void WatchlistVisitor::setSafeOP(bool safeOP) { this->safeOP = safeOP; }

	DataEntry* WatchlistVisitor::getWidget() const { return entry; }

	std::function<void(QMenu* menu, const datatypes::DataObject& obj, bool busLive)>
	WatchlistVisitor::getMenuGenerator() const
	{
		return generator;
	}

	void WatchlistVisitor::handlePDO(const datatypes::PDO& object)
	{
		entry = nullptr;
		if (object.getDirection() == datatypes::PDODirection::INPUT)
		{
			entry = new EditableDataEntry();
		}
		generator = [](QMenu* menu, const datatypes::DataObject& obj, bool busLive) {
			(void)menu;
			(void)obj;
			(void)busLive;
		};
	}

	void WatchlistVisitor::handleCoE(const datatypes::CoEObject& object)
	{
		entry = nullptr;
		if ((safeOP && object.isWritableInSafeOp()) || (!safeOP && object.isWritableInOp()))
		{
			entry = new EditableDataEntry();
		}
		if ((safeOP && object.isReadableInSafeOp()) || (!safeOP && object.isReadableInOp()))
		{
			generator
			    = [this, &object](QMenu* menu, const datatypes::DataObject& obj, bool busLive) {
				      if (!busLive)
					      return;
				      QAction* readAction = new QAction("Read CoE object");
				      menu->addAction(readAction);
				      QObject::connect(readAction, &QAction::triggered, [this, &object]() {
					      try
					      {
						      adapter.readCoEObject(object);
					      }
					      catch (std::exception& e)
					      {
						      /* Note: This *might* look weird on some platforms because it has no
						       * parent and thus may show up wherever it wants to */
						      QMessageBox::warning(nullptr, "Error",
						          "Error reading CoE object:\n" + QString::fromStdString(e.what()));
					      }
				      });
				      (void)obj;
			      };
		}
		else
		{
			generator = [](QMenu* menu, const datatypes::DataObject& obj, bool busLive) {
				(void)menu;
				(void)obj;
				(void)busLive;
			};
		}
	}

	void WatchlistVisitor::handleErrorStatistic(const datatypes::ErrorStatistic& statistic)
	{
		(void)statistic;
		entry = nullptr;
		generator = [](QMenu* menu, const datatypes::DataObject& obj, bool busLive) {
			(void)menu;
			(void)obj;
			(void)busLive;
		};
	}

	void WatchlistVisitor::handleRegister(const datatypes::Register& reg)
	{
		(void)reg;
		entry = nullptr;
		generator = [](QMenu* menu, const datatypes::DataObject& obj, bool busLive) {
			(void)menu;
			(void)obj;
			(void)busLive;
		};
	}

} // namespace etherkitten::gui
