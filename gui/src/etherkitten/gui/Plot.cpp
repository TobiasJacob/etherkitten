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

#include "Plot.hpp"
#include <QString>
#include <chrono>

namespace etherkitten::gui
{

	Plot::Plot(
	    QWidget* parent, int id, DataModelAdapter& adapter, TimeStampConverter& timeConverter)
	    : QCustomPlot(parent)
	    , adapter(adapter)
	    , timeConverter(timeConverter)
	    , timeTicker(new QCPAxisTickerTime)
	    , cacheStart(0)
	    , cacheEnd(0)
	    , offset(0)
	    , duration(1)
	    , timeStep(0)
	    , maxLocalDuration(0)
	    , maxValue(1)
	    , minValue(0)
	    , padding(0)
	    , neededMargin(0)
	    , followData(true)
	    , valueAxisScaled(false)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &Plot::customContextMenuRequested, this, &Plot::showContextMenu);
		setMinimumHeight(250);
		timeTicker->setTimeFormat("%h:%m:%s.%z");
		xAxis->setTicker(timeTicker);
		pens.emplace_back(QPen(QColor(40, 110, 40)));
		pens.emplace_back(QPen(QColor(40, 110, 255)));
		pens.emplace_back(QPen(QColor(255, 0, 0)));
		pens.emplace_back(QPen(QColor(255, 200, 0)));
		pens.emplace_back(QPen(QColor(102, 51, 0)));
		pens.emplace_back(QPen(QColor(255, 0, 102)));
		pens.emplace_back(QPen(QColor(153, 0, 153)));
		pens.emplace_back(QPen(QColor(0, 255, 255)));
		pens.emplace_back(QPen(QColor(40, 40, 40)));
		pens.emplace_back(QPen(QColor(255, 110, 40)));

		/* set legend beneath plot */
		QCPLayoutGrid* subLayout = new QCPLayoutGrid();
		plotLayout()->addElement(1, 0, subLayout);
		subLayout->setMargins(QMargins(5, 0, 5, 5));
		subLayout->addElement(0, 0, legend);
		legend->setFillOrder(QCPLegend::foColumnsFirst);
		plotLayout()->setRowStretchFactor(1, 0.001);
		legend->setVisible(true);

		idLabel = new QCPTextElement(this);
		plotLayout()->insertRow(0);
		plotLayout()->addElement(0, 0, idLabel);
		setID(id);
		/* remove the unnecessary margin below the text label */
		axisRect()->setAutoMargins(
		    QCP::MarginSide::msLeft | QCP::MarginSide::msRight | QCP::MarginSide::msBottom);
		axisRect()->setMargins(QMargins(0, 0, 0, 0));
	}

	void Plot::updateData()
	{
		/* The NewestValueViews are used separately to get the newest possible
		 * time because the scrollbar at the bottom of the PlotList needs to be
		 * updated even if the data is not being followed, so the DataViews
		 * cannot be used for this task. */
		for (Metadata& m : dataList)
		{
			if (m.newest->isEmpty())
				continue;
			long time = timeConverter.timeToMilli((**m.newest).getTime());
			if (time > maxLocalDuration)
				maxLocalDuration = time;
		}
		if (!followData || graphCount() == 0)
			return;
		long tmp = 0;
		for (size_t i = 0; i < dataList.size(); i++)
		{
			std::shared_ptr<datatypes::AbstractDataView> view = dataList[i].view;
			if (view->isEmpty())
				continue;
			auto d = graph(static_cast<int>(i))->data();
			QCPGraphData gd;
			while (view->hasNext())
			{
				++(*view);
				tmp = timeConverter.timeToMilli(view->getTime());
				if (tmp < dataList[i].lastTime + timeStep)
					continue;
				dataList[i].lastTime = tmp;
				gd.key = static_cast<double>(tmp) / 1000;
				gd.value = view->asDouble();
				d->add(gd);
				if (gd.value > maxValue)
					maxValue = gd.value;
				if (gd.value < minValue)
					minValue = gd.value;
			}
		}
		cacheEnd = maxLocalDuration;
	}

	bool Plot::getFollowData() const { return followData; }

	void Plot::setFollowData(bool newFollowData) { followData = newFollowData; }

	long Plot::getMaxLocalDuration() const { return maxLocalDuration; }

	void Plot::addData(const datatypes::DataObject& data)
	{
		/* This is a bit ugly, but the view has to be set to *something* */
		std::shared_ptr<datatypes::AbstractDataView> view
		    = adapter.getDataView(data, timeConverter.milliToTimeSeries(0, timeStep));
		std::unique_ptr<datatypes::AbstractNewestValueView> newest
		    = adapter.getNewestValueView(data);
		dataList.push_back({ data, view, std::move(newest), 0 });
		addGraph();
		graph(graphCount() - 1)->setPen(pens[dataList.size() % pens.size()]);
		graph(graphCount() - 1)
		    ->setName(QString("Slave %1: %2")
		                  .arg(QString::number(data.getSlaveID()),
		                      QString::fromStdString(data.getName())));
		refillData();
	}

	void Plot::setOffset(long time) { offset = time; }

	void Plot::setShownDuration(long newDuration) { duration = newDuration; }

	void Plot::refillData()
	{
		if (graphCount() == 0)
			return;
		if (xAxis->axisRect()->width() / IDEAL_PIXELS_PER_POINT > 0)
			timeStep = duration / (xAxis->axisRect()->width() / IDEAL_PIXELS_PER_POINT);
		else
			timeStep = 0;
		/* hack to make sure the cache actually has anything in it if timeStep
		 * becomes zero */
		long tmpTimeStep = timeStep > 0 ? timeStep : 1;
		if (offset > tmpTimeStep * static_cast<long>(MAX_CACHE_POINTS))
			cacheStart = offset - tmpTimeStep * static_cast<long>(MAX_CACHE_POINTS);
		else
			cacheStart = 0;
		cacheEnd = offset + duration + tmpTimeStep * static_cast<long>(MAX_CACHE_POINTS);
		double cacheEndSeconds = cacheEnd / 1000.0;
		long tmp = 0;
		for (size_t i = 0; i < dataList.size(); i++)
		{
			auto d = graph(static_cast<int>(i))->data();
			/* This could potentially be made more efficient by calling setAutoSqueeze(false)
			 * on the internal data for the graphs, but that has a big warning written all
			 * over it in the documentation and it seems to be efficient enough at the
			 * moment, so that shouldn't be a problem. */
			d->clear();
			const datatypes::DataObject& obj = dataList[i].data;
			std::shared_ptr<datatypes::AbstractDataView> view
			    = adapter.getDataView(obj, timeConverter.milliToTimeSeries(cacheStart, timeStep));
			dataList[i].view = view;
			if (view->isEmpty())
				continue;
			tmp = timeConverter.timeToMilli(view->getTime());
			QCPGraphData gd(static_cast<double>(tmp) / 1000, view->asDouble());
			d->add(gd);
			while (view->hasNext() && gd.key < cacheEndSeconds)
			{
				++(*view);
				tmp = timeConverter.timeToMilli(view->getTime());
				gd.key = static_cast<double>(tmp) / 1000;
				gd.value = view->asDouble();
				d->add(gd);
				if (gd.value > maxValue)
					maxValue = gd.value;
				if (gd.value < minValue)
					minValue = gd.value;
			}
			dataList[i].lastTime = tmp;
			if (tmp > maxLocalDuration)
				maxLocalDuration = tmp;
		}
		if (cacheEnd > maxLocalDuration)
			cacheEnd = maxLocalDuration;
		xAxis->setRange(
		    static_cast<double>(offset) / 1000, static_cast<double>(offset + duration) / 1000);
		if (!valueAxisScaled)
		{
			yAxis->setRange(minValue - (maxValue - minValue) / VALUE_EXTRA,
			    maxValue + (maxValue - minValue) / VALUE_EXTRA);
		}
		if (isVisible())
			replotAndCalcMargin();
	}

	void Plot::replotAndCalcMargin()
	{
		replot();
		int newNeededMargin = yAxis->axisRect()->left() - padding;
		if (newNeededMargin != neededMargin)
		{
			neededMargin = newNeededMargin;
			emit neededMarginChanged();
		}
	}

	int Plot::getNeededMargin() const { return neededMargin; }

	void Plot::recalculate()
	{
		if (graphCount() == 0)
			return;
		int pixelsPerItem = 0;
		if (timeStep > 0 && (duration / timeStep) > 0)
			pixelsPerItem = xAxis->axisRect()->width() / (duration / timeStep);
		/* hack to make sure this isn't called every time when zoomed in very far */
		long realTimeStep = timeStep > 0 ? timeStep : 1;
		if (followData && offset > cacheStart + static_cast<long>(MAX_CACHE_POINTS) * realTimeStep)
		{
			/* remove old data from the cache */
			cacheStart += static_cast<long>(MAX_CACHE_POINTS) * realTimeStep;
			for (int i = 0; i < graphCount(); i++)
			{
				auto d = graph(i)->data();
				d->removeBefore(cacheStart / 1000.0);
			}
		}
		/* if the needed data is cached and the number of pixels between data points is
		 * acceptable, just set the range properly instead of refilling */
		if (offset >= cacheStart && (offset + duration <= cacheEnd || duration == maxDuration)
		    && pixelsPerItem >= MIN_PIXELS_PER_POINT && pixelsPerItem <= MAX_PIXELS_PER_POINT)
		{
			xAxis->setRange(offset / 1000.0, (offset + duration) / 1000.0);
			if (!valueAxisScaled)
			{
				yAxis->setRange(minValue - (maxValue - minValue) / VALUE_EXTRA,
				    maxValue + (maxValue - minValue) / VALUE_EXTRA);
			}
			if (isVisible())
				replotAndCalcMargin();
		}
		else
		{
			refillData();
		}
	}

	void Plot::setMaxDuration(long dur)
	{
		maxDuration = dur;
		if (followData)
		{
			if (offset > 0)
			{
				/* if not all data is shown, move the offset right */
				setOffset(maxDuration - duration);
			}
			else
			{
				/* if all data is shown, scale the duration */
				duration = maxDuration;
			}
			recalculate();
		}
	}

	void Plot::mousePressEvent(QMouseEvent* event)
	{
		lastMouseX = event->x();
		lastMouseY = event->y();
		emit dragStarted();
	}

	void Plot::mouseReleaseEvent(QMouseEvent* event)
	{
		(void)event;
		emit dragStopped();
	}

	void Plot::mouseMoveEvent(QMouseEvent* event)
	{
		if (event->buttons() & Qt::LeftButton)
		{
			emit requestMovePlot(lastMouseX - event->x());
			lastMouseX = event->x();
			moveValueAxis(event->y() - lastMouseY);
			lastMouseY = event->y();
		}
	}

	void Plot::moveValueAxis(int delta)
	{
		if (!valueAxisScaled)
			return;
		valueOffset += (delta * valueLength) / yAxis->axisRect()->height();
		double realMin = minValue - (valueLength / VALUE_EXTRA);
		double realMax = maxValue + (valueLength / VALUE_EXTRA);
		if (valueOffset < realMin)
			valueOffset = realMin;
		if (valueOffset + valueLength > realMax)
		{
			valueOffset = realMax - valueLength;
			if (valueOffset < realMin)
			{
				valueAxisScaled = false;
				valueOffset = realMin;
				valueLength = realMax - realMin;
			}
		}
		yAxis->setRange(valueOffset, valueOffset + valueLength);
		replotAndCalcMargin();
	}

	void Plot::scaleValueAxis(int angleDelta, int mouseY)
	{
		if (!valueAxisScaled)
		{
			/* don't do anything when zooming out to avoid glitch */
			if (angleDelta > 0)
				return;
			double extra = (maxValue - minValue) / VALUE_EXTRA;
			valueLength = maxValue - minValue + 2 * extra;
			valueOffset = minValue - extra;
		}
		valueAxisScaled = true;
		double newLength = valueLength * (1 + angleDelta / 360.0);
		double moveScale = static_cast<double>(mouseY) / yAxis->axisRect()->height();
		valueOffset += (valueLength - newLength) * (1 - moveScale);
		valueLength = newLength;
		double realMin = minValue - (valueLength / VALUE_EXTRA);
		double realMax = maxValue + (valueLength / VALUE_EXTRA);
		if (valueOffset < realMin)
			valueOffset = realMin;
		if (valueOffset + valueLength > realMax)
		{
			valueOffset = realMax - valueLength;
			if (valueOffset < realMin)
			{
				valueAxisScaled = false;
				valueOffset = realMin;
				valueLength = realMax - realMin;
			}
		}
		yAxis->setRange(valueOffset, valueOffset + valueLength);
		replotAndCalcMargin();
	}

	void Plot::resetValueAxis()
	{
		valueAxisScaled = false;
		double extra = (maxValue - minValue) / VALUE_EXTRA;
		valueLength = maxValue - minValue + 2 * extra;
		valueOffset = minValue - extra;
		yAxis->setRange(valueOffset, valueOffset + valueLength);
		padding = 0;
		yAxis->setPadding(0);
	}

	void Plot::setLeftMargin(int margin)
	{
		padding = margin - (yAxis->axisRect()->left() - padding);
		if (padding < 0)
			padding = 0; /* can only happen when adding a new plot (I think) */
		yAxis->setPadding(padding);
		replotAndCalcMargin();
	}

	void Plot::wheelEvent(QWheelEvent* event)
	{
		bool shift = event->modifiers() & Qt::ShiftModifier;
		bool ctrl = event->modifiers() & Qt::ControlModifier;
		if (ctrl && !shift)
			emit requestScaleX(event->angleDelta().y(), event->x());
		else if (ctrl && shift)
			scaleValueAxis(-event->angleDelta().y(), event->y());
		else if (!ctrl && shift)
			moveValueAxis(event->angleDelta().y());
		else if (!ctrl && !shift)
			emit requestMovePlot(-event->angleDelta().y());
	}

	void Plot::resizeEvent(QResizeEvent* event)
	{
		/* this could be improved to actually change the amount of shown
		 * data instead of just resizing, but it shouldn't really matter
		 * during normal usage */
		QCustomPlot::resizeEvent(event);
		recalculate();
	}

	void Plot::showEvent(QShowEvent* event)
	{
		(void)event;
		replotAndCalcMargin();
	}

	void Plot::setID(int id)
	{
		this->id = id;
		/* the shown id is not zero-indexed */
		idLabel->setText(QStringLiteral("Plot ") + QString::number(id + 1));
	}
	long Plot::getOffset() const { return offset; }
	long Plot::getDuration() const { return duration; }

	void Plot::removeData(int dataID)
	{
		dataList.erase(dataList.begin() + dataID);
		removeGraph(dataID);
		if (dataList.empty())
		{
			emit requestRemovePlot(id);
			return;
		}
		maxValue = 1;
		minValue = 0;
		for (int i = 0; i < graphCount(); i++)
		{
			bool found = false;
			auto range = graph(i)->getValueRange(found);
			if (found)
			{
				if (range.upper > maxValue)
					maxValue = range.upper;
				if (range.lower < minValue)
					minValue = range.lower;
			}
		}
		recalculate();
	}

	void Plot::removePlot() { emit requestRemovePlot(id); }

	void Plot::showContextMenu(const QPoint& pos)
	{
		QMenu menu(this);
		QAction* removePlotAction = new QAction("Remove plot");
		menu.addAction(removePlotAction);
		connect(removePlotAction, &QAction::triggered, this, &Plot::removePlot);
		QMenu* removeMenu = new QMenu("Remove data");
		menu.addMenu(removeMenu);
		for (size_t i = 0; i < dataList.size(); i++)
		{
			QAction* act
			    = new QAction(QString("Slave %1: %2")
			                      .arg(QString::number(dataList[i].data.get().getSlaveID()),
			                          QString::fromStdString(dataList[i].data.get().getName())));
			removeMenu->addAction(act);
			connect(act, &QAction::triggered, [this, i]() { removeData(static_cast<int>(i)); });
		}
		menu.exec(this->mapToGlobal(pos));
		menu.clear();
	}

} // namespace etherkitten::gui
