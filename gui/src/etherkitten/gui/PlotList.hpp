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
#include "Plot.hpp"
#include <QFrame>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <etherkitten/datatypes/dataobjects.hpp>

namespace etherkitten::gui
{

	/*!
	 * \brief Holds and manages multiple Plots.
	 */
	class PlotList : public QFrame
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new Plotlist.
		 * \param parent The parent widget.
		 * \param dataAdapter The adapter to use for obtaining the data.
		 */
		PlotList(QWidget* parent, BusInfoSupplier& busInfo, DataModelAdapter& dataAdapter);
		/*!
		 * \brief Update the data in the plots.
		 */
		void updateData();
		/*!
		 * \brief Remove all plots.
		 */
		void clear();
		/*!
		 * \brief Return the number of plots.
		 * \return The number of plots.
		 */
		int getNumPlots() const;

	public slots:
		/*!
		 * \brief Add data to a plot.
		 * \param plot The plot to add the data to.
		 * \param data The data to add to the plot.
		 */
		void addToPlot(int plot, const datatypes::DataObject& data);
		/*!
		 * \brief Add a plot and initialize it with the given data.
		 * \param data The data to initialize the plot with.
		 */
		void addPlot(const datatypes::DataObject& data);

	private slots:
		/*!
		 * \brief Remove a plot.
		 * \param plotID The id of the plot to remove.
		 */
		void removePlot(int plotID);
		/*!
		 * \brief Move the plots.
		 * \param delta The delta from the mouse drag.
		 */
		void movePlots(int delta);
		/*!
		 * \brief Scale the time axis.
		 * \param angleDelta The delta of the mouse wheel.
		 * \param mouseX The x position of the mouse.
		 */
		void scaleX(int angleDelta, int mouseX);
		/*!
		 * \brief Set whether the plots should follow the data on the right.
		 * \param followData Whether the plots should follow the data on the right.
		 */
		void setFollowData(bool followData);

	private:
		/*!
		 * \brief Set the offset and duration of the scrollbar used to scroll the
		 * plots. The offset and duration are used to calculate the position and
		 * size of the scrollbar handle.
		 * \param offset
		 * \param duration
		 */
		void setScrollBar(long offset, long duration);

		/*!
		 * \brief The number of steps in the scrollbar used to scroll the plots.
		 * This is used to scale down the raw milliseconds used elsewhere
		 * because the QScrollBar only supports int instead of long as its
		 * value, so the milliseconds could overflow when stored as ints.
		 * This program will probably never run on a system where an int is not
		 * at least 32 bits long, but it's still good to be careful.
		 */
		static const int SCROLL_POINTS = 32000;
		DataModelAdapter& dataAdapter;
		BusInfoSupplier& busInfo;
		std::vector<Plot*> plots;
		TimeStampConverter timeConverter;
		QFrame* plotsFrame;
		QVBoxLayout* layout;
		QVBoxLayout* plotsLayout;
		QScrollArea* scrollArea;
		QScrollBar* scrollBar;
		double scrollScale;
		double durationMoveExtra; /* the extra move (in milliseconds) that has to be added the next
		                          time the plots are moved or zoomed - when zoomed in, the
		                          millisecond resolution is not enough for the mouse dragging (this
		                          is also used for zooming because the plots attempt to zoom towards
		                          the mouse pointer, which becomes very inaccurate when zoomed in
		                          far enough for the same reason as the mouse dragging) */
		long maxDuration; /* the maximum time in milliseconds (the far right of the plots) */
		int maxMargin; /* the maximum left margin (from left of widget to actual plot) needed by any
		                  plot */
	};

} // namespace etherkitten::gui
