/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/qtwidgets/properties/transferfunctioneditorcontrolpoint.h>
#include <modules/qtwidgets/properties/transferfunctioncontrolpointconnection.h>

#include <modules/qtwidgets/properties/transferfunctioneditor.h>
#include <modules/qtwidgets/properties/transferfunctioneditorview.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

TransferFunctionEditorControlPoint::TransferFunctionEditorControlPoint(
    TransferFunctionDataPoint* datapoint, QGraphicsScene* scene, float size)
    : TransferFunctionEditorPrimitive(scene, vec2(datapoint->getPos(), datapoint->getRGBA().a),
                                      size)
    , dataPoint_(datapoint) {
    dataPoint_->addObserver(this);
}

void TransferFunctionEditorControlPoint::set(float scalar, float alpha) {
    dataPoint_->setPosA(scalar, alpha);
}

void TransferFunctionEditorControlPoint::setScalarValue(float scalar) {
    dataPoint_->setPos(scalar);
}

float TransferFunctionEditorControlPoint::getScalarValue() const { return dataPoint_->getPos(); }

void TransferFunctionEditorControlPoint::setColor(const vec4& color) { dataPoint_->setRGBA(color); }

void TransferFunctionEditorControlPoint::setColor(const vec3& color) { dataPoint_->setRGB(color); }

const vec4& TransferFunctionEditorControlPoint::getColor() const { return dataPoint_->getRGBA(); }

void TransferFunctionEditorControlPoint::onTransferFunctionPointChange(
    const TransferFunctionDataPoint* p) {
    setTFPosition(vec2(p->getPos(), p->getRGBA().a));
}

void TransferFunctionEditorControlPoint::setDataPoint(TransferFunctionDataPoint* dataPoint) {
    dataPoint_ = dataPoint;
}

TransferFunctionDataPoint* TransferFunctionEditorControlPoint::getPoint() const {
    return dataPoint_;
}

QRectF TransferFunctionEditorControlPoint::boundingRect() const {
    double bBoxSize = getSize() + 5.0;  //<! consider size of pen
    auto bRect = QRectF(-bBoxSize / 2.0, -bBoxSize / 2.0, bBoxSize, bBoxSize);

    return bRect;
}

QPainterPath TransferFunctionEditorControlPoint::shape() const {
    QPainterPath path;
    const auto radius = getSize() * 0.5 + 1.5;  //<! consider size of pen
    path.addEllipse(QPointF(0.0, 0.0), radius, radius);
    return path;
}

void TransferFunctionEditorControlPoint::paintPrimitive(QPainter* painter) {
    const auto radius = getSize() * 0.5;
    painter->drawEllipse(QPointF(0.0, 0.0), radius, radius);
}

QPointF TransferFunctionEditorControlPoint::prepareItemPositionChange(const QPointF& pos) {
    QPointF adjustedPos(pos);

    if (auto tfe = qobject_cast<TransferFunctionEditor*>(scene())) {
        QRectF rect = scene()->sceneRect();
        const double d = 2.0 * rect.width() * glm::epsilon<float>();

        const int moveMode = tfe->getMoveMode();

        // need to update position prior to updating connections
        currentPos_ = adjustedPos;

        if (left_) {
            if (left_->left_ && *(left_->left_) > *this) {
                switch (moveMode) {
                    case 0:  // Free
                        break;
                    case 1:  // Restrict
                        adjustedPos.setX(left_->left_->getCurrentPos().x() + d);
                        // need to update position prior to updating connections
                        currentPos_ = adjustedPos;
                        break;
                    case 2:  // Push
                        left_->left_->setPos(
                            QPointF(adjustedPos.x() - d, left_->left_->getCurrentPos().y()));
                        break;
                }

                tfe->updateConnections();
            } else {
                currentPos_ = adjustedPos;
                left_->updateShape();
            }
        }
        if (right_) {
            if (right_->right_ && *(right_->right_) < *this) {
                switch (moveMode) {
                    case 0:  // Free
                        break;
                    case 1:  // Restrict
                        adjustedPos.setX(right_->right_->getCurrentPos().x() - d);
                        // need to update position prior to updating connections
                        currentPos_ = adjustedPos;
                        break;
                    case 2:  // Push
                        right_->right_->setPos(
                            QPointF(adjustedPos.x() + d, right_->right_->getCurrentPos().y()));
                        break;
                }

                tfe->updateConnections();
            } else {
                right_->updateShape();
            }
        }
    }

    return adjustedPos;
}

void TransferFunctionEditorControlPoint::onItemPositionChange(const vec2& newPos) {
    dataPoint_->setPosA(newPos.x, newPos.y);
}

void TransferFunctionEditorControlPoint::onItemSceneHasChanged() {
    onTransferFunctionPointChange(dataPoint_);
}

bool operator==(const TransferFunctionEditorControlPoint& lhs,
                const TransferFunctionEditorControlPoint& rhs) {
    return *lhs.dataPoint_ == *rhs.dataPoint_;
}

bool operator!=(const TransferFunctionEditorControlPoint& lhs,
                const TransferFunctionEditorControlPoint& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TransferFunctionEditorControlPoint& lhs,
               const TransferFunctionEditorControlPoint& rhs) {
    return lhs.currentPos_.x() < rhs.currentPos_.x();
}

bool operator>(const TransferFunctionEditorControlPoint& lhs,
               const TransferFunctionEditorControlPoint& rhs) {
    return rhs < lhs;
}

bool operator<=(const TransferFunctionEditorControlPoint& lhs,
                const TransferFunctionEditorControlPoint& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TransferFunctionEditorControlPoint& lhs,
                const TransferFunctionEditorControlPoint& rhs) {
    return !(lhs < rhs);
}

}  // namespace inviwo
