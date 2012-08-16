#include "inviwo/core/properties/property.h"
#include "inviwo/core/properties/propertyowner.h"

namespace inviwo {

PropertyOwner::PropertyOwner()
    : invalid_(true) {}

PropertyOwner::~PropertyOwner() {
    for (size_t i=0; i<properties_.size(); i++)
        properties_[i]->setOwner(0);
    properties_.clear();
}

void PropertyOwner::addProperty(Property* property) {
    // TODO: check if property with same name has been added before
    properties_.push_back(property);
    property->setOwner(this);
}

void PropertyOwner::addProperty(Property& property) {
    addProperty(&property);
}

Property* PropertyOwner::getPropertyByIdentifier(std::string identifier) {
    for (size_t i=0; i<properties_.size(); i++)
        if ((properties_[i]->getIdentifier()).compare(identifier) == 0)
            return properties_[i];
    return 0;
}

void PropertyOwner::invalidate() {
    invalid_ = true;
}

void PropertyOwner::setValid() {
    invalid_ = false;
}

bool PropertyOwner::isValid() {
    return (invalid_ == false);
}


} // namespace
