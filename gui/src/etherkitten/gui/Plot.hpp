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

#include "DataModelAdapter.hpp"
#include "TimeStampConverter.hpp"
#include "qcustomplot.hpp"
#include <QMouseEvent>
#include <QPoint>
#include <QWheelEvent>
#include <QWidget>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::gui
{

	/*!
	 * \brief Represents a plot that may show several DataObjects.
	 */
	class Plot : public QCustomPlot
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new Plot.
		 * \param parent The parent widget.
		 * \param id The id of the plot.
		 * \param adapter The adapter used for obtaining the DataViews.
		 * \param timeConverter The converter used to convert absolute timestamps into
		 * relative milliseconds and vice versa.
		 */
		Plot(QWidget* parent, int id, DataModelAdapter& adapter, TimeStampConverter& timeConverter);
		/*!
		 * \brief Add data to be shown on the plot.
		 * \param data The DataObject to be shown.
		 */
		void addData(const datatypes::DataObject& data);
		/*!
		 * \brief Update the data on the right and recalculate if the data is being followed.
		 */
		void updateData();
		/*!
		 * \brief Return whether the plot should follow the new data on the right.
		 * \return Whether the plot should follow the new data on the right.
		 */
		bool getFollowData() const;
		/*!
		 * \brief Set whether the plot should follow the new data on the right.
		 * \param followData Whether the plot should follow the new data on the right.
		 */
		void setFollowData(bool followData);
		/*!
		 * \brief Set the left position of the plot.
		 * \param time The left position of the plot in milliseconds.
		 */
		void setOffset(long time);
		/*!
		 * \brief Reset the value axis to its default scale.
		 */
		void resetValueAxis();
		/*!
		 * \brief Set the shown duration.
		 * \param duration The shown duration in milliseconds.
		 */
		void setShownDuration(long duration);
		/*!
		 * \brief Set the id of the plot.
		 * \param id The new id.
		 */
		void setID(int id);
		/*!
		 * \brief Recalculate the plots, i.e. set the shown range and check
		 * if the current time step for the data points is still acceptable.
		 */
		void recalculate();
		/*!
		 * \brief Return the current left side of the plot in milliseconds.
		 * \return The current left side of the plot in milliseconds.
		 */
		long getOffset() const;
		/*!
		 * \brief Return the currently shown duration in milliseconds.
		 * \return The currently shown duration in milliseconds.
		 */
		long getDuration() const;
		/*!
		 * \brief Set the maximum time in milliseconds (the far right side of the plot).
		 * \param dur
		 */
		void setMaxDuration(long dur);
		/*!
		 * \brief Return the maximum time in milliseconds seen in the local data.
		 * \return The maximum time in milliseconds seen in the local data.
		 */
		long getMaxLocalDuration() const;
		/*!
		 * \brief Calculate the optimal time step for the points and refill the data accordingly.
		 */
		void refillData();
		/*!
		 * \brief Set the minimum margin on the left side of the plot. If the labels on
		 * this plot don't need that much space, padding is added. Note that the plot must
		 * be in a consistent state before this is called, i.e. the left margin needed must
		 * not have changed since the last call to replot(). This is not ideal because it
		 * means that the plots are replotted twice when the needed margin changes, but to
		 * improve that, there would need to be a way to get the needed margin from
		 * QCustomPlot without replotting. Maybe there is such a way, but the author of
		 * this code doesn't know about it.
		 * \param margin The minimum margin on the left side of the plot.
		 */
		void setLeftMargin(int margin);
		/*!
		 * \brief Return the minimum margin this plot needs on the left side.
		 * \return The minimum margin this plot needs on the left side.
		 */
		int getNeededMargin() const;
		/*!
		 * \brief The maximum number of data points cached on either
		 * side of the currently visible area.
		 */
		static const size_t MAX_CACHE_POINTS = 100;
		/*!
		 * \brief The maximum pixels between data points.
		 */
		static const int MAX_PIXELS_PER_POINT = 8;
		/*!
		 * \brief The ideal number of pixels between data points.
		 */
		static const int IDEAL_PIXELS_PER_POINT = 5;
		/*!
		 * \brief The minimum number of pixels between data points.
		 */
		static const int MIN_PIXELS_PER_POINT = 2;

	public slots:
		/*!
		 * \brief Remove the data with the given id. If no data is left, the plot is removed.
		 * \param id The data index in the internal list.
		 */
		void removeData(int id);
		/*!
		 * \brief Remove this plot (requests that the PlotList remove the plot).
		 */
		void removePlot();

	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void showEvent(QShowEvent* event) override;

	private slots:
		/*!
		 * \brief Show the context menu.
		 * \param pos The position to show the menu at.
		 */
		void showContextMenu(const QPoint& pos);

	signals:
		/*!
		 * \brief Request that all plots be moved.
		 * \param delta The delta of the mouse drag.
		 */
		void requestMovePlot(int delta);
		/*!
		 * \brief Request that all plots be scaled.
		 * \param angleDelta The delta of the scroll wheel.
		 * \param mouseX The current x position of the mouse.
		 */
		void requestScaleX(int angleDelta, int mouseX);
		/*!
		 * \brief Request that this plot be removed.
		 * \param plotID The ID of this plot.
		 */
		void requestRemovePlot(int plotID);
		/*!
		 * \brief Request that all plots follow the data automatically or not.
		 * \param followData Whether to follow the data or not.
		 */
		void requestFollowData(bool followData);
		/*!
		 * \brief Notify that the needed margin for this plot has changed.
		 * The needed margin is the smallest amount of space required between
		 * the left edge of the widget and the actual plot. This signal is
		 * meant to be used to synchronize the value axes of different plots
		 * so padding can be added when the labels on one plot take more space
		 * than on another one.
		 */
		void neededMarginChanged();
		/*!
		 * \brief Notify that the user has started dragging the plot.
		 */
		void dragStarted();
		/*!
		 * \brief Notify that the user has stopped dragging the plot.
		 */
		void dragStopped();

	private:
		/*!
		 * \brief Move the value axis by the given delta in pixels.
		 * \param delta The delta in pixels (from mouse drag).
		 */
		void moveValueAxis(int delta);
		/*!
		 * \brief Scale the value axis.
		 * \param angleDelta The delta of the mouse wheel.
		 * \param mouseY The current y position of the mouse.
		 */
		void scaleValueAxis(int angleDelta, int mouseY);
		/*!
		 * \brief Replot and calculate the minimum needed space at the left
		 * of the plot (where the value axis is drawn). If it has changed,
		 * emit neededMarginChanged() signal.
		 */
		void replotAndCalcMargin();
		struct Metadata
		{
			Metadata(const datatypes::DataObject& data,
			    std::shared_ptr<datatypes::AbstractDataView> view,
			    std::unique_ptr<datatypes::AbstractNewestValueView> newest, long lastTime)
			    : data(data)
			    , view(view)
			    , newest(std::move(newest))
			    , lastTime(lastTime)
			{
			}
			Metadata(Metadata&& other)
			    : data(other.data)
			    , view(other.view)
			    , newest(std::move(other.newest))
			    , lastTime(other.lastTime)
			{
			}
			std::reference_wrapper<const datatypes::DataObject> data;
			std::shared_ptr<datatypes::AbstractDataView> view;
			std::unique_ptr<datatypes::AbstractNewestValueView> newest;
			long lastTime; /* the last time that a point was added for this data (to avoid adding
			                  too many points) */
			Metadata& operator=(Metadata&& other)
			{
				data = other.data;
				view = std::move(other.view);
				newest = std::move(other.newest);
				lastTime = other.lastTime;
				return *this;
			}
		};
		DataModelAdapter& adapter;
		TimeStampConverter& timeConverter;
		std::vector<Metadata> dataList;
		QSharedPointer<QCPAxisTickerTime> timeTicker;
		std::vector<QPen> pens;
		QCPTextElement* idLabel;
		static const int VALUE_EXTRA
		    = 20; /* determines how much extra space is shown above the largest and below the
		             smallest number - this is needed because otherwise the largest or smallest data
		             points sometimes are hidden at the edge; the actual extra space is calculated
		             as ((range of current values) / VALUE_EXTRA) */
		long cacheStart; /* start of cache in milliseconds */
		long cacheEnd; /* end of cache in milliseconds */
		long offset; /* current left position in milliseconds */
		long duration; /* current shown duration in milliseconds */
		long timeStep; /* current number of milliseconds between shown points - this is the ideal
		                  value, not what the dataview actually gave */
		long maxDuration; /* maximum duration in milliseconds, i.e. the far right of the plot */
		long maxLocalDuration; /* maximum duration in milliseconds that was seen in local data */
		double valueOffset; /* current bottom position of the value axis */
		double valueLength; /* current shown length of the value axis */
		double maxValue; /* maximum value from all data, but never smaller than 1 - this is done to
		                    simplify handling of the special case when all values are 0 */
		double minValue; /* minimum value from all data, but never greater than 0 */
		int id; /* id of this plot - used for requestRemovePlot signal */
		int lastMouseX; /* last mouse x position while dragging */
		int lastMouseY; /* last mouse y position while dragging */
		int padding; /* current extra padding to align value axis */
		int neededMargin; /* minimum needed margin (distance from left edge of widget to actual
		                     plot) */
		bool followData; /* whether the data is currently being followed, i.e. new data is added
		                    automatically on the right side */
		bool valueAxisScaled; /* whether the value axis is currently scaled (so it isn't
		                         automatically adjusted anymore) */
	};

} // namespace etherkitten::gui
