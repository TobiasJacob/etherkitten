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
#include "DataModelAdapter.hpp"
#include "Edge.hpp"
#include "Node.hpp"
#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include <etherkitten/config/BusLayout.hpp>
#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace etherkitten::gui
{

	/*!
	 * \brief Displays all slaves and the connections between them, along with
	 * certain types of errors.
	 */
	class SlaveGraph : public QFrame
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new SlaveGraph.
		 * \param parent The parent widget.
		 * \param busInfo The BusInfoSupplier used to obtain information about the slaves.
		 * \param busLayout The BusLayout that is used to read and write the slave positions.
		 */
		SlaveGraph(QWidget* parent, DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo,
		    config::BusLayout& busLayout);

		/*!
		 * \brief Update the data shown, i.e. check if there are new errors to show.
		 */
		void updateData();

		/*!
		 * \brief Setup the graph using information obtained from the BusInfoSupplier.
		 * Note that this does not respect whatever positions are stored in the BusLayout. In
		 * order to set those positions, applyBusLayout() must be used.
		 */
		void setupData();

		/*!
		 * \brief Clear all shown data.
		 */
		void clear();

		/*!
		 * \brief Set the positions of all slaves to the ones stored in the BusLayout.
		 * Note that this will produce weird results when there are more slaves than were
		 * saved in the BusLayout.
		 */
		void applyBusLayout();

		/*!
		 * \brief Save the current positions of the slaves in the BusLayout.
		 */
		void writeBusLayout();

		/*!
		 * \brief Set the new BusLayout to use. Note that this does not set the positions
		 * of the slaves, use applyBusLayout() for that.
		 * \param newBusLayout The new BusLayout.
		 */
		void setBusLayout(config::BusLayout& newBusLayout);

	public slots:
		/*!
		 * \brief Remove all errors shown on the graph.
		 */
		void clearErrors();

	signals:
		/*!
		 * \brief Signal that a slave was double-clicked.
		 * \param slave The slave that was double-clicked.
		 */
		void slaveClicked(unsigned int slave);

	private:
		/*!
		 * \brief Handle an error message.
		 * \param msg The error message to handle;
		 */
		void handleErrorMessage(const datatypes::ErrorMessage&& msg);
		bool eventFilter(QObject* object, QEvent* event) override;

		struct NodeMetadata
		{
			NodeMetadata(std::string baseTooltip,
			    std::unique_ptr<datatypes::AbstractNewestValueView> view,
			    std::vector<std::shared_ptr<datatypes::AbstractDataView>> statViews, Node* node,
			    bool lowFreqError, bool highFreqError)
			    : baseTooltip(baseTooltip)
			    , view(std::move(view))
			    , statViews(std::move(statViews))
			    , node(node)
			    , lowFreqError(lowFreqError)
			    , highFreqError(highFreqError)
			{
			}
			NodeMetadata(NodeMetadata&& other)
			    : baseTooltip(other.baseTooltip)
			    , view(std::move(other.view))
			    , statViews(std::move(other.statViews))
			    , node(other.node)
			    , lowFreqError(other.lowFreqError)
			    , highFreqError(other.highFreqError)
			{
			}
			std::string baseTooltip;
			std::unique_ptr<datatypes::AbstractNewestValueView> view;
			std::vector<std::shared_ptr<datatypes::AbstractDataView>> statViews;
			Node* node;
			/* used to save whether frequency errors are currently shown on the
			 * node so they aren't set again if they haven't changed as a small
			 * efficiency improvement */
			bool lowFreqError;
			bool highFreqError;
			NodeMetadata& operator=(NodeMetadata&& other)
			{
				baseTooltip = other.baseTooltip;
				view = std::move(other.view);
				statViews = std::move(other.statViews);
				node = other.node;
				lowFreqError = other.lowFreqError;
				highFreqError = other.highFreqError;
				return *this;
			}
		};

		static constexpr double lowErrorFreq = 0.0001; /* when combined error frequency is >= this,
		                                                  a low error is shown for the slave */
		static constexpr double highErrorFreq = 100; /* when combined error frequency is >= this, a
		                                                high error is shown for the slave */
		BusInfoSupplier& busInfo;
		DataModelAdapter& dataAdapter;
		QGraphicsScene* scene;
		QGraphicsView* view;
		std::vector<NodeMetadata> nodes;
		std::shared_ptr<datatypes::ErrorIterator> errorLog;
		std::reference_wrapper<config::BusLayout> busLayout;
		QPointF targetScenePos, targetViewportPos; /* for zooming */
	};

} // namespace etherkitten::gui
