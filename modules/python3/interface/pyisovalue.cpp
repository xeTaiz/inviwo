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

#include <modules/python3/interface/pyisovalue.h>

#include <modules/python3/interface/pyproperties.h>

#include <inviwo/core/datastructures/isovalue.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/util/colorconversion.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <sstream>

namespace py = pybind11;

namespace inviwo {

void exposeIsoValueData(py::module &m) {

    py::class_<IsoValueData>(m, "IsoValueData")
        .def(py::init())
        .def(py::init<float, vec4>())
        .def(py::init([](float iso, const std::string &color) {
            return new IsoValueData{iso, color::hex2rgba(color)};
        }))
        .def_readwrite("isovalue", &IsoValueData::isovalue)
        .def_readwrite("color", &IsoValueData::color)
        .def("__repr__", [](const IsoValueData &v) {
            std::ostringstream oss;
            oss << "Isovalue: [" << v.isovalue << ", " << color::rgba2hex(v.color) << "]";
            return oss.str();
        });

    py::class_<IsoValueProperty, Property, PropertyPtr<IsoValueProperty>>(m, "IsoValueProperty")
        .def_property("enabled", &IsoValueProperty::getEnabled, &IsoValueProperty::setEnabled)
        .def_property("zoomH", &IsoValueProperty::getZoomH, &IsoValueProperty::setZoomH)
        .def_property("zoomV", &IsoValueProperty::getZoomV, &IsoValueProperty::setZoomV)
        .def("save", [](IsoValueProperty *ivp, std::string filename) { ivp->get().save(filename); })
        .def("load", [](IsoValueProperty *ivp, std::string filename) { ivp->get().load(filename); })
        .def("clear", [](IsoValueProperty &ivp) { ivp.get().clearIsoValues(); })
        .def("addIsoValue", [](IsoValueProperty &ivp, float value,
                               const vec4 &color) { ivp.get().addIsoValue(value, color); })
        .def("addIsoValue",
             [](IsoValueProperty &ivp, const vec2 &pos) { ivp.get().addIsoValue(pos); })
        .def("addIsoValue",
             [](IsoValueProperty &ivp, const IsoValueData &v) { ivp.get().addIsoValue(v); })
        .def("addIsoValues",
             [](IsoValueProperty &ivp, const std::vector<IsoValueData> &values) {
                 ivp.get().addIsoValues(values);
             })
        .def("setIsoValues",
             [](IsoValueProperty &ivp, const std::vector<IsoValueData> &values) {
                 ivp.get().clearIsoValues();
                 ivp.get().addIsoValues(values);
             })
        .def("getIsoValues",
             [](IsoValueProperty &ivp) -> std::vector<IsoValueData> {
                 return ivp.get().getIsoValues();
             })
        .def("__repr__", [](const IsoValueProperty &ivp) {
            std::ostringstream oss;
            oss << "[IsoValueProperty:  " << ivp.get().getNumIsoValues() << " isovalues";
            for (auto &v : ivp.get().getSortedIsoValues()) {
                oss << "\n    " << v.isovalue << ", " << color::rgba2hex(v.color);
            }
            oss << "]";
            return oss.str();
        });
}

}  // namespace inviwo
