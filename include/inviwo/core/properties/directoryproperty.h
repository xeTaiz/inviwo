#ifndef IVW_DIRECTORY_PROPERTY_H
#define IVW_DIRECTORY_PROPERTY_H

#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {
/** class DirectoryProperty
*  A class for file representations.
*  Holds the value of the path to a file as a string.
*  
* @see FileProperty, StringProperty
*/

class DirectoryProperty : public TemplateProperty<std::string> {
public:
    DirectoryProperty(std::string identifier, std::string displayName, std::string value=IVW_DIR,
                 PropertyOwner::InvalidationLevel invalidationLevel=PropertyOwner::INVALID_OUTPUT, 
                 PropertySemantics::Type semantics = PropertySemantics::Default);
    virtual std::vector<std::string> getDirectoryTree()  const ;
    virtual void setDirectoryTree(std::vector<std::string> dirTree) ;
    virtual Variant getVariant();
    virtual void setVariant(const Variant&);
    virtual int getVariantType();
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    virtual std::string getClassName()  const { return "DirectoryProperty"; }
protected:
    //TODO: currently tree contains file names only.
    std::vector<std::string> directoryTree_;
};

} // namespace

#endif // IVW_DIRECTORY_PROPERTY_H