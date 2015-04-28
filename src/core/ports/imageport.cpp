/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/multidatainport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {

// Image Inport
ImageInport::ImageInport(std::string identifier, bool outportDeterminesSize)
    : DataInport<Image>(identifier)
    , requestedDimensions_(8, 8)
    , outportDeterminesSize_(outportDeterminesSize) {}

ImageInport::~ImageInport() {}

void ImageInport::connectTo(Outport* outport) {
    ResizeEvent resizeEvent(requestedDimensions_);
    ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(outport);
    imageOutport->changeDataDimensions(&resizeEvent);

    DataInport<Image>::connectTo(outport);
}

// set dimensions based on port groups
void ImageInport::changeDataDimensions(ResizeEvent* resizeEvent) {
    requestedDimensions_ = resizeEvent->size();

    // Find the image port with largest dimensions
    for (auto port : getProcessor()->getPortsInSameSet(this)) {
        auto imageOutport = dynamic_cast<const ImageOutport*>(port);

        if (imageOutport && imageOutport->isConnected()) {
            const uvec2 dim = imageOutport->getDimensions();

            // Largest outport dimensions
            if (requestedDimensions_.x * requestedDimensions_.y < dim.x * dim.y)
                requestedDimensions_ = dim;
        }
    }

    resizeEvent->setSize(requestedDimensions_);

    if (auto imageOutport = static_cast<ImageOutport*>(getConnectedOutport())) {
        imageOutport->changeDataDimensions(resizeEvent);
    }
}

uvec2 ImageInport::getDimensions() const { return requestedDimensions_; }

const Image* ImageInport::getData() const {
    if (isConnected()) {
        auto outport = static_cast<const ImageOutport*>(getConnectedOutport());
        if (isOutportDeterminingSize()) {
            return outport->getConstData();
        } else {
            return outport->getResizedImageData(requestedDimensions_);
        }
    } else {
        return nullptr;
    }
}

bool ImageInport::isOutportDeterminingSize() const { return outportDeterminesSize_; }

void ImageInport::setOutportDeterminesSize(bool outportDeterminesSize) {
    outportDeterminesSize_ = outportDeterminesSize;
}

std::string ImageInport::getContentInfo() const {
    if (hasData())
        return getData()->getDataInfo();
    else
        return getClassIdentifier() + " has no data.";
}

void ImageInport::passOnDataToOutport(ImageOutport* outport) const {
    if (hasData()) {
        const Image* img = getData();
        Image* out = outport->getData();
        if (out) img->resizeRepresentations(out, out->getDimensions());
    }
}

////////////////////////////// ImageOutport ////////////////////////////////////////////

ImageOutport::ImageOutport(std::string identifier, const DataFormatBase* format,
                           bool handleResizeEvents)
    : DataOutport<Image>(identifier)
    , dimensions_(uvec2(8, 8))
    , handleResizeEvents_(handleResizeEvents) {
 
    // create a default image
    setData(new Image(dimensions_, format));
}

ImageOutport::~ImageOutport() {}

void ImageOutport::invalidate(InvalidationLevel invalidationLevel) {
    if (invalidationLevel > VALID) {
        cache_.setInvalid();
    }
    Outport::invalidate(invalidationLevel);
}

void ImageOutport::setData(Image* data, bool ownsData /*= true*/) {
    DataOutport<Image>::setData(data, ownsData);
    dimensions_ = data_->getDimensions();
    cache_.setMaster(data);
}

void ImageOutport::setConstData(const Image* data) {
    DataOutport<Image>::setConstData(data);
    dimensions_ = data_->getDimensions();
    cache_.setMaster(data);
}

void ImageOutport::changeDataDimensions(ResizeEvent* resizeEvent) {
    // This function should check which dimensions request exists, by going through the successors
    // and checking registeredDimensions.
    // Allocates space holder, sets largest data, cleans up un-used data

    uvec2 requiredDimensions = resizeEvent->size();
    std::vector<const uvec2> registeredDimensions;

    for (auto inport : connectedInports_) {
        auto imageInport = dynamic_cast<ImageInport*>(inport);
        if (imageInport && !imageInport->isOutportDeterminingSize()) {
            util::push_back_unique(registeredDimensions, imageInport->getDimensions());
        }
    }
    util::push_back_unique(registeredDimensions, resizeEvent->size());


    // find the largest dimension.
    uvec2 newDimensions =
        *std::max_element(registeredDimensions.begin(), registeredDimensions.end(),
                          [](const uvec2& a, const uvec2& b) { return a.x * a.y < b.x * b.y; });

    if (isHandlingResizeEvents() && newDimensions != data_->getDimensions()) {  // resize data.

        data_->resize(newDimensions);
        dimensions_ = data_->getDimensions();
        cache_.setInvalid();

        ResizeEvent e(newDimensions, data_->getDimensions());
        broadcast(&e);
    }

    // remove unused image from cache
    cache_.prune(registeredDimensions);


    // Make sure that all ImageOutports in the same group (dependency set) that has the same size.
    for (auto port : getProcessor()->getPortsInSameSet(this)) {
        auto imageOutport = dynamic_cast<ImageOutport*>(port);
        if (imageOutport && imageOutport != this) {
            imageOutport->setDimensions(resizeEvent->size());
        }
    }

    // Propagate the resize event
    if (getProcessor()->propagateResizeEvent(resizeEvent, this)) {
        // Happens if propagation ended
        getProcessor()->invalidate(INVALID_OUTPUT);
    }
}

uvec2 ImageOutport::getDimensions() const { return dimensions_; }

const Image* ImageOutport::getResizedImageData(uvec2 requiredDimensions) const {
    return cache_.getImage(requiredDimensions);
}

bool ImageOutport::addResizeEventListener(EventListener* el) { return addEventListener(el); }

bool ImageOutport::removeResizeEventListener(EventListener* el) { return removeEventListener(el); }

void ImageOutport::setDimensions(const uvec2& newDimension) {
    // Set new dimensions
    DataOutport<Image>::getData()->resize(newDimension);
    dimensions_ = newDimension;
    cache_.setInvalid();
}

void ImageOutport::setHandleResizeEvents(bool handleResizeEvents) {
    handleResizeEvents_ = handleResizeEvents;
}

bool ImageOutport::isHandlingResizeEvents() const { return handleResizeEvents_ && isDataOwner(); }

}  // namespace
