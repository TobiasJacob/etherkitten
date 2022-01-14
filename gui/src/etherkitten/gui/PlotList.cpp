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

#include "PlotList.hpp"
#include <iostream>

namespace etherkitten::gui
{

	PlotList::PlotList(QWidget* parent, BusInfoSupplier& busInfo, DataModelAdapter& dataAdapter)
	    : QFrame(parent)
	    , dataAdapter(dataAdapter)
	    , busInfo(busInfo)
	    , scrollScale(0)
	    , durationMoveExtra(0)
	    , maxDuration(0)
	    , maxMargin(0)
	{
		layout = new QVBoxLayout(this);
		QLabel* label
		    = new QLabel("Ctrl+Scroll: Zoom time axis; Scroll: Move time axis; Ctrl+Shift+Scroll: "
		                 "Zoom value axis; Shift+Scroll: Move value axis");
		label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
		label->setWordWrap(true);
		layout->addWidget(label);
		QFrame* btnFrame = new QFrame(this);
		QHBoxLayout* btnLayout = new QHBoxLayout(btnFrame);
		btnFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
		layout->addWidget(btnFrame);
		QPushButton* resetBtn = new QPushButton("Reset zoom");
		resetBtn->setToolTip("Reset the zoom so the full time axis is shown.");
		connect(resetBtn, &QPushButton::clicked, [this]() {
			for (auto plot : plots)
			{
				plot->resetValueAxis();
				plot->setOffset(0);
				plot->setShownDuration(maxDuration);
				plot->setFollowData(true);
				plot->refillData();
			}
			scrollBar->setHidden(true);
		});
		btnLayout->addWidget(resetBtn);
		QPushButton* zoomBtn = new QPushButton("Ensure all points are shown");
		zoomBtn->setToolTip(
		    "Zoom in far enough that all points are guaranteed to be shown. Note that this may "
		    "zoom in much further than would actually be needed to show all points.");
		connect(zoomBtn, &QPushButton::clicked, [this]() {
			if (plots.empty())
				return;
			long offset = plots[0]->getOffset();
			int width = plots[0]->xAxis->axisRect()->width();
			long oldDuration = plots[0]->getDuration();
			long duration = oldDuration;
			/* this ensures that the timeStep in the Plot will be 0,
			   causing every point to be shown */
			if (width / Plot::IDEAL_PIXELS_PER_POINT > 0)
				duration = width / Plot::IDEAL_PIXELS_PER_POINT - 1;
			if (duration >= oldDuration)
				return;
			for (auto plot : plots)
			{
				plot->setOffset(offset + (oldDuration - duration) / 2);
				plot->setShownDuration(duration);
				plot->setFollowData(false);
				plot->refillData();
			}
			scrollBar->setHidden(false);
		});
		btnLayout->addWidget(zoomBtn);

		scrollArea = new QScrollArea(this);
		plotsFrame = new QFrame(scrollArea);
		plotsLayout = new QVBoxLayout(plotsFrame);
		scrollArea->setWidget(plotsFrame);
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		scrollArea->setWidgetResizable(true);
		layout->addWidget(scrollArea);

		scrollBar = new QScrollBar(Qt::Orientation::Horizontal, this);
		scrollBar->setMinimum(0);
		scrollBar->setMaximum(SCROLL_POINTS);
		scrollBar->setPageStep(1);
		scrollBar->setHidden(true);
		connect(scrollBar, &QScrollBar::actionTriggered, [this](int action) {
			(void)action;
			int value = scrollBar->sliderPosition();
			long newOffset = static_cast<long>(value * scrollScale);
			for (auto plot : plots)
			{
				plot->setOffset(newOffset);
				if (value >= scrollBar->maximum())
					plot->setFollowData(true);
				else
					plot->setFollowData(false);
				plot->recalculate();
			}
		});
		layout->addWidget(scrollBar);
	}

	void PlotList::setScrollBar(long offset, long duration)
	{
		/* duration needs to be subtracted here because the actual
		 * scrollable area only goes to the left edge of the handle */
		scrollScale = static_cast<double>(maxDuration - duration) / SCROLL_POINTS;
		/* when this happens, the scrollbar should be hidden anyways */
		if (maxDuration - duration < 1)
			return;
		int scrollOffset = static_cast<int>(offset / scrollScale);
		int scrollDuration = static_cast<int>(duration / scrollScale);
		scrollBar->setPageStep(scrollDuration);
		scrollBar->setValue(scrollOffset);
	}

	void PlotList::updateData()
	{
		if (plots.empty())
			return;
		long oldMax = maxDuration;
		for (auto plot : plots)
		{
			plot->updateData();
			if (plot->getMaxLocalDuration() > maxDuration)
				maxDuration = plot->getMaxLocalDuration();
		}
		if (oldMax != maxDuration)
		{
			for (auto plot : plots)
			{
				plot->setMaxDuration(maxDuration);
			}
			setScrollBar(plots[0]->getOffset(), plots[0]->getDuration());
		}
	}

	void PlotList::clear()
	{
		for (auto plot : plots)
		{
			plotsLayout->removeWidget(plot);
			plot->deleteLater();
		}
		plots.clear();
		maxDuration = 0;
	}

	int PlotList::getNumPlots() const { return static_cast<int>(plots.size()); }

	void PlotList::addToPlot(int plot, const datatypes::DataObject& data)
	{
		if (plot < 0 || static_cast<size_t>(plot) >= plots.size())
			return;
		plots[static_cast<size_t>(plot)]->addData(data);

		/* check if the newest time for this DataObject is greater than the current
		 * maximum time and update plots accordingly */
		std::unique_ptr<datatypes::AbstractNewestValueView> newestView
		    = dataAdapter.getNewestValueView(data);
		if (newestView->isEmpty())
			return;
		long tmpDur = timeConverter.timeToMilli((**newestView).getTime());
		if (tmpDur > maxDuration)
		{
			maxDuration = tmpDur;
			for (auto plot : plots)
			{
				plot->setMaxDuration(maxDuration);
			}
		}
	}

	void PlotList::addPlot(const datatypes::DataObject& data)
	{
		/* if the plots are empty, the start time that everything else
		 * is relative to needs to be set first */
		if (plots.empty())
			timeConverter.setStart(busInfo.getStartTime());

		Plot* newPlot
		    = new Plot(plotsFrame, static_cast<int>(plots.size()), dataAdapter, timeConverter);

		if (!plots.empty())
		{
			newPlot->setFollowData(plots[0]->getFollowData());
			newPlot->setOffset(plots[0]->getOffset());
			newPlot->setShownDuration(plots[0]->getDuration());
		}
		newPlot->setMaxDuration(maxDuration);

		plots.emplace_back(newPlot);
		plotsLayout->addWidget(newPlot);
		connect(newPlot, &Plot::requestScaleX, this, &PlotList::scaleX);
		connect(newPlot, &Plot::requestMovePlot, this, &PlotList::movePlots);
		connect(newPlot, &Plot::requestRemovePlot, this, &PlotList::removePlot);
		connect(newPlot, &Plot::requestFollowData, this, &PlotList::setFollowData);
		connect(newPlot, &Plot::dragStarted, [this]() { durationMoveExtra = 0; });
		connect(newPlot, &Plot::dragStopped, [this]() { durationMoveExtra = 0; });
		connect(newPlot, &Plot::neededMarginChanged, [this]() {
			/* set the margins to align the left edge of the plots */
			maxMargin = 0;
			for (auto plot : plots)
			{
				if (plot->getNeededMargin() > maxMargin)
					maxMargin = plot->getNeededMargin();
			}
			for (auto plot : plots)
			{
				plot->setLeftMargin(maxMargin);
			}
		});
		addToPlot(static_cast<int>(plots.size()) - 1, data);
		newPlot->refillData();
	}

	void PlotList::setFollowData(bool followData)
	{
		for (auto plot : plots)
		{
			plot->setFollowData(followData);
		}
	}

	void PlotList::removePlot(int plotID)
	{
		if (plotID < 0 || static_cast<size_t>(plotID) >= plots.size())
			return;
		plots[static_cast<size_t>(plotID)]->deleteLater();
		plots.erase(plots.begin() + plotID);
		for (size_t i = static_cast<size_t>(plotID); i < plots.size(); i++)
		{
			plots[i]->setID(static_cast<int>(i));
		}
		maxDuration = 0;
		for (auto plot : plots)
		{
			if (plot->getMaxLocalDuration() > maxDuration)
				maxDuration = plot->getMaxLocalDuration();
		}
		for (auto plot : plots)
		{
			plot->setMaxDuration(maxDuration);
		}
	}

	void PlotList::movePlots(int delta)
	{
		/* Note: the duration should be synchronized between all plots anyways,
		 * so this should work to get the duration used for all of them */
		long duration = plots[0]->getDuration();
		int width = plots[0]->xAxis->axisRect()->width();
		double realDurationMove
		    = (delta * duration) / static_cast<double>(width) + durationMoveExtra;
		long durationMove = static_cast<long>(realDurationMove);
		durationMoveExtra = realDurationMove - durationMove;
		long oldOffset = plots[0]->getOffset();
		/* if all data is shown, nothing needs to be done */
		if (oldOffset == 0 && duration == maxDuration)
			return;
		long newOffset = oldOffset + durationMove;
		if (newOffset < 0)
			newOffset = 0;
		if (newOffset + duration >= maxDuration)
		{
			newOffset = maxDuration - duration;
			setFollowData(true);
			if (newOffset < 0)
				newOffset = 0;
			for (auto plot : plots)
			{
				plot->setOffset(newOffset);
				plot->setMaxDuration(maxDuration);
			}
		}
		else
		{
			setFollowData(false);
			for (auto plot : plots)
			{
				plot->setOffset(newOffset);
				plot->recalculate();
			}
		}
		setScrollBar(newOffset, plots[0]->getDuration());
	}

	void PlotList::scaleX(int angleDelta, int mouseX)
	{
		long duration = plots[0]->getDuration();
		long newDuration = duration - static_cast<long>((angleDelta / 360.0) * duration);
		/* Make sure it doesn't get too small */
		if (newDuration < 10)
			newDuration = 10 < maxDuration ? 10 : maxDuration;
		/* attempt to scroll towards the point under the mouse */
		double moveScale = static_cast<double>(mouseX - plots[0]->yAxis->axisRect()->left())
		    / plots[0]->xAxis->axisRect()->width();
		double realOffset
		    = plots[0]->getOffset() + (duration - newDuration) * moveScale + durationMoveExtra;
		long offset = static_cast<long>(realOffset);
		durationMoveExtra = realOffset - offset;
		if (offset < 0)
			offset = 0;
		if (offset + newDuration > maxDuration)
		{
			offset = maxDuration - newDuration;
			setFollowData(true);
			if (offset < 0)
			{
				offset = 0;
				newDuration = maxDuration;
				scrollBar->setHidden(true);
			}
			else
			{
				scrollBar->setHidden(false);
			}
			for (auto plot : plots)
			{
				plot->setOffset(offset);
				plot->setShownDuration(newDuration);
				plot->setMaxDuration(maxDuration);
			}
		}
		else
		{
			scrollBar->setHidden(false);
			setFollowData(false);
			for (auto plot : plots)
			{
				plot->setOffset(offset);
				plot->setShownDuration(newDuration);
				plot->recalculate();
			}
		}
		setScrollBar(offset, newDuration);
	}

} // namespace etherkitten::gui
