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

#include "BusInfoSupplier.hpp"
#include "DataEntry.hpp"
#include "DataModelAdapter.hpp"
#include "TooltipFormatter.hpp"
#include "esi.hpp"
#include "util.hpp"
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace etherkitten::gui
{

	/*!
	 * \brief Shows all slave information in a tree.
	 */
	class SlaveTree : public QFrame
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Constructs a new SlaveTree.
		 * \param parent The parent widget.
		 * \param dataAdapter The DataModelAdapter to use for obtaining data to display.
		 * \param tooltipFormatter The TooltipFormatter to use for creating tooltips.
		 */
		SlaveTree(QWidget* parent, DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo,
		    TooltipFormatter& tooltipFormatter);
		/*!
		 * \brief Update all shown data.
		 */
		void updateData();
		/*!
		 * \brief Setup the data shown in the tree, which is obtained through
		 * the BusInfoSupplier. This automatically clears the tree beforehand
		 * and sets it to active.
		 * \param registers A map specifying which registers should be shown.
		 */
		void setupData(const std::unordered_map<datatypes::RegisterEnum, bool>& registers);
		/*!
		 * \brief Register a function that will be called when a context menu is generated.
		 * \param generator The function to call.
		 */
		void registerMenuGenerator(
		    std::function<void(QMenu* menu, const datatypes::DataObject& obj)> generator);
		/*!
		 * \brief Rebuild the registers for all slaves. This has to be called when the registers
		 * to read are changed.
		 * \param registers A map specifying which registers should be shown.
		 */
		void rebuildRegisters(std::unordered_map<datatypes::RegisterEnum, bool>& registers);
		/*!
		 * \brief Set the SafeOp mode. This needs to be called so the CoE items can be adjusted
		 * depending on whether they are writable in the current mode. Note that the SlaveTree
		 * can obtain this information itself because it has access to the BusInfoSupplier, but
		 * this method should still be called by the GUIController to keep it more consistent
		 * with other components such as the WatchList.
		 * \param newSafeOP true if the bus is in Safe-Op, false if it is in Op.
		 */
		void setSafeOP(bool newSafeOP);
		/*!
		 * \brief Set whether the bus is live (connected to an interface) or not. Note that the
		 * SlaveTree can obtain this information itself because it has access to the
		 * BusInfoSupplier, but this method should still be called by the GUIController to keep it
		 * more consistent with other components such as the WatchList. \param busLive whether the
		 * bus is live or not.
		 */
		void setBusLive(bool busLive);

		/*!
		 * \brief Clear all shown data.
		 */
		void clear();

	protected:
		bool eventFilter(QObject* watched, QEvent* event) override;
		void showEvent(QShowEvent* event) override;

	public slots:
		/*!
		 * \brief Jump to the given slave in the tree.
		 * \param slave The slave to jump to.
		 */
		void jumpToSlave(unsigned int slave);

	signals:
		/*!
		 * \brief Signal that an error has occurred.
		 * \param msg The error message.
		 */
		void errorMessage(std::string msg);

	private slots:
		/*!
		 * \brief Show a context menu for the object at the given position.
		 * \param pos The position of the menu.
		 */
		void showContextMenu(const QPoint& pos);
		/*!
		 * \brief Filter the items in the tree based on a search term the user has typed.
		 */
		void filter();

	private:
		template<typename T>
		struct Metadata
		{
			Metadata(datatypes::NumberFormat base, datatypes::TimeStamp lastTime, const T& obj,
			    std::unique_ptr<datatypes::AbstractNewestValueView> view, DataEntry* entry,
			    bool editable, bool dirty)
			    : base(base)
			    , lastTime(lastTime)
			    , obj(obj)
			    , view(std::move(view))
			    , entry(entry)
			    , editable(editable)
			    , dirty(dirty)
			{
			}
			Metadata(Metadata&& other)
			    : base(other.base)
			    , lastTime(other.lastTime)
			    , obj(other.obj)
			    , view(std::move(other.view))
			    , entry(other.entry)
			    , editable(other.editable)
			    , dirty(other.dirty)
			{
			}
			datatypes::NumberFormat base;
			datatypes::TimeStamp lastTime;
			std::reference_wrapper<const T> obj;
			std::unique_ptr<datatypes::AbstractNewestValueView> view;
			DataEntry* entry;
			bool editable;
			bool dirty;
			Metadata& operator=(Metadata&& other)
			{
				base = other.base;
				lastTime = other.lastTime;
				obj = other.obj;
				view = std::move(other.view);
				entry = other.entry;
				editable = other.editable;
				dirty = other.dirty;
				return *this;
			}
		};

		struct SlaveMetadata
		{
			std::vector<std::vector<Metadata<datatypes::CoEObject>>> coes;
			std::vector<Metadata<datatypes::PDO>> pdos;
			std::vector<Metadata<datatypes::ErrorStatistic>> statistics;
			std::vector<Metadata<datatypes::Register>> regs;
			std::vector<std::unique_ptr<AbstractESIDataWrapper>> esis;
		};
		struct RegCmp
		{
			bool operator()(
			    const Metadata<datatypes::Register>& a, const Metadata<datatypes::Register>& b)
			{
				return compareRegisters(a.obj.get().getRegister(), b.obj.get().getRegister());
			}
		};
		QTreeWidget* treeWidget;
		QLineEdit* filterText;
		QPushButton* resetButton;
		DataModelAdapter& dataAdapter;
		BusInfoSupplier& busInfo;
		TooltipFormatter& tooltipFormatter;
		std::vector<std::function<void(QMenu* menu, const datatypes::DataObject& obj)>>
		    menuGenerators;
		std::vector<SlaveMetadata> slaveMetadata;
		bool safeOP;
		bool busLive;

		/*!
		 * \brief Recursively set the hidden state of item and all its children.
		 * \param hidden The hidden state.
		 * \param item The item to set the hidden state of, along with its children.
		 */
		void setHiddenAll(bool hidden, QTreeWidgetItem* item);
		/*!
		 * \brief Unhide the given item and all its parents.
		 * \param item The item to unhide.
		 */
		void unhideItem(QTreeWidgetItem* item);
		/*!
		 * \brief Set up the internal data structures for the CoE objects of one CoEEntry.
		 * \param coes The CoE objects to add.
		 * \param meta The vector to add the CoE objects to.
		 */
		void prepareCoEs(const std::vector<datatypes::CoEObject>& coes,
		    std::vector<Metadata<datatypes::CoEObject>>& meta);
		/*!
		 * \brief Set up the internal data structures for the PDOs.
		 * \param pdos The PDOs to add.
		 * \param meta The slave metadata that the PDOs should be added to.
		 */
		void preparePDOs(const std::vector<datatypes::PDO>& pdos, SlaveMetadata& meta);
		/*!
		 * \brief Set up the internal data structures for the error statistics.
		 * \param statistics The error statistics to add.
		 * \param meta The slave metadata that the statistics should be added to.
		 */
		void prepareErrorStatistics(
		    const std::vector<datatypes::ErrorStatistic>& statistics, SlaveMetadata& meta);
		/*!
		 * \brief Set up the internal data structures for the registers.
		 * \param regs The possible registers.
		 * \param meta The slave metadata that the registers should be added to.
		 * \param registers A mapping to determine which of the registers should be added.
		 */
		void prepareRegisters(const std::vector<datatypes::Register>& regs, SlaveMetadata& meta,
		    const std::unordered_map<datatypes::RegisterEnum, bool>& registers);
		/*!
		 * \brief Generate a number format menu (decimal, etc.).
		 * \param menu The menu to add the actions to.
		 * \param callback A callback to call with the chosen number format.
		 */
		void generateNumberFormatMenu(
		    QMenu* menu, std::function<void(datatypes::NumberFormat f)> callback);
		/*!
		 * \brief Apply the registered menu generators.
		 * \param menu The menu to add the actions to.
		 * \param obj The DataObject for which to generate the menu actions.
		 */
		void applyMenuGenerators(QMenu* menu, const datatypes::DataObject& obj);
		/*!
		 * \brief Tell the backend to write data.
		 * \param obj The DataObject to identify the data.
		 * \param entry The DataEntry containing the text to write.
		 */
		void writeData(const datatypes::DataObject& obj, DataEntry* entry);
	};

} // namespace etherkitten::gui
