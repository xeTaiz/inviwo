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

    py::class_<TFPrimitiveData>(m, "TFPrimitiveData")
        .def(py::init())
        .def(py::init<float, vec4>())
        .def(py::init([](float iso, const std::string &color) {
            return new TFPrimitiveData{iso, color::hex2rgba(color)};
        }))
        .def_readwrite("pos", &TFPrimitiveData::pos)
        .def_readwrite("color", &TFPrimitiveData::color)
        .def("__repr__", [](const TFPrimitiveData &p) {
            std::ostringstream oss;
            oss << "TFPrimitiveData: [" << p.pos << ", " << color::rgba2hex(p.color) << "]";
            return oss.str();
        });

    py::class_<IsoValueProperty, Property, PropertyPtr<IsoValueProperty>>(m, "IsoValueProperty")
        .def_property("enabled", &IsoValueProperty::getEnabled, &IsoValueProperty::setEnabled)
        .def_property("zoomH", &IsoValueProperty::getZoomH, &IsoValueProperty::setZoomH)
        .def_property("zoomV", &IsoValueProperty::getZoomV, &IsoValueProperty::setZoomV)
        .def("save", [](IsoValueProperty *ivp, std::string filename) { ivp->get().save(filename); })
        .def("load", [](IsoValueProperty *ivp, std::string filename) { ivp->get().load(filename); })
        .def("clear", [](IsoValueProperty &ivp) { ivp.get().clear(); })
        .def("add",
             [](IsoValueProperty &ivp, float value, const vec4 &color) {
                 ivp.get().add(TFPrimitiveData({value, color}));
             })
        .def("add", [](IsoValueProperty &ivp, const vec2 &pos) { ivp.get().add(pos); })
        .def("add", [](IsoValueProperty &ivp, const TFPrimitiveData &v) { ivp.get().add(v); })
        .def("add", [](IsoValueProperty &ivp,
                       const std::vector<TFPrimitiveData> &values) { ivp.get().add(values); })
        .def("setValues",
             [](IsoValueProperty &ivp, const std::vector<TFPrimitiveData> &values) {
                 ivp.get().clear();
                 ivp.get().add(values);
             })
        .def("getValues",
             [](IsoValueProperty &ivp) -> std::vector<TFPrimitiveData> { return ivp.get().get(); })
        .def("__repr__", [](const IsoValueProperty &ivp) {
            std::ostringstream oss;
            oss << "[IsoValueProperty:  " << ivp.get().size() << " isovalues";
            for (auto &p : ivp.get().getSorted()) {
                oss << "\n    " << p.pos << ", " << color::rgba2hex(p.color);
            }
            oss << "]";
            return oss.str();
        });
}

}  // namespace inviwo
