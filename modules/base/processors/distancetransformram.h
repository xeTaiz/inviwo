/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#ifndef IVW_DISTANCETRANSFORMRAM_H
#define IVW_DISTANCETRANSFORMRAM_H

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/clock.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {


/** \docpage{org.inviwo.DistanceTransformRAM, Distance Transform}
 * ![](org.inviwo.DistanceTransformRAM.png?classIdentifier=org.inviwo.DistanceTransformRAM)
 *
 * Computes the distance transform of a binary volume dataset using the data range as low 
 * and high value. The result is the distance from each voxel to the closest feature 
 * (high value). It uses the Saito's algorithm to compute the Euclidean distance.
 * 
 * ### Inports
 *   * __inputVolume__ Binary input volume
 * 
 * ### Outports
 *   * __outputVolume__ Scalar volume representing the distance transform (Uint16)
 * 
 * ### Properties
 *   * __Enabled__              Enables the computation. If disabled, the output is identical 
 *                              to the input volume.
 *   * __Squared Distance__     Use squared distances instead of Euclidean distance.
 *   * __Scaling Factor__       Scales the resulting distances.
 *   * __Update Distance Map__  Triggers a re-computation of the distance transform
 *
 */
class IVW_MODULE_BASE_API DistanceTransformRAM : public Processor, public ProgressBarOwner {
public:  
    enum class DataRangeMode {Diagonal, MinMax, Custom};


    DistanceTransformRAM();
    virtual ~DistanceTransformRAM();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    void updateOutport();
    void paramChanged();

    VolumeInport volumePort_;
    VolumeOutport outport_;

    std::future<std::shared_ptr<const Volume>> newVolume_;

    DoubleProperty threshold_;
    BoolProperty flip_;
    BoolProperty normalize_;
    DoubleProperty resultDistScale_; // scaling factor for distances
    BoolProperty resultSquaredDist_; // determines whether output uses squared euclidean distances
    IntSize3Property upsample_;      // Upscale the output field 
    
    DoubleMinMaxProperty dataRange_;
    TemplateOptionProperty<DataRangeMode> dataRangeMode_;
    DoubleMinMaxProperty customDataRange_;

    ButtonProperty btnForceUpdate_;

    bool distTransformDirty_;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             DistanceTransformRAM::DataRangeMode m) {
    switch (m) {
        case DistanceTransformRAM::DataRangeMode::Diagonal:
            ss << "Diagonal";
            break;
        case DistanceTransformRAM::DataRangeMode::MinMax:
            ss << "MinMax";
            break;
        case DistanceTransformRAM::DataRangeMode::Custom:
            ss << "Custom";
            break;
        default:
            break;
    }
    return ss;
}

} // namespace

#endif // IVW_DISTANCETRANSFORMRAM_H
