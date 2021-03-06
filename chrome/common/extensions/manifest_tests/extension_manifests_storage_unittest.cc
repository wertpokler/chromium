// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/extensions/manifest_tests/extension_manifest_test.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "chrome/common/extensions/extension_manifest_constants.h"

namespace keys = extension_manifest_keys;

TEST_F(ExtensionManifestTest, StorageAPIManifestVersionAvailability) {
  DictionaryValue base_manifest;
  {
    base_manifest.SetString(keys::kName, "test");
    base_manifest.SetString(keys::kVersion, "0.1");
    ListValue* permissions = new ListValue();
    permissions->Append(Value::CreateStringValue("storage"));
    base_manifest.Set(keys::kPermissions, permissions);
  }

  std::string kManifestVersionError =
      "'storage' requires manifest version of at least 2.";

  // Extension with no manifest version cannot use storage API.
  {
    Manifest manifest(&base_manifest, "test");
    scoped_refptr<Extension> extension = LoadAndExpectSuccess(manifest);
    if (extension.get()) {
      std::vector<std::string> warnings;
      warnings.push_back(kManifestVersionError);
      EXPECT_EQ(warnings, extension->install_warnings());
    }
  }

  // Extension with manifest version 1 cannot use storage API.
  {
    DictionaryValue manifest_with_version;
    manifest_with_version.SetInteger(keys::kManifestVersion, 1);
    manifest_with_version.MergeDictionary(&base_manifest);

    Manifest manifest(&manifest_with_version, "test");
    scoped_refptr<Extension> extension = LoadAndExpectSuccess(manifest);
    if (extension.get()) {
      std::vector<std::string> warnings;
      warnings.push_back(kManifestVersionError);
      EXPECT_EQ(warnings, extension->install_warnings());
    }
  }

  // Extension with manifest version 2 *can* use storage API.
  {
    DictionaryValue manifest_with_version;
    manifest_with_version.SetInteger(keys::kManifestVersion, 2);
    manifest_with_version.MergeDictionary(&base_manifest);

    Manifest manifest(&manifest_with_version, "test");
    scoped_refptr<Extension> extension = LoadAndExpectSuccess(manifest);
    if (extension.get()) {
      std::vector<std::string> empty;
      EXPECT_EQ(empty, extension->install_warnings());
    }
  }
}
