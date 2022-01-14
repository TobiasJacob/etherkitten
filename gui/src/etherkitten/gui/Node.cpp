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

#include "Node.hpp"
#include "Edge.hpp"

namespace etherkitten::gui
{

	Node::Node(QGraphicsScene* scene, std::string name, unsigned int id,
	    std::function<void()> clickFunction)
	    : id(id)
	    , status(NodeStatus::UNKNOWN)
	    , lowImg(nullptr)
	    , mediumImg(nullptr)
	    , scene(scene)
	    , clickFunction(clickFunction)
	{
		setFlag(ItemIsMovable);
		setFlag(ItemSendsGeometryChanges);
		setCacheMode(DeviceCoordinateCache);
		setZValue(-1);
		label = QString::number(id) + ": ";
		if (name.length() > MAX_NAME_LENGTH)
			label += QString::fromStdString(name.substr(0, MAX_NAME_LENGTH)) + "...";
		else
			label += QString::fromStdString(name);
		scene->setFont(QFont(scene->font().defaultFamily(), 16, QFont::Bold));
		QFontMetricsF fm(scene->font());
		/* the tight bounding rect is needed to put the text properly in the middle,
		 * but the big bounding rect is used for the actual size of the drawn rectangle
		 * so all nodes are the same height */
		QRectF rect = fm.tightBoundingRect(label);
		QRectF bigRect = fm.boundingRect(label);
		textWidth = rect.width();
		textHeight = rect.height();
		width = static_cast<int>(bigRect.width()) + TEXT_PADDING;
		height = static_cast<int>(bigRect.height()) + TEXT_PADDING;
	}

	void Node::clearErrors()
	{
		if (lowImg != nullptr)
			lowImg->hide();
		if (mediumImg != nullptr)
			mediumImg->hide();
		for (auto edge : qAsConst(edges))
		{
			edge->clearError();
		}
	}

	void Node::showConnectionError(
	    unsigned int otherSlave, std::string msg, datatypes::ErrorSeverity severity)
	{
		for (auto edge : qAsConst(edges))
		{
			if (edge->sourceNode()->getSlaveID() == otherSlave
			    || edge->destNode()->getSlaveID() == otherSlave)
			{
				edge->showError(msg, severity);
				break;
			}
		}
	}

	void Node::showSlaveError(std::string msg, datatypes::ErrorSeverity severity)
	{
		QGraphicsPixmapItem** error;
		QString path;
		if (severity == datatypes::ErrorSeverity::LOW)
		{
			error = &lowImg;
			path = ":/images/low-error.png";
		}
		else
		{
			error = &mediumImg;
			path = ":/images/medium-error.png";
		}
		if (*error == nullptr)
		{
			*error = new QGraphicsPixmapItem(QPixmap(path));
			(*error)->setZValue(1);
			scene->addItem(*error);
		}
		(*error)->setToolTip(QString::fromStdString(msg));
		(*error)->show();
		(*error)->setPos(pos());
	}

	void Node::addEdge(Edge* edge)
	{
		edges << edge;
		edge->adjustPosition();
	}

	unsigned int Node::getSlaveID() const { return id; }

	QPainterPath Node::shape() const
	{
		QPainterPath path;
		path.addRect(-(width / 2), -(height / 2), width, height);
		return path;
	}

	void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		(void)widget;
		QLinearGradient gradient(0, -(height / 2), 0, height / 2);
		painter->setFont(scene->font());
		QColor col1;
		QColor col2;
		if (status == NodeStatus::OP)
		{
			col1 = Qt::green;
			col2 = Qt::darkGreen;
		}
		else if (status == NodeStatus::SAFE_OP)
		{
			col1 = Qt::yellow;
			col2 = Qt::darkYellow;
		}
		else
		{
			col1 = Qt::gray;
			col2 = Qt::darkGray;
		}
		if (option->state & QStyle::State_Sunken)
		{
			gradient.setColorAt(1, col1);
			gradient.setColorAt(0, col2);
		}
		else
		{
			gradient.setColorAt(1, col2);
			gradient.setColorAt(0, col1);
		}
		painter->setBrush(gradient);

		painter->setPen(QPen(Qt::black, 0));
		painter->drawRect(-(width / 2), -(height / 2), width, height);
		painter->drawText(QPointF(-textWidth / 2, textHeight / 2), label);
	}

	void Node::setStatus(NodeStatus status)
	{
		this->status = status;
		update();
	}

	Node::NodeStatus Node::getStatus() const { return status; }

	QRectF Node::boundingRect() const { return QRectF(-(width / 2), -(height / 2), width, height); }

	QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value)
	{
		if (change == ItemPositionHasChanged)
		{
			for (auto edge : qAsConst(edges))
				edge->adjustPosition();
			if (lowImg != nullptr && lowImg->isVisible())
				lowImg->setPos(pos());
			if (mediumImg != nullptr && mediumImg->isVisible())
				mediumImg->setPos(pos());
		};

		return QGraphicsItem::itemChange(change, value);
	}

	void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		update();
		QGraphicsItem::mousePressEvent(event);
	}

	void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		update();
		QGraphicsItem::mouseReleaseEvent(event);
	}

	void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsItem::mouseDoubleClickEvent(event);
		clickFunction();
	}

} // namespace etherkitten::gui
