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

#include "SlaveGraph.hpp"
#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QtMath>
#include <chrono>
#include <etherkitten/datatypes/EtherCATTypeParser.hpp>
#include <etherkitten/datatypes/EtherCATTypeStringFormatter.hpp>
#include <etherkitten/datatypes/errorstatistic.hpp>

namespace etherkitten::gui
{

	SlaveGraph::SlaveGraph(QWidget* parent, DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo,
	    config::BusLayout& busLayout)
	    : QFrame(parent)
	    , busInfo(busInfo)
	    , dataAdapter(dataAdapter)
	    , scene(new QGraphicsScene(this))
	    , view(new QGraphicsView(this))
	    , errorLog(nullptr)
	    , busLayout(busLayout)
	{
		scene->setItemIndexMethod(QGraphicsScene::NoIndex);
		view->setScene(scene);
		view->setCacheMode(QGraphicsView::CacheBackground);
		view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
		view->setRenderHint(QPainter::Antialiasing);
		view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		view->scale(qreal(0.8), qreal(0.8));
		view->setFrameShape(NoFrame);
		view->viewport()->installEventFilter(this);
		view->setMouseTracking(true);
		view->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(new QLabel("Ctrl+Scroll to zoom"));
		QPushButton* clearBtn = new QPushButton("Clear errors");
		clearBtn->setToolTip("Clear the errors shown in the graph. This only resets the error "
		                     "icons in the graph, it does not actually reset any error registers.");
		connect(clearBtn, &QPushButton::clicked, this, &SlaveGraph::clearErrors);
		layout->addWidget(clearBtn);
		layout->addWidget(view);
	}

	void SlaveGraph::handleErrorMessage(const datatypes::ErrorMessage&& msg)
	{
		auto slavePair = msg.getAssociatedSlaves();
		unsigned int first = slavePair.first;
		unsigned int second = slavePair.second;
		if (first < nodes.size() && second < nodes.size())
		{
			nodes[first].node->showConnectionError(second, msg.getMessage(), msg.getSeverity());
		}
		else if (first < nodes.size())
		{
			nodes[first].node->showSlaveError(msg.getMessage(), msg.getSeverity());
			nodes[first].lowFreqError = false;
			nodes[first].highFreqError = false;
		}
		else if (second < nodes.size())
		{
			nodes[second].node->showSlaveError(msg.getMessage(), msg.getSeverity());
			nodes[second].lowFreqError = false;
			nodes[second].highFreqError = false;
		}
	}

	void SlaveGraph::updateData()
	{
		for (size_t i = 1; i < nodes.size(); i++)
		{
			NodeMetadata& nm = nodes[i];
			/* show errors for error statistic frequencies */
			double totalErrorFreq = 0;
			for (auto& view : nm.statViews)
			{
				while (view->hasNext())
					++(*view);
				if (view->isEmpty())
					continue;
				double freq = view->asDouble();
				if (freq >= 0)
					totalErrorFreq += freq;
			}
			if (totalErrorFreq >= highErrorFreq)
			{
				if (!nm.highFreqError)
				{
					nm.node->showSlaveError(
					    "Error statistic frequencies very high", datatypes::ErrorSeverity::MEDIUM);
					nm.highFreqError = true;
					nm.lowFreqError = false;
				}
			}
			else if (totalErrorFreq >= lowErrorFreq)
			{
				if (!nm.lowFreqError)
				{
					nm.node->showSlaveError(
					    "Error statistic frequencies non-zero", datatypes::ErrorSeverity::LOW);
					nm.lowFreqError = true;
					nm.highFreqError = false;
				}
			}

			/* change color of node based on bus status */
			if (!nm.view || nm.view->isEmpty())
				continue;
			std::string valueStr = (**nm.view).asString(datatypes::NumberFormat::DECIMAL);
			auto value
			    = datatypes::EtherCATTypeParser<datatypes::EtherCATDataType::UNSIGNED8>::parse(
			        valueStr);
			Node::NodeStatus status = Node::NodeStatus::UNKNOWN;
			if ((value & 0xF) == 4)
				status = Node::NodeStatus::SAFE_OP;
			else if ((value & 0xF) == 8)
				status = Node::NodeStatus::OP;
			if (status != nm.node->getStatus())
			{
				nm.node->setStatus(status);
				std::string statusStr = "Unknown";
				if (status == Node::NodeStatus::SAFE_OP)
					statusStr = "Safe-Op";
				else if (status == Node::NodeStatus::OP)
					statusStr = "Op";
				nm.node->setToolTip(
				    QString::fromStdString(nm.baseTooltip + "\nStatus: " + statusStr));
			}
		}

		/* show errors for messages in the error log */
		if (errorLog == nullptr)
			return;
		while (errorLog->hasNext())
		{
			++(*errorLog);
			handleErrorMessage((**errorLog).getValue());
		}
	}

	void SlaveGraph::clear()
	{
		scene->clear();
		errorLog.reset();
		nodes.clear();
	}

	void SlaveGraph::setupData()
	{
		clear();
		unsigned int slaveCount = busInfo.getSlaveCount();
		nodes.reserve(slaveCount + 1);
		NodeMetadata& master = nodes.emplace_back("", nullptr,
		    std::vector<std::shared_ptr<datatypes::AbstractDataView>>(),
		    new Node(scene, "Master", 0, []() {}), false, false);
		scene->addItem(master.node);
		master.node->setToolTip("Master");
		master.node->setStatus(Node::NodeStatus::OP);

		int padding = 5;
		int masterWidth = static_cast<int>(master.node->boundingRect().width());
		int totalWidth = masterWidth + padding;

		/* these are all the error statistic frequencies that are used to
		 * determine whether an error should be shown on the slave */
		std::unordered_map<datatypes::ErrorStatisticType, bool> neededStats;
		for (auto& stat : datatypes::errorStatisticInfos)
		{
			neededStats[stat.getType(datatypes::ErrorStatisticCategory::FREQ_SLAVE)] = true;
		}
		for (unsigned int i = 1; i <= slaveCount; i++)
		{
			const datatypes::SlaveInfo& slave = busInfo.getSlaveInfo(i);
			std::string tooltip = "Name: " + slave.getName();
			std::optional<datatypes::ESIData> esi = slave.getESI();
			if (esi.has_value())
			{
				if (esi.value().general.has_value()
				    && esi.value().general.value().nameIdx < esi.value().strings.size())
				{
					tooltip += "\nVendor-specific name: "
					    + esi.value().strings[esi.value().general.value().nameIdx];
				}
				tooltip += "\nSerial number: "
				    + datatypes::EtherCATTypeStringFormatter<
				          datatypes::EtherCATDataType::UNSIGNED32>::asString(esi.value()
				                                                                 .header
				                                                                 .serialNumber,
				          datatypes::NumberFormat::DECIMAL);
			}

			NodeMetadata& nm = nodes.emplace_back(tooltip, nullptr,
			    std::vector<std::shared_ptr<datatypes::AbstractDataView>>(),
			    new Node(scene, slave.getName(), i, [this, i]() { emit slaveClicked(i); }), false,
			    false);
			scene->addItem(nm.node);
			totalWidth += static_cast<int>(nm.node->boundingRect().width()) + padding;

			for (auto& reg : slave.getRegisters())
			{
				if (reg.getRegister() == datatypes::RegisterEnum::STATUS)
				{
					nm.view = dataAdapter.getNewestValueView(reg);
					break;
				}
			}
			/* try to make the DataViews jump forward quickly */
			datatypes::TimeSeries series
			    = { datatypes::TimeStamp(std::chrono::seconds(0)), datatypes::TimeStep(10000000) };
			for (auto& stat : slave.getErrorStatistics())
			{
				if (neededStats[stat.getStatisticType()])
				{
					nm.statViews.emplace_back(dataAdapter.getDataView(stat, series));
				}
			}
		}

		int curPos = -(totalWidth / 2);
		master.node->setPos(curPos + (masterWidth / 2), 0);
		curPos += masterWidth + padding;
		/* add connections between slaves */
		for (unsigned int i = 1; i <= slaveCount; i++)
		{
			NodeMetadata& nm = nodes[i];
			int width = static_cast<int>(nm.node->boundingRect().width());
			nm.node->setPos(curPos + (width / 2), 0);
			curPos += width + padding;
			const datatypes::SlaveInfo& slave = busInfo.getSlaveInfo(i);
			auto neighbors = slave.getNeighbors();
			for (size_t port = 0; port < neighbors.size(); port++)
			{
				unsigned int neighbor = neighbors[port];

				/* Unused positions in the neighbor array are set to the
				 * maximum size for unsigned int, so skip them here. */
				if (neighbor >= nodes.size())
					continue;
				std::string slave = neighbor == 0 ? "Master" : "Slave " + std::to_string(neighbor);
				nm.baseTooltip += "\nPort " + std::to_string(port) + ": " + slave;

				/* Skip edges to slaves with a lower id since they were
				 * already created starting from that slave. */
				if (neighbor <= i && neighbor != 0)
					continue;
				scene->addItem(new Edge(scene, nodes.at(i).node, nodes.at(neighbor).node));
			}
			nm.node->setToolTip(QString::fromStdString(nm.baseTooltip) + "\nStatus: Unknown");
		}
		errorLog = busInfo.getErrorLog();
		/* get the first message if it exists */
		if (!errorLog->isEmpty())
			handleErrorMessage((**errorLog).getValue());
	}

	void SlaveGraph::writeBusLayout()
	{
		for (auto& nm : nodes)
		{
			config::Position pos(static_cast<int>(nm.node->x()), static_cast<int>(nm.node->y()));
			busLayout.get().setPosition(nm.node->getSlaveID(), pos);
		}
	}

	void SlaveGraph::applyBusLayout()
	{
		unsigned int count = busLayout.get().getSlaveCount();
		for (size_t i = 0; i < nodes.size() && i < count; i++)
		{
			config::Position pos = busLayout.get().getPosition(nodes[i].node->getSlaveID());
			nodes[i].node->setPos(pos.x, pos.y);
		}
	}

	void SlaveGraph::setBusLayout(config::BusLayout& newBusLayout) { busLayout = newBusLayout; }

	void SlaveGraph::clearErrors()
	{
		for (auto& nm : nodes)
		{
			nm.node->clearErrors();
			nm.lowFreqError = false;
			nm.highFreqError = false;
		}
	}

	/* See
	 * https://stackoverflow.com/questions/19113532/qgraphicsview-zooming-in-and-out-under-mouse-position-using-mouse-wheel
	 */
	bool SlaveGraph::eventFilter(QObject* object, QEvent* event)
	{
		(void)object;
		if (event->type() == QEvent::MouseMove)
		{
			QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
			QPointF delta = targetViewportPos - mouseEvent->pos();
			if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
			{
				targetViewportPos = mouseEvent->pos();
				targetScenePos = view->mapToScene(mouseEvent->pos());
			}
		}
		else if (event->type() == QEvent::Wheel)
		{
			QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(event);
			if (QApplication::keyboardModifiers() == Qt::ControlModifier)
			{
				if (wheelEvent->orientation() == Qt::Vertical)
				{
					double angle = wheelEvent->angleDelta().y();
					double factor = qPow(1.0015, angle);
					view->scale(factor, factor);
					view->centerOn(targetScenePos);
					QPointF delta = targetViewportPos
					    - QPointF(
					          view->viewport()->width() / 2.0, view->viewport()->height() / 2.0);
					QPointF center = view->mapFromScene(targetScenePos) - delta;
					view->centerOn(view->mapToScene(center.toPoint()));
					return true;
				}
			}
		}
		return false;
	}

} // namespace etherkitten::gui
