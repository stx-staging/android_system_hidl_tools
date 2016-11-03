/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Scope.h"

#include "Interface.h"

#include <android-base/logging.h>
#include <hidl-util/Formatter.h>
#include <vector>

namespace android {

Scope::Scope(const char *localName,
        const Location &location)
    : NamedType(localName, location) {
}
Scope::~Scope(){}

bool Scope::addType(NamedType *type, std::string *errorMsg) {
    const std::string &localName = type->localName();

    auto it = mTypeIndexByName.find(localName);

    if (it != mTypeIndexByName.end()) {
        *errorMsg = "A type named '";
        (*errorMsg) += localName;
        (*errorMsg) += "' is already declared in the  current scope.";

        return false;
    }

    size_t index = mTypes.size();
    mTypes.push_back(type);
    mTypeIndexByName[localName] = index;

    return true;
}

NamedType *Scope::lookupType(const FQName &fqName) const {
    CHECK(fqName.package().empty() && fqName.version().empty());
    if (!fqName.valueName().empty()) {
        LOG(WARNING) << fqName.string() << " does not refer to a type.";
        return nullptr;
    }
    std::vector<std::string> names = fqName.names();
    CHECK_GT(names.size(), 0u);
    auto it = mTypeIndexByName.find(names[0]);

    if (it == mTypeIndexByName.end()) {
        return nullptr;
    }

    NamedType *outerType = mTypes[it->second];
    if (names.size() == 1) {
        return outerType;
    }
    if (!outerType->isScope()) {
        // more than one names, but the first name is not a scope
        return nullptr;
    }
    Scope *outerScope = static_cast<Scope *>(outerType);
    // *slowly* pop first element
    names.erase(names.begin());
    FQName innerName(names);
    return outerScope->lookupType(innerName);
}

LocalIdentifier *Scope::lookupIdentifier(const std::string & /*name*/) const {
    return NULL;
}

bool Scope::isScope() const {
    return true;
}

Interface *Scope::getInterface() const {
    if (mTypes.size() == 1 && mTypes[0]->isInterface()) {
        return static_cast<Interface *>(mTypes[0]);
    }

    return NULL;
}

bool Scope::containsSingleInterface(std::string *ifaceName) const {
    Interface *iface = getInterface();

    if (iface != NULL) {
        *ifaceName = iface->localName();
        return true;
    }

    return false;
}

bool Scope::containsInterfaces() const {
    for (const NamedType *type : mTypes) {
        if (type->isInterface()) {
            return true;
        }
    }

    return false;
}

status_t Scope::emitTypeDeclarations(Formatter &out) const {
    for (size_t i = 0; i < mTypes.size(); ++i) {
        status_t err = mTypes[i]->emitTypeDeclarations(out);

        if (err != OK) {
            return err;
        }
    }

    return OK;
}

status_t Scope::emitGlobalTypeDeclarations(Formatter &out) const {
    for (size_t i = 0; i < mTypes.size(); ++i) {
        status_t err = mTypes[i]->emitGlobalTypeDeclarations(out);

        if (err != OK) {
            return err;
        }
    }

    return OK;
}

status_t Scope::emitJavaTypeDeclarations(
        Formatter &out, bool atTopLevel) const {
    for (size_t i = 0; i < mTypes.size(); ++i) {
        status_t err = mTypes[i]->emitJavaTypeDeclarations(out, atTopLevel);

        if (err != OK) {
            return err;
        }
    }

    return OK;
}

status_t Scope::emitTypeDefinitions(
        Formatter &out, const std::string prefix) const {
    for (size_t i = 0; i < mTypes.size(); ++i) {
        status_t err = mTypes[i]->emitTypeDefinitions(out, prefix);

        if (err != OK) {
            return err;
        }
    }

    return OK;
}

const std::vector<NamedType *> &Scope::getSubTypes() const {
    return mTypes;
}

status_t Scope::emitVtsTypeDeclarations(Formatter &out) const {
    for (size_t i = 0; i < mTypes.size(); ++i) {
        status_t status = mTypes[i]->emitVtsTypeDeclarations(out);
        if (status != OK) {
            return status;
        }
    }
    return OK;
}

bool Scope::isJavaCompatible() const {
    for (const auto &type : mTypes) {
        if (!type->isJavaCompatible()) {
            return false;
        }
    }

    return true;
}

void Scope::appendToExportedTypesVector(
        std::vector<const Type *> *exportedTypes) const {
    for (const NamedType *type : mTypes) {
        type->appendToExportedTypesVector(exportedTypes);
    }
}

LocalIdentifier::LocalIdentifier(){}
LocalIdentifier::~LocalIdentifier(){}

bool LocalIdentifier::isEnumValue() const {
    return false;
}

}  // namespace android

