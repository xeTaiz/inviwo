/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/qtwidgets/properties/transferfunctioneditorisovalue.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/transferfunctioneditor.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

TransferFunctionEditorIsoValue::TransferFunctionEditorIsoValue(IsoValue* isovalue,
                                                               QGraphicsScene* scene, float size)
    : TransferFunctionEditorPrimitive(scene, vec2(isovalue->getIsoValue(), isovalue->getColor().a),
                                      size)
    , isoValue_(isovalue) {
    isoValue_->addObserver(this);
}

void TransferFunctionEditorIsoValue::set(float scalar, float alpha) {
    isoValue_->set({scalar, vec4(vec3(isoValue_->getColor()), alpha)});
}

void TransferFunctionEditorIsoValue::setScalarValue(float scalar) {
    isoValue_->setIsoValue(scalar);
}

float TransferFunctionEditorIsoValue::getScalarValue() const { return isoValue_->getIsoValue(); }

void TransferFunctionEditorIsoValue::setColor(const vec4& color) { isoValue_->setColor(color); }

void TransferFunctionEditorIsoValue::setColor(const vec3& color) {
    isoValue_->setColor(vec4(color, isoValue_->getColor().a));
}

const vec4& TransferFunctionEditorIsoValue::getColor() const { return isoValue_->getColor(); }

void TransferFunctionEditorIsoValue::onIsoValueChange(const IsoValue& v) {
    setTFPosition(vec2(v.getIsoValue(), v.getColor().a));
}

void TransferFunctionEditorIsoValue::paintPrimitive(QPainter* painter) {
    // draw vertical line
    painter->save();

    QPen pen = QPen();
    pen.setCosmetic(true);
    pen.setCapStyle(Qt::RoundCap);

    pen.setWidthF(2.0);
    pen.setColor(QColor(64, 64, 64, 255));
    pen.setStyle(Qt::DashLine);
    // ensure line is always covering entire scene
    QRectF rect = scene()->sceneRect();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    double verticalScaling = 1.0;
    if (auto tfe = qobject_cast<TransferFunctionEditor*>(scene())) {
        const auto zoom = tfe->getTransferFunctionProperty()->getZoomV();
        verticalScaling = 1.0 / (zoom.y - zoom.x);
    }

    painter->drawLine(QLineF(QPointF(0.0, -rect.height() * verticalScaling),
                             QPointF(0.0, rect.height() * verticalScaling)));

    painter->restore();

    // draw square for indicating isovalue
    const auto width = getSize();
    painter->drawRect(QRectF(QPointF(-0.5f * width, -0.5f * width), QSizeF(width, width)));
}

QRectF TransferFunctionEditorIsoValue::boundingRect() const {
    double bBoxSize = getSize() + 5.0;  //<! consider size of pen
    auto bRect = QRectF(-bBoxSize / 2.0, -bBoxSize / 2.0, bBoxSize, bBoxSize);

    // add box of vertical line
    double verticalScaling = 1.0;
    if (auto tfe = qobject_cast<TransferFunctionEditor*>(scene())) {
        const auto zoom = tfe->getTransferFunctionProperty()->getZoomV();
        verticalScaling = 1.0 / (zoom.y - zoom.x);
    }
    QRectF rect = scene()->sceneRect();
    return bRect.united(QRectF(QPointF(-1.0, -rect.height() * verticalScaling),
                               QPointF(1.0, rect.height() * verticalScaling)));
}

void TransferFunctionEditorIsoValue::onItemPositionChange(const vec2& newPos) {
    isoValue_->set(newPos.x, vec4(vec3(isoValue_->getColor()), newPos.y));
}

void TransferFunctionEditorIsoValue::onItemSceneHasChanged() { onIsoValueChange(*isoValue_); }

QPainterPath TransferFunctionEditorIsoValue::shape() const {
    QPainterPath path;
    const auto width = getSize() + 3.0;  //<! consider size of pen;
    path.addRect(QRectF(QPointF(-0.5 * width, -0.5 * width), QSizeF(width, width)));

    // add box of vertical line
    double verticalScaling = 1.0;
    if (auto tfe = qobject_cast<TransferFunctionEditor*>(scene())) {
        const auto zoom = tfe->getTransferFunctionProperty()->getZoomV();
        verticalScaling = 1.0 / (zoom.y - zoom.x);
    }
    QRectF rect = scene()->sceneRect();
    path.moveTo(QPointF(0.0, -rect.height() * verticalScaling));
    path.lineTo(QPointF(0.0, rect.height() * verticalScaling));

    return path;
}

}  // namespace inviwo
