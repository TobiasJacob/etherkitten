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

#include "Edge.hpp"
#include "Node.hpp"

namespace etherkitten::gui
{

	Edge::Edge(QGraphicsScene* scene, Node* sourceNode, Node* destNode)
	    : source(sourceNode)
	    , dest(destNode)
	    , scene(scene)
	    , lowImg(nullptr)
	    , mediumImg(nullptr)
	{
		setAcceptedMouseButtons(Qt::NoButton);
		sourceNode->addEdge(this);
		destNode->addEdge(this);
		setZValue(-2);
		adjustPosition();
	}

	Node* Edge::sourceNode() const { return source; }

	Node* Edge::destNode() const { return dest; }

	void Edge::adjustPosition()
	{
		QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));

		prepareGeometryChange();

		sourcePoint = line.p1();
		destPoint = line.p2();

		if (lowImg != nullptr && lowImg->isVisible())
			setErrorPos();
		if (mediumImg != nullptr && mediumImg->isVisible())
			setErrorPos();
	}

	void Edge::clearError()
	{
		if (lowImg != nullptr)
			lowImg->hide();
		if (mediumImg != nullptr)
			mediumImg->hide();
	}

	void Edge::setErrorPos()
	{
		double x = std::min(sourcePoint.x(), destPoint.x());
		double y = std::min(sourcePoint.y(), destPoint.y());
		double w = std::abs(sourcePoint.x() - destPoint.x());
		double h = std::abs(sourcePoint.y() - destPoint.y());
		if (lowImg != nullptr)
			lowImg->setPos(x + w / 2, y + h / 2);
		if (mediumImg != nullptr)
			mediumImg->setPos(x + w / 2, y + h / 2);
	}

	void Edge::showError(std::string msg, datatypes::ErrorSeverity severity)
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
		(*error)->show();
		(*error)->setToolTip(QString::fromStdString(msg));
		setErrorPos();
	}

	QRectF Edge::boundingRect() const
	{
		return QRectF(
		    sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(), destPoint.y() - sourcePoint.y()))
		    .normalized();
	}

	void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		(void)option;
		(void)widget;
		QLineF line(sourcePoint, destPoint);
		painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		painter->drawLine(line);
	}

} // namespace etherkitten::gui
