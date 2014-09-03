/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.6b
 *
 * Copyright (c) 2012-2014 Inviwo Foundation
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
 * Main file authors: Erik Sund�n, Timo Ropinski
 *
 *********************************************************************************/

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/imageio.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/datetime.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/network/processornetworkevaluator.h>

namespace inviwo {

CanvasProcessor::CanvasProcessor()
    : Processor()
    , inport_("inport")
    , dimensions_("dimensions", "Dimensions", ivec2(256,256), ivec2(128,128), ivec2(4096,4096), ivec2(1,1), PropertyOwner::VALID)
    , enableCustomInputDimensions_("enableCustomInputDimensions", "Enable Custom Input Dimensions", false, PropertyOwner::VALID)
    , customInputDimensions_("customInputDimensions", "Input Image Dimensions", ivec2(256,256), ivec2(128,128), ivec2(4096,4096), ivec2(1,1), PropertyOwner::VALID)
    , keepAspectRatio_("keepAspectRatio", "Keep Aspect Ratio", true, PropertyOwner::VALID)
    , aspectRatioScaling_("aspectRatioScaling", "Aspect Ratio Scaling", 1.f, 0.25f, 4.f, 0.01f, PropertyOwner::VALID)
    , visibleLayer_("visibleLayer", "Visible Layer")
    , saveLayerDirectory_("layerDir", "Output Directory", "" , "image")
    , saveLayerButton_("saveLayer", "Save Image Layer", PropertyOwner::VALID)
    , canvas_(NULL)
    , disableResize_(false)
    , queuedRequest_(false)
{
    addPort(inport_);
    dimensions_.onChange(this, &CanvasProcessor::resizeCanvas);
    addProperty(dimensions_);
    enableCustomInputDimensions_.onChange(this, &CanvasProcessor::sizeSchemeChanged);
    addProperty(enableCustomInputDimensions_);
    customInputDimensions_.onChange(this, &CanvasProcessor::sizeSchemeChanged);
    customInputDimensions_.setVisible(false);
    addProperty(customInputDimensions_);
    keepAspectRatio_.onChange(this, &CanvasProcessor::ratioChanged);
    keepAspectRatio_.setVisible(false);
    addProperty(keepAspectRatio_);
    aspectRatioScaling_.onChange(this, &CanvasProcessor::ratioChanged);
    aspectRatioScaling_.setVisible(false);
    addProperty(aspectRatioScaling_);
    dimensions_.setGroupID("inputSize");
    enableCustomInputDimensions_.setGroupID("inputSize");
    keepAspectRatio_.setGroupID("inputSize");
    aspectRatioScaling_.setGroupID("inputSize");
    customInputDimensions_.setGroupID("inputSize");
    Property::setGroupDisplayName("inputSize", "Input Dimension Parameters");

    visibleLayer_.addOption("color", "Color layer", COLOR_LAYER);
    visibleLayer_.addOption("depth", "Depth layer", DEPTH_LAYER);
    visibleLayer_.addOption("picking", "Picking layer", PICKING_LAYER);
    visibleLayer_.set(COLOR_LAYER);
    addProperty(visibleLayer_);
    addProperty(saveLayerDirectory_);
    saveLayerButton_.onChange(this, &CanvasProcessor::saveImageLayer);
    addProperty(saveLayerButton_);

    setAllPropertiesCurrentStateAsDefault();
}


CanvasProcessor::~CanvasProcessor() {
    if (processorWidget_) {
        processorWidget_->setProcessor(NULL);
    }
}

void CanvasProcessor::initialize() {
    Processor::initialize();
}

void CanvasProcessor::deinitialize() {
    if (processorWidget_)
        processorWidget_->hide();

    if(canvas_ && (!canvas_->getProcessorNetworkEvaluator() || canvas_ != canvas_->getProcessorNetworkEvaluator()->getDefaultRenderContext())){
        delete canvas_;
        canvas_ = NULL;
    }

    Processor::deinitialize();
}

void CanvasProcessor::setCanvas(Canvas* canvas) { 
    canvas_ = canvas; 
}

Canvas* CanvasProcessor::getCanvas() const { 
    return canvas_; 
}

void CanvasProcessor::resizeCanvas() {
    if (!disableResize_) {
        if (processorWidget_){
            uvec2 size = dimensions_.get();
            processorWidget_->setDimension(size);
        }
    }
}

void CanvasProcessor::sizeSchemeChanged() {
    customInputDimensions_.setVisible(enableCustomInputDimensions_.get());
    customInputDimensions_.setReadOnly(keepAspectRatio_.get());
    keepAspectRatio_.setVisible(enableCustomInputDimensions_.get());
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_.get() && keepAspectRatio_.get());

    if(canvas_){
        if(enableCustomInputDimensions_.get()){
            canvas_->resize(uvec2(dimensions_.get()), uvec2(customInputDimensions_.get()));
        }
        else{
            canvas_->resize(uvec2(dimensions_.get()), uvec2(dimensions_.get()));
        }
    }
}

void CanvasProcessor::ratioChanged(){
    customInputDimensions_.setReadOnly(keepAspectRatio_.get());
    keepAspectRatio_.setVisible(enableCustomInputDimensions_.get());
    aspectRatioScaling_.setVisible(enableCustomInputDimensions_.get() && keepAspectRatio_.get());

    if(enableCustomInputDimensions_.get()){
        if(keepAspectRatio_.get()){
            ivec2 size = dimensions_.get();

            int maxDim, minDim;

            if(size.x >= size.y){
                maxDim = 0;
                minDim = 1;
            }
            else{
                maxDim = 1;
                minDim = 0;
            }

            float ratio = static_cast<float>(size[minDim])/static_cast<float>(size[maxDim]);
            size[maxDim] = static_cast<int>(static_cast<float>(size[maxDim])*aspectRatioScaling_.get());
            size[minDim] = static_cast<int>(static_cast<float>(size[maxDim])*ratio);

            customInputDimensions_.set(size);
        }
        else if(canvas_){
            canvas_->resize(uvec2(dimensions_.get()), uvec2(customInputDimensions_.get()));
        }
    }
}

void CanvasProcessor::setCanvasSize(ivec2 dim) {
    disableResize_ = true;
    dimensions_.set(dim);

    sizeSchemeChanged();

    disableResize_ = false;
}

ivec2 CanvasProcessor::getCanvasSize() const {
    return dimensions_.get();
}

void CanvasProcessor::saveImageLayer() {
    std::string snapshotPath(saveLayerDirectory_.get() + "/" + toLower(getIdentifier()) + "-" + currentDateTime() + ".png");
    saveImageLayer(snapshotPath.c_str());
}

void CanvasProcessor::saveImageLayer(const char* snapshotPath) {
    if(!inport_.hasData()) return;
    const Image* image = inport_.getData();
    const Layer* layer = image->getLayer(static_cast<LayerType>(visibleLayer_.get()));
    ImageIO::saveLayer(snapshotPath, layer);
}


std::vector<unsigned char>* CanvasProcessor::getImageLayerAsCodedBuffer(const std::string& type) {
    if (!inport_.hasData()) return NULL;
    const Image* image = inport_.getData();
    const Layer* layer = image->getLayer(static_cast<LayerType>(visibleLayer_.get()));
    return ImageIO::saveLayerToBuffer(type.c_str(), layer);
}


void CanvasProcessor::process() {
    Processor::process();
    canvas_->activate();
}

void CanvasProcessor::invalidate(PropertyOwner::InvalidationLevel invalidationLevel,
                                 Property* modifiedProperty) {
    Processor::invalidate(invalidationLevel, modifiedProperty);
}

void CanvasProcessor::triggerQueuedEvaluation() {
    if (queuedRequest_) {
        performEvaluateRequest();
        queuedRequest_ = false;
    }
}

void CanvasProcessor::performEvaluationAtNextShow() {
    queuedRequest_ = true;
}

void CanvasProcessor::performEvaluateRequest() {
    if (canvas_ && canvas_->getProcessorNetworkEvaluator()) {
        if (processorWidget_) {
            if (processorWidget_->getVisibilityMetaData())
                notifyObserversRequestEvaluate(this);
            else
                performEvaluationAtNextShow();
        }
        else
            notifyObserversRequestEvaluate(this);
    }
}

bool CanvasProcessor::isReady() const {
    return Processor::isReady() && processorWidget_ && processorWidget_->isVisible();
}

} // namespace
