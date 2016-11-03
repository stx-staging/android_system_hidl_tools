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

#ifndef INTERFACE_H_

#define INTERFACE_H_

#include "Scope.h"
#include <vector>

namespace android {

struct Method;
struct InterfaceAndMethod;

struct Interface : public Scope {
    Interface(const char *localName, const Location &location, Interface *super);

    bool addMethod(Method *method);

    bool isInterface() const override;
    bool isBinder() const override;
    bool isRootType() const { return mSuperType == nullptr; }

    const Interface *superType() const;

    Method *lookupMethod(std::string name) const;
    // Super type chain to root type.
    // First element is superType().
    std::vector<const Interface *> superTypeChain() const;
    // Super type chain to root type, including myself.
    // First element is this.
    std::vector<const Interface *> typeChain() const;

    // user defined methods (explicit definition in HAL files)
    const std::vector<Method *> &userDefinedMethods() const;
    // HIDL reserved methods (every interface has these implicitly defined)
    const std::vector<Method *> &hidlReservedMethods() const;
    // the sum of userDefinedMethods() and hidlReservedMethods().
    std::vector<Method *> methods() const;

    // userDefinedMethods() for all super type + methods()
    // The order will be as follows (in the transaction code order):
    // great-great-...-great-grand parent->userDefinedMethods()
    // ...
    // parent->userDefinedMethods()
    // this->userDefinedMethods()
    // this->hidlReservedMethods()
    std::vector<InterfaceAndMethod> allMethodsFromRoot() const;

    std::string getBaseName() const;

    std::string getCppType(
            StorageMode mode,
            std::string *extra,
            bool specifyNamespaces) const override;

    std::string getJavaType(
            std::string *extra, bool forInitializer) const override;

    void emitReaderWriter(
            Formatter &out,
            const std::string &name,
            const std::string &parcelObj,
            bool parcelObjIsPointer,
            bool isReader,
            ErrorMode mode) const override;

    void emitJavaReaderWriter(
            Formatter &out,
            const std::string &parcelObj,
            const std::string &argName,
            bool isReader) const override;

    status_t emitVtsAttributeType(Formatter &out) const override;

    status_t emitVtsAttributeDeclaration(Formatter &out) const;
    status_t emitVtsMethodDeclaration(Formatter &out) const;

    bool hasOnewayMethods() const;

    bool isJavaCompatible() const override;

private:
    Interface *mSuperType;
    std::vector<Method *> mUserMethods;
    std::vector<Method *> mReservedMethods;
    mutable bool mIsJavaCompatibleInProgress;
    Method *createDescriptorChainMethod() const;

    DISALLOW_COPY_AND_ASSIGN(Interface);
};

// An interface / method tuple.
struct InterfaceAndMethod {
    InterfaceAndMethod(const Interface *iface, Method *method)
        : mInterface(iface),
          mMethod(method) {}
    Method *method() const { return mMethod; }
    const Interface *interface() const { return mInterface; }
private:
    // do not own these objects.
    const Interface *mInterface;
    Method *mMethod;
};

}  // namespace android

#endif  // INTERFACE_H_

