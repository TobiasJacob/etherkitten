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

#include "SlaveTree.hpp"
#include "util.hpp"
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>

namespace etherkitten::gui
{

	/* This is ugly, but we need some way of translating between
	 * the child IDs in the tree widget and our metadata */
	static const int esiIdx = 0;
	static const int coeIdx = 1;
	static const int pdoIdx = 2;
	static const int statIdx = 3;
	static const int regIdx = 4;

	SlaveTree::SlaveTree(QWidget* parent, DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo,
	    TooltipFormatter& tooltipFormatter)
	    : QFrame(parent)
	    , dataAdapter(dataAdapter)
	    , busInfo(busInfo)
	    , tooltipFormatter(tooltipFormatter)
	    , safeOP(false)
	    , busLive(false)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		QFrame* filterFrame = new QFrame(this);
		QHBoxLayout* filterLayout = new QHBoxLayout(filterFrame);
		filterText = new QLineEdit(filterFrame);
		QPushButton* filterButton = new QPushButton("Filter", filterFrame);
		filterButton->setToolTip("Filter the items in the tree by the filter text.");
		filterLayout->addWidget(filterText);
		filterLayout->addWidget(filterButton);
		filterText->installEventFilter(this);
		installEventFilter(this);
		connect(filterButton, &QPushButton::clicked, this, &SlaveTree::filter);

		treeWidget = new QTreeWidget(this);
		QTreeWidgetItem* header = treeWidget->headerItem();
		header->setText(0, "Name");
		header->setText(1, "Value");
		treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
		treeWidget->setColumnCount(2);
		layout->addWidget(filterFrame);
		layout->addWidget(treeWidget);
		treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(treeWidget, &QTreeWidget::customContextMenuRequested, this,
		    &SlaveTree::showContextMenu);
		connect(treeWidget, &QTreeWidget::itemExpanded, [this](QTreeWidgetItem* item) {
			updateData();
			(void)item;
		});

		resetButton = new QPushButton("Reset all error registers");
		resetButton->setToolTip("Reset the error registers of all slaves.");
		layout->addWidget(resetButton);
		connect(resetButton, &QPushButton::clicked, [this]() {
			try
			{
				this->dataAdapter.resetAllErrorRegisters();
			}
			catch (std::exception& e)
			{
				QMessageBox::warning(this, "Error",
				    "Error resetting registers:\n" + QString::fromStdString(e.what()));
			}
		});
		resetButton->setDisabled(true);
	}

	void SlaveTree::showEvent(QShowEvent* event)
	{
		/* force update so there is no lag when the tab is shown */
		updateData();
		QFrame::showEvent(event);
	}

	bool SlaveTree::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QKeyEvent::KeyPress)
		{
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (watched == filterText
			    && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
			{
				filter();
				return true;
			}
			else if (keyEvent->key() == Qt::Key_F && (keyEvent->modifiers() & Qt::ControlModifier))
			{
				filterText->setFocus();
				filterText->selectAll();
				return true;
			}
		}
		return QFrame::eventFilter(watched, event);
	}

	void SlaveTree::setBusLive(bool busLive)
	{
		this->busLive = busLive;
		resetButton->setEnabled(busLive);
		for (size_t i = 0; i < slaveMetadata.size(); ++i)
		{
			for (auto& coeEntry : slaveMetadata[i].coes)
			{
				for (auto& coe : coeEntry)
				{
					if (coe.entry != nullptr)
						coe.entry->setActive(busLive);
				}
			}
			for (auto& pdo : slaveMetadata[i].pdos)
			{
				if (pdo.entry != nullptr)
					pdo.entry->setActive(busLive);
			}
		}
	}

	void SlaveTree::updateData()
	{
		if (!this->isVisible())
			return;
		for (size_t i = 0; i < slaveMetadata.size(); i++)
		{
			SlaveMetadata& m = slaveMetadata[i];
			QTreeWidgetItem* slaveItem = treeWidget->topLevelItem(static_cast<int>(i));
			if (!slaveItem->isExpanded())
				continue;
			QTreeWidgetItem* pdoSection = slaveItem->child(pdoIdx);
			QTreeWidgetItem* coeSection = slaveItem->child(coeIdx);
			QTreeWidgetItem* statSection = slaveItem->child(statIdx);
			QTreeWidgetItem* regSection = slaveItem->child(regIdx);
			if (coeSection->isExpanded())
			{
				for (size_t j = 0; j < m.coes.size(); j++)
				{
					QTreeWidgetItem* coeEntry = coeSection->child(static_cast<int>(j));
					if (coeEntry->isExpanded())
					{
						for (size_t k = 0; k < m.coes[j].size(); k++)
						{
							Metadata<datatypes::CoEObject>& coeMeta = m.coes[j][k];
							QTreeWidgetItem* item = coeEntry->child(static_cast<int>(k));
							if (coeMeta.editable && coeMeta.entry == nullptr)
							{
								coeMeta.entry = new EditableDataEntry();
								coeMeta.entry->setActive(busLive);
								treeWidget->setItemWidget(item, 1, coeMeta.entry);
								connect(coeMeta.entry, &DataEntry::requestWrite, this,
								    [this, &coeMeta]() { writeData(coeMeta.obj, coeMeta.entry); });
							}
							if (item->isHidden() || coeMeta.view->isEmpty()
							    || (!coeMeta.dirty
							           && (**coeMeta.view).getTime() == coeMeta.lastTime))
								continue;
							coeMeta.lastTime = (**coeMeta.view).getTime();
							coeMeta.dirty = false;
							if (coeMeta.entry != nullptr)
							{
								if (!coeMeta.entry->editing())
								{
									coeMeta.entry->setValue(
									    (**coeMeta.view).asString(coeMeta.base));
								}
							}
							else
							{
								item->setText(1,
								    QString::fromStdString(
								        (**coeMeta.view).asString(coeMeta.base)));
							}
						}
					}
				}
			}
			if (pdoSection->isExpanded())
			{
				for (size_t j = 0; j < m.pdos.size(); j++)
				{
					Metadata<datatypes::PDO>& pdoMeta = m.pdos[j];
					QTreeWidgetItem* item = pdoSection->child(static_cast<int>(j));
					if (pdoMeta.editable && pdoMeta.entry == nullptr)
					{
						pdoMeta.entry = new EditableDataEntry();
						pdoMeta.entry->setActive(busLive);
						treeWidget->setItemWidget(item, 1, pdoMeta.entry);
						connect(pdoMeta.entry, &DataEntry::requestWrite, this,
						    [this, &pdoMeta]() { writeData(pdoMeta.obj, pdoMeta.entry); });
					}
					if (item->isHidden() || pdoMeta.view->isEmpty()
					    || (!pdoMeta.dirty && (**pdoMeta.view).getTime() == pdoMeta.lastTime))
						continue;
					pdoMeta.lastTime = (**pdoMeta.view).getTime();
					pdoMeta.dirty = false;
					if (pdoMeta.entry != nullptr)
					{
						if (!pdoMeta.entry->editing())
							pdoMeta.entry->setValue((**pdoMeta.view).asString(pdoMeta.base));
					}
					else
					{
						item->setText(
						    1, QString::fromStdString((**pdoMeta.view).asString(pdoMeta.base)));
					}
				}
			}
			if (statSection->isExpanded())
			{
				for (size_t j = 0; j < m.statistics.size(); j++)
				{
					Metadata<datatypes::ErrorStatistic>& statMeta = m.statistics[j];
					if (statMeta.view->isEmpty()
					    || (!statMeta.dirty && (**statMeta.view).getTime() == statMeta.lastTime))
						continue;
					QTreeWidgetItem* item = statSection->child(static_cast<int>(j));
					if (item->isHidden())
						continue;
					statMeta.lastTime = (**statMeta.view).getTime();
					statMeta.dirty = false;
					item->setText(
					    1, QString::fromStdString((**statMeta.view).asString(statMeta.base)));
				}
			}
			if (regSection->isExpanded())
			{
				for (size_t j = 0; j < m.regs.size(); j++)
				{
					Metadata<datatypes::Register>& regMeta = m.regs[j];
					if (regMeta.view->isEmpty()
					    || (!regMeta.dirty && (**regMeta.view).getTime() == regMeta.lastTime))
						continue;
					QTreeWidgetItem* item = regSection->child(static_cast<int>(j));
					if (item->isHidden())
						continue;
					regMeta.lastTime = (**regMeta.view).getTime();
					regMeta.dirty = false;
					item->setText(
					    1, QString::fromStdString((**regMeta.view).asString(regMeta.base)));
				}
			}
		}
	}

	void SlaveTree::writeData(const datatypes::DataObject& obj, DataEntry* entry)
	{
		if (!busLive)
			return;
		try
		{
			dataAdapter.writeData(obj, entry->getValue());
			entry->setEditing(false);
		}
		catch (std::exception& e)
		{
			QMessageBox::warning(
			    this, "Error", "Error writing data:\n" + QString::fromStdString(e.what()));
		}
	}

	void SlaveTree::clear()
	{
		treeWidget->clear();
		slaveMetadata.clear();
	}

	void SlaveTree::setupData(const std::unordered_map<datatypes::RegisterEnum, bool>& registers)
	{
		clear();
		resetButton->setEnabled(busLive);
		for (unsigned int i = 1; i <= busInfo.getSlaveCount(); i++)
		{
			const datatypes::SlaveInfo& slave = busInfo.getSlaveInfo(i);
			SlaveMetadata& m = slaveMetadata.emplace_back();
			std::optional<datatypes::ESIData> esiData = slave.getESI();
			if (esiData.has_value())
				m.esis = ESIAdapter::convertESIData(esiData.value());
			preparePDOs(slave.getPDOs(), m);
			prepareErrorStatistics(slave.getErrorStatistics(), m);
			prepareRegisters(slave.getRegisters(), m, registers);

			QTreeWidgetItem* slaveItem = new QTreeWidgetItem(treeWidget);
			std::string slaveName = std::to_string(slave.getID()) + " " + slave.getName();
			slaveItem->setText(0, QString::fromStdString(slaveName));
			QTreeWidgetItem* esiSection = new QTreeWidgetItem(slaveItem);
			esiSection->setText(0, "ESI");
			QTreeWidgetItem* coeSection = new QTreeWidgetItem(slaveItem);
			coeSection->setText(0, "CoE");
			QTreeWidgetItem* pdoSection = new QTreeWidgetItem(slaveItem);
			pdoSection->setText(0, "PDO");
			QTreeWidgetItem* statSection = new QTreeWidgetItem(slaveItem);
			statSection->setText(0, "Error Statistics");
			QTreeWidgetItem* regSection = new QTreeWidgetItem(slaveItem);
			regSection->setText(0, "Registers");

			for (auto& esi : m.esis)
			{
				QTreeWidgetItem* esiItem = new QTreeWidgetItem(esiSection);
				esiItem->setToolTip(0, esi->getTooltip());
				esiItem->setText(0, QString::fromStdString(esi.get()->getName()));
				esiItem->setText(1,
				    QString::fromStdString(esi.get()->asString(datatypes::NumberFormat::DECIMAL)));
			}

			for (auto& coeEntry : slave.getCoEs())
			{
				std::vector<Metadata<datatypes::CoEObject>>& vec = m.coes.emplace_back();
				prepareCoEs(coeEntry.getObjects(), vec);
				QTreeWidgetItem* entryRoot = new QTreeWidgetItem(coeSection);
				entryRoot->setText(0, QString::fromStdString(coeEntry.getName()));
				for (auto& coe : vec)
				{
					QTreeWidgetItem* coeItem = new QTreeWidgetItem(entryRoot);
					coeItem->setText(0, QString::fromStdString(coe.obj.get().getName()));
				}
			}

			for (auto& pdo : m.pdos)
			{
				QTreeWidgetItem* pdoItem = new QTreeWidgetItem(pdoSection);
				pdoItem->setText(0, QString::fromStdString(pdo.obj.get().getName()));
			}

			for (auto& stat : m.statistics)
			{
				stat.obj.get().acceptVisitor(tooltipFormatter);
				QTreeWidgetItem* statItem = new QTreeWidgetItem(statSection);
				statItem->setToolTip(0, tooltipFormatter.getTooltip());
				statItem->setText(0, QString::fromStdString(stat.obj.get().getName()));
			}

			for (auto& reg : m.regs)
			{
				reg.obj.get().acceptVisitor(tooltipFormatter);
				QTreeWidgetItem* regItem = new QTreeWidgetItem(regSection);
				regItem->setToolTip(0, tooltipFormatter.getTooltip());
				regItem->setText(0, QString::fromStdString(reg.obj.get().getName()));
			}
		}
		setBusLive(busLive);
	}

	void SlaveTree::prepareCoEs(const std::vector<datatypes::CoEObject>& coes,
	    std::vector<Metadata<datatypes::CoEObject>>& meta)
	{
		for (auto& coe : coes)
		{
			bool editable = false;
			if ((safeOP && coe.isWritableInSafeOp()) || (!safeOP && coe.isWritableInOp()))
				editable = true;
			meta.emplace_back(datatypes::NumberFormat::DECIMAL, datatypes::TimeStamp(), coe,
			    dataAdapter.getNewestValueView(coe), nullptr, editable, true);
		}
	}

	void SlaveTree::preparePDOs(const std::vector<datatypes::PDO>& pdos, SlaveMetadata& meta)
	{
		for (auto& pdo : pdos)
		{
			bool editable = false;
			if (pdo.getDirection() == datatypes::PDODirection::INPUT)
				editable = true;
			meta.pdos.emplace_back(datatypes::NumberFormat::DECIMAL, datatypes::TimeStamp(), pdo,
			    dataAdapter.getNewestValueView(pdo), nullptr, editable, true);
		}
	}

	void SlaveTree::prepareErrorStatistics(
	    const std::vector<datatypes::ErrorStatistic>& statistics, SlaveMetadata& meta)
	{
		for (auto& stat : statistics)
		{
			meta.statistics.emplace_back(datatypes::NumberFormat::DECIMAL, datatypes::TimeStamp(),
			    stat, dataAdapter.getNewestValueView(stat), nullptr, false, true);
		}
	}

	void SlaveTree::prepareRegisters(const std::vector<datatypes::Register>& regs,
	    SlaveMetadata& meta, const std::unordered_map<datatypes::RegisterEnum, bool>& registers)
	{
		for (auto& reg : regs)
		{
			auto iter = registers.find(reg.getRegister());
			if ((iter != registers.end()) && iter->second)
			{
				meta.regs.emplace_back(datatypes::NumberFormat::DECIMAL, datatypes::TimeStamp(),
				    reg, dataAdapter.getNewestValueView(reg), nullptr, false, true);
			}
		}
		std::sort(meta.regs.begin(), meta.regs.end(), RegCmp());
	}

	void SlaveTree::registerMenuGenerator(
	    std::function<void(QMenu* menu, const datatypes::DataObject& obj)> generator)
	{
		menuGenerators.emplace_back(generator);
	}

	void SlaveTree::rebuildRegisters(std::unordered_map<datatypes::RegisterEnum, bool>& registers)
	{
		/* Note: This resets all the number formats of the registers! */
		for (size_t i = 0; i < slaveMetadata.size() && i < busInfo.getSlaveCount(); i++)
		{
			slaveMetadata[i].regs.clear();
			prepareRegisters(busInfo.getSlaveInfo(static_cast<unsigned int>(i + 1)).getRegisters(),
			    slaveMetadata[i], registers);
			QTreeWidgetItem* slaveItem = treeWidget->topLevelItem(static_cast<int>(i));
			QTreeWidgetItem* regSection = slaveItem->child(regIdx);

			int num = static_cast<int>(slaveMetadata[i].regs.size());
			int curNum = regSection->childCount();
			if (num < curNum)
			{
				for (int j = curNum - 1; j >= num; j--)
				{
					QTreeWidgetItem* item = regSection->takeChild(j);
					delete item;
				}
			}
			else if (num > curNum)
			{
				for (int j = curNum; j < num; j++)
				{
					(void)new QTreeWidgetItem(regSection);
				}
			}

			for (size_t j = 0; j < slaveMetadata[i].regs.size(); j++)
			{
				Metadata<datatypes::Register>& reg = slaveMetadata[i].regs[j];
				reg.obj.get().acceptVisitor(tooltipFormatter);
				QTreeWidgetItem* regItem = regSection->child(static_cast<int>(j));
				regItem->setToolTip(0, tooltipFormatter.getTooltip());
				regItem->setText(0, QString::fromStdString(reg.obj.get().getName()));
				regItem->setText(1, "");
			}
		}
	}

	void SlaveTree::setSafeOP(bool newSafeOP)
	{
		safeOP = newSafeOP;
		for (size_t i = 0; i < slaveMetadata.size(); i++)
		{
			SlaveMetadata& m = slaveMetadata[i];
			QTreeWidgetItem* slaveItem = treeWidget->topLevelItem(static_cast<int>(i));
			QTreeWidgetItem* coeSection = slaveItem->child(coeIdx);
			for (size_t j = 0; j < m.coes.size(); j++)
			{
				QTreeWidgetItem* coeEntry = coeSection->child(static_cast<int>(j));
				for (size_t k = 0; k < m.coes[j].size(); k++)
				{
					QTreeWidgetItem* coeItem = coeEntry->child(static_cast<int>(k));
					Metadata<datatypes::CoEObject>& meta = m.coes[j][k];
					bool writable = (safeOP && meta.obj.get().isWritableInSafeOp())
					    || (!safeOP && meta.obj.get().isWritableInOp());
					if (writable && meta.entry == nullptr)
					{
						meta.editable = true;
						coeItem->setText(1, "");
						meta.dirty = true;
					}
					else if (!writable)
					{
						if (meta.entry != nullptr)
							treeWidget->removeItemWidget(coeItem, 1);
						meta.entry = nullptr;
						meta.editable = false;
						meta.dirty = true;
					}
				}
			}
		}
	}

	void SlaveTree::jumpToSlave(unsigned int slave)
	{
		if (slave > static_cast<std::size_t>(treeWidget->topLevelItemCount()))
			return;
		/* index is offset by one because the slaves are 1-indexed */
		QTreeWidgetItem* item = treeWidget->topLevelItem(static_cast<int>(slave - 1));
		treeWidget->scrollToItem(item);
		treeWidget->setCurrentItem(item);
		item->setExpanded(true);
	}

	void SlaveTree::generateNumberFormatMenu(
	    QMenu* menu, std::function<void(datatypes::NumberFormat f)> callback)
	{
		QMenu* formatMenu = new QMenu("Number format");
		QAction* dec = new QAction("Decimal");
		QAction* hex = new QAction("Hexadecimal");
		QAction* bin = new QAction("Binary");
		formatMenu->addAction(dec);
		formatMenu->addAction(hex);
		formatMenu->addAction(bin);
		connect(
		    dec, &QAction::triggered, [callback]() { callback(datatypes::NumberFormat::DECIMAL); });
		connect(hex, &QAction::triggered,
		    [callback]() { callback(datatypes::NumberFormat::HEXADECIMAL); });
		connect(
		    bin, &QAction::triggered, [callback]() { callback(datatypes::NumberFormat::BINARY); });
		menu->addMenu(formatMenu);
	}

	void SlaveTree::applyMenuGenerators(QMenu* menu, const datatypes::DataObject& obj)
	{
		for (auto fn : menuGenerators)
		{
			fn(menu, obj);
		}
	}

	void SlaveTree::showContextMenu(const QPoint& pos)
	{
		QTreeWidgetItem* item = treeWidget->itemAt(pos);
		QTreeWidgetItem* tmp = item;
		std::vector<int> indeces;
		do
		{
			if (tmp->parent() != nullptr)
				indeces.emplace_back(tmp->parent()->indexOfChild(tmp));
			else
				indeces.emplace_back(treeWidget->indexOfTopLevelItem(tmp));
		} while ((tmp = tmp->parent()) != nullptr);
		/* Only items at the bottom of the hierarchy currently have menus.
		 * Note: I don't check if item->childCount() > 0 in case one of the
		 * subcategories doesn't have any entries (for whatever reason) */
		if (indeces.size() < 3 || (indeces[indeces.size() - 2] == coeIdx && indeces.size() < 4))
			return;
		int slaveID = indeces[indeces.size() - 1];
		int sectionID = indeces[indeces.size() - 2];
		int dataID = indeces[0];
		if (dataID < 0 || slaveID < 0 || static_cast<std::size_t>(slaveID) >= slaveMetadata.size()
		    || sectionID < 0 || sectionID > regIdx)
			return; /* This should never happen */
		std::size_t dataIDx = static_cast<std::size_t>(dataID);
		SlaveMetadata& m = slaveMetadata[static_cast<std::size_t>(slaveID)];

		QMenu menu(this);
		if (sectionID == esiIdx)
		{
			if (dataIDx >= m.esis.size() || !m.esis[dataIDx].get()->isNumeric())
				return;
			generateNumberFormatMenu(
			    &menu, [data = m.esis[dataIDx].get(), item](datatypes::NumberFormat f) {
				    item->setText(1, QString::fromStdString(data->asString(f)));
			    });
		}
		else if (sectionID == coeIdx)
		{
			int entryID = indeces[1];
			if (entryID < 0 || static_cast<size_t>(entryID) >= m.coes.size())
				return;
			size_t entryIDx = static_cast<size_t>(indeces[1]);
			if (dataIDx >= m.coes[entryIDx].size())
				return;
			const datatypes::CoEObject& coe = m.coes[entryIDx][dataIDx].obj.get();
			auto& coeMeta = m.coes[entryIDx][dataIDx];
			applyMenuGenerators(&menu, coe);
			if (busLive
			    && ((safeOP && coe.isReadableInSafeOp()) || (!safeOP && coe.isReadableInOp())))
			{
				QAction* readAction = new QAction("Read CoE object");
				menu.addAction(readAction);
				QObject::connect(
				    readAction, &QAction::triggered, [this, slaveID, entryIDx, dataIDx]() {
					    SlaveMetadata& m = this->slaveMetadata[static_cast<std::size_t>(slaveID)];
					    auto& coeMeta = m.coes[entryIDx][dataIDx];
					    try
					    {
						    dataAdapter.readCoEObject(coeMeta.obj.get());
					    }
					    catch (std::exception& e)
					    {
						    QMessageBox::warning(this, "Error",
						        "Error reading CoE object:\n" + QString::fromStdString(e.what()));
					    }
					    if (coeMeta.entry && coeMeta.entry->editing())
						    coeMeta.entry->setEditing(false);
					    coeMeta.dirty = true;
				    });
			}
			if (dataObjectIsNumeric(coe))
			{
				generateNumberFormatMenu(&menu, [&coeMeta](datatypes::NumberFormat f) {
					coeMeta.base = f;
					if (coeMeta.entry && coeMeta.entry->editing())
						coeMeta.entry->setEditing(false);
					coeMeta.dirty = true;
				});
			}
		}
		else if (sectionID == pdoIdx)
		{
			if (dataIDx >= m.pdos.size())
				return;
			Metadata<datatypes::PDO>& pdoMeta = m.pdos[dataIDx];
			applyMenuGenerators(&menu, pdoMeta.obj.get());
			generateNumberFormatMenu(&menu, [&pdoMeta](datatypes::NumberFormat f) {
				if (pdoMeta.entry && pdoMeta.entry->editing())
					pdoMeta.entry->setEditing(false);
				pdoMeta.dirty = true;
				pdoMeta.base = f;
			});
		}
		else if (sectionID == statIdx)
		{
			if (dataIDx >= m.statistics.size())
				return;
			Metadata<datatypes::ErrorStatistic>& statMeta = m.statistics[dataIDx];
			applyMenuGenerators(&menu, statMeta.obj.get());
			if (busLive)
			{
				QAction* resetAction = new QAction("Reset error registers");
				menu.addAction(resetAction);
				connect(resetAction, &QAction::triggered, [this, slaveID]() {
					try
					{
						dataAdapter.resetErrorRegisters(static_cast<unsigned int>(slaveID));
					}
					catch (std::exception& e)
					{
						QMessageBox::warning(this, "Error",
						    "Error resetting registers:\n" + QString::fromStdString(e.what()));
					}
				});
			}
			generateNumberFormatMenu(&menu, [&statMeta](datatypes::NumberFormat f) {
				statMeta.dirty = true;
				statMeta.base = f;
			});
		}
		else if (sectionID == regIdx)
		{
			if (dataIDx >= m.regs.size())
				return;
			Metadata<datatypes::Register>& regMeta = m.regs[dataIDx];
			applyMenuGenerators(&menu, regMeta.obj.get());
			generateNumberFormatMenu(&menu, [&regMeta](datatypes::NumberFormat f) {
				regMeta.dirty = true;
				regMeta.base = f;
			});
		}
		menu.exec(treeWidget->mapToGlobal(pos));
		menu.clear();
	}

	void SlaveTree::setHiddenAll(bool hidden, QTreeWidgetItem* item)
	{
		item->setHidden(hidden);
		for (int i = 0; i < item->childCount(); i++)
		{
			setHiddenAll(hidden, item->child(i));
		}
	}

	void SlaveTree::unhideItem(QTreeWidgetItem* item)
	{
		do
		{
			item->setHidden(false);
		} while ((item = item->parent()));
	}

	void SlaveTree::filter()
	{
		QString text = filterText->text();
		bool defaultHidden = true;
		if (text.isEmpty())
		{
			defaultHidden = false;
		}

		for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
		{
			setHiddenAll(defaultHidden, treeWidget->topLevelItem(i));
		}
		if (text.isEmpty())
			return;
		for (auto& item :
		    treeWidget->findItems(filterText->text(), Qt::MatchContains | Qt::MatchRecursive, 0))
		{
			unhideItem(item);
		}
		for (auto& item :
		    treeWidget->findItems(filterText->text(), Qt::MatchContains | Qt::MatchRecursive, 1))
		{
			unhideItem(item);
		}
	}

} // namespace etherkitten::gui
