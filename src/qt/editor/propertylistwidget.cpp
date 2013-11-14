#include <inviwo/qt/editor/propertylistwidget.h>
#include <inviwo/core/properties/propertywidgetfactory.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QSettings>


namespace inviwo {

PropertyListWidget* PropertyListWidget::propertyListWidget_ = 0;

PropertyListWidget::PropertyListWidget(QWidget* parent) : InviwoDockWidget(tr("Properties"), parent), VoidObservable(){
    setObjectName("ProcessorListWidget");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    propertyListWidget_ = this;
    multiSelect_ = false;

    scrollArea_ = new QScrollArea(propertyListWidget_);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(300);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea_->setFrameShape(QFrame::NoFrame);

    listWidget_ = new QWidget();
    listWidgetLayout_ = new QVBoxLayout(listWidget_);
    listWidgetLayout_->setAlignment(Qt::AlignTop);

    scrollArea_->setWidget(listWidget_);
    setWidget(scrollArea_);

    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("PropertyListwidget");
    developerViewMode_ = settings.value("developerViewMode",true).toBool();
    applicationViewMode_ = settings.value("applicationViewMode",false).toBool();
    settings.endGroup();
}

PropertyListWidget::~PropertyListWidget() {}

void PropertyListWidget::showProcessorProperties(std::vector<Processor*> processors) {

    multiSelect_ = true;
    for (size_t i=0; i<processors.size(); i++) {
        addProcessorPropertiesToLayout(processors[i]);
    }
    if (developerViewMode_)
        setDeveloperViewMode(true);
    if (applicationViewMode_)
        setApplicationViewMode(true);
}

void PropertyListWidget::showProcessorProperties(Processor* processor) {
    // check if processor widget has been already generated
    std::map<std::string, QWidget*>::iterator it = propertyWidgetMap_.find(processor->getIdentifier());
    QWidget* processorPropertyWidget = 0;
    if (it != propertyWidgetMap_.end()) {
        // property widget has already been created and stored in the map
        processorPropertyWidget = it->second;
    } else {
        processorPropertyWidget = createNewProcessorPropertiesItem(processor);
    }
    if (processorPropertyWidget) {
        removeAllProcessorProperties();
        processorPropertyWidget->setVisible(true);
        if (multiSelect_) {
            CollapsiveGroupBoxWidgetQt* groupBox = new CollapsiveGroupBoxWidgetQt(processor->getIdentifier(),processor->getIdentifier());
            groupBox->addWidget(processorPropertyWidget);
            listWidgetLayout_->addWidget(groupBox);
            groupBox->hide();
        }else{
            listWidgetLayout_->addWidget(processorPropertyWidget);
        }
    }
}

void PropertyListWidget::addProcessorPropertiesToLayout(Processor* processor) {
    // check if processor widget has been already generated
    std::map<std::string, QWidget*>::iterator it = propertyWidgetMap_.find(processor->getIdentifier());
    QWidget* processorPropertyWidget = 0;
    if (it != propertyWidgetMap_.end()) {
        // property widget has already been created and stored in the map
        processorPropertyWidget = it->second;
    } else {
        processorPropertyWidget = createNewProcessorPropertiesItem(processor); 
    }

    if (processorPropertyWidget) {
        processorPropertyWidget->setVisible(true);
        if (multiSelect_) {
            CollapsiveGroupBoxWidgetQt* groupBox = new CollapsiveGroupBoxWidgetQt(processor->getIdentifier(),processor->getIdentifier());
            groupBox->addWidget(processorPropertyWidget);
            listWidgetLayout_->addWidget(groupBox);
            groupBox->hide();
        }else{
            listWidgetLayout_->addWidget(processorPropertyWidget);
        }
    }
}

void PropertyListWidget::removeAllProcessorProperties() {
    while (QLayoutItem* item = listWidgetLayout_->takeAt(0)) {
        QWidget* processorPropertyWidget = item->widget();
        listWidgetLayout_->removeWidget(processorPropertyWidget);
        processorPropertyWidget->setVisible(false);
    }
}

QWidget* PropertyListWidget::createNewProcessorPropertiesItem(Processor* processor) {
    // create property widget and store it in the map
    QWidget* processorPropertyWidget = new QWidget(this, Qt::WindowStaysOnBottomHint);

    QVBoxLayout* vLayout = new QVBoxLayout(processorPropertyWidget);
    vLayout->setAlignment(Qt::AlignTop);
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->setContentsMargins(0,0,0,0);

    QLabel* processorLabel = new QLabel(QString::fromStdString(processor->getIdentifier()));
    processorLabel->setAlignment(Qt::AlignCenter);
    processorLabel->setAutoFillBackground(true);
    processorLabel->setFrameStyle(QFrame::StyledPanel);

    vLayout->addWidget(processorLabel);
    properties_ = processor->getProperties();
    std::vector<Property*> addedProperties;
    for (size_t i=0; i<properties_.size(); i++) {
        Property* curProperty = properties_[i];
        // check if the property is already added
        if(std::find(addedProperties.begin(),addedProperties.end(),curProperty) != addedProperties.end())
            continue;
        // add to group box if one is assigned to the property
        else if (curProperty->getGroupID()!="") {
            CollapsiveGroupBoxWidgetQt* group = new CollapsiveGroupBoxWidgetQt(curProperty->getGroupID(),curProperty->getGroupDisplayName());
            // add all the properties with the same group assigned
            for (size_t k=0; k<properties_.size(); k++){
                Property* tmpProperty = properties_[k];
                if (curProperty->getGroupID() == tmpProperty->getGroupID()) {
                    group->addProperty(tmpProperty);
                    addedProperties.push_back(tmpProperty);
                }
            }
            group->generatePropertyWidgets();

            vLayout->addWidget(group);

        }
        else {
            PropertyWidgetQt* propertyWidget = PropertyWidgetFactoryQt::getRef().create(curProperty);
            if (propertyWidget) {
              
                curProperty->registerPropertyWidget(propertyWidget);
                vLayout->addWidget(propertyWidget);

                connect(propertyWidget, SIGNAL(modified()), this, SLOT(propertyModified()));
                addedProperties.push_back(curProperty);
            }
        }
    } 

    vLayout->addStretch(1);
    propertyWidgetMap_.insert(std::make_pair(processor->getIdentifier(), processorPropertyWidget));
    setApplicationViewMode(applicationViewMode_);

    return processorPropertyWidget;
}

void PropertyListWidget::removeProcessorProperties(Processor* processor) {
    std::map<std::string, QWidget*>::iterator it = propertyWidgetMap_.find(processor->getIdentifier());
    if (it != propertyWidgetMap_.end()) { 
        QWidget* processorPropertyWidget = propertyWidgetMap_[processor->getIdentifier()];
        listWidgetLayout_->removeWidget(processorPropertyWidget);
        processorPropertyWidget->setVisible(false);
        propertyWidgetMap_.erase(processor->getIdentifier());
        delete processorPropertyWidget;
    }
}

void PropertyListWidget::propertyModified() {
    notifyObservers();
}

PropertyListWidget* PropertyListWidget::instance() {
    return propertyListWidget_;
}

void PropertyListWidget::setDeveloperViewMode( bool value  ){
    if (value) {
        InviwoApplication* inviwoApp = InviwoApplication::getPtr();
        static_cast<OptionPropertyInt*>(inviwoApp->getSettings()->getPropertyByIdentifier("viewMode"))->set(0);
        
        developerViewMode_ = value;
        applicationViewMode_ = !value;

        for (size_t i = 0; i < properties_.size(); i++ ) {
            properties_[i]->updateVisibility();
        }
        
    }
}

void PropertyListWidget::setApplicationViewMode( bool value ){
    if (value) { 
        InviwoApplication* inviwoApp = InviwoApplication::getPtr();
        dynamic_cast<OptionPropertyInt*>(inviwoApp->getSettings()->getPropertyByIdentifier("viewMode"))->set(1);
       
        applicationViewMode_ = value;
        developerViewMode_ = !value;
        for (size_t i = 0; i < properties_.size(); i++ ) {
            properties_[i]->updateVisibility();
        }
    }
}



void PropertyListWidget::saveState(){
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("PropertyListwidget");
    settings.setValue("developerViewMode", developerViewMode_);
    settings.setValue("applicationViewMode",applicationViewMode_);
    settings.endGroup();
}

PropertyVisibility::VisibilityMode PropertyListWidget::getVisibilityMode(){
    if (developerViewMode_) 
        return PropertyVisibility::DEVELOPMENT;
    else
        return PropertyVisibility::APPLICATION;
}

void PropertyListWidget::notify(){ 

}

} // namespace