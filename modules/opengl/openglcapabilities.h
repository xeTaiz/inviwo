/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.6b
 *
 * Copyright (c) 2013-2014 Inviwo Foundation
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
 * Main file author: Erik Sund�n
 *
 *********************************************************************************/

#ifndef IVW_OPENGLCAPABILITIES_H
#define IVW_OPENGLCAPABILITIES_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/util/capabilities.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API OpenGLCapabilities : public Capabilities  {
public:
    class IVW_MODULE_OPENGL_API GLSLShaderVersion {
    public:
        GLSLShaderVersion();
        GLSLShaderVersion(int num);
        GLSLShaderVersion(int num, std::string pro);

        std::string getProfile();
        int getVersion();
        std::string getVersionAsString();
        std::string getVersionAndProfileAsString();

        bool hasProfile();

    private:
        int number_;
        std::string profile_;
    };

    enum GlVendor {
        NVIDIA,
        AMD,
        INTEL,
        UNKNOWN
    };

    OpenGLCapabilities();
    virtual ~OpenGLCapabilities();

    void printInfo();
    void printDetailedInfo();

    bool canAllocate(glm::u64 dataSize, glm::u8 percentageOfAvailableMemory = 100);
    uvec3 calculateOptimalBrickSize(uvec3 dimensions, size_t formatSizeInBytes, glm::u8 percentageOfAvailableMemory = 100);

    bool isExtensionSupported(const char*);
    bool isSupported(const char*);

    bool isTexturesSupported();
    bool is3DTexturesSupported();
    bool isTextureArraysSupported();
    bool isFboSupported();
    bool isShadersSupported();
    bool isShadersSupportedARB();

    GLSLShaderVersion getCurrentShaderVersion();
    std::string getCurrentGlobalGLSLHeader();
    std::string getCurrentGlobalGLSLVertexDefines();
    std::string getCurrentGlobalGLSLFragmentDefines();

    glm::u64 getCurrentAvailableTextureMem() throw (Exception);
    glm::u64 getTotalAvailableTextureMem() throw (Exception);

    int getMaxProgramLoopCount();
    int getMaxTexSize();
    int getMax3DTexSize();
    int getMaxArrayTexSize();
    int getMaxColorAttachments();
    int getNumTexUnits();

    bool setPreferredGLSLProfile(std::string, bool);

protected:
    void retrieveStaticInfo();
    void retrieveDynamicInfo();

    void rebuildGLSLHeader();
    void rebuildGLSLVertexDefines();
    void rebuildGLSLFragmentDefines();

    void addShaderVersion(GLSLShaderVersion);
    void addShaderVersionIfEqualOrLower(GLSLShaderVersion, int);
    void parseAndAddShaderVersion(std::string, int);
    int parseAndRetrieveVersion(std::string);

private:
    GlVendor glVendor_;

    std::string glVendorStr_;
    std::string glRenderStr_;
    std::string glVersionStr_;
    std::string glslVersionStr_;

    //GLSL
    bool shadersAreSupported_;
    bool shadersAreSupportedARB_;
    int maxProgramLoopCount_;

    size_t currentGlobalGLSLVersionIdx_;
    std::string currentGlobalGLSLHeader_;
    std::string currentGlobalGLSLVertexDefines_;
    std::string currentGlobalGLSLFragmentDefines_;
    std::string preferredGLSLProfile_;
    std::vector<GLSLShaderVersion> supportedShaderVersions_;

    //Texturing
    bool texSupported_;
    bool tex3DSupported_;
    bool texArraySupported_;
    bool fboSupported_;
    int maxTexSize_;
    int max3DTexSize_;
    int maxArrayTexSize_;
    int maxColorAttachments_;
    int numTexUnits_;
};

} // namespace

#endif // IVW_OPENGLCAPABILITIES_H
