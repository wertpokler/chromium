// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dbus/property.h"

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "dbus/bus.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"
#include "dbus/property.h"
#include "dbus/test_service.h"
#include "testing/gtest/include/gtest/gtest.h"

// The property test exerises the asynchronous APIs in PropertySet and
// Property<>.
class PropertyTest : public testing::Test {
 public:
  PropertyTest() {
  }

  struct Properties : public dbus::PropertySet {
    dbus::Property<std::string> name;
    dbus::Property<int16> version;
    dbus::Property<std::vector<std::string> > methods;
    dbus::Property<std::vector<dbus::ObjectPath> > objects;

    Properties(dbus::ObjectProxy* object_proxy,
               PropertyChangedCallback property_changed_callback)
        : dbus::PropertySet(object_proxy,
                            "org.chromium.TestService",
                            property_changed_callback) {
      RegisterProperty("Name", &name);
      RegisterProperty("Version", &version);
      RegisterProperty("Methods", &methods);
      RegisterProperty("Objects", &objects);
    }
  };

  virtual void SetUp() {
    // Make the main thread not to allow IO.
    base::ThreadRestrictions::SetIOAllowed(false);

    // Start the D-Bus thread.
    dbus_thread_.reset(new base::Thread("D-Bus Thread"));
    base::Thread::Options thread_options;
    thread_options.message_loop_type = MessageLoop::TYPE_IO;
    ASSERT_TRUE(dbus_thread_->StartWithOptions(thread_options));

    // Start the test service, using the D-Bus thread.
    dbus::TestService::Options options;
    options.dbus_thread_message_loop_proxy = dbus_thread_->message_loop_proxy();
    test_service_.reset(new dbus::TestService(options));
    ASSERT_TRUE(test_service_->StartService());
    ASSERT_TRUE(test_service_->WaitUntilServiceIsStarted());
    ASSERT_TRUE(test_service_->HasDBusThread());

    // Create the client, using the D-Bus thread.
    dbus::Bus::Options bus_options;
    bus_options.bus_type = dbus::Bus::SESSION;
    bus_options.connection_type = dbus::Bus::PRIVATE;
    bus_options.dbus_thread_message_loop_proxy =
        dbus_thread_->message_loop_proxy();
    bus_ = new dbus::Bus(bus_options);
    object_proxy_ = bus_->GetObjectProxy(
        "org.chromium.TestService",
        dbus::ObjectPath("/org/chromium/TestObject"));
    ASSERT_TRUE(bus_->HasDBusThread());

    // Create the properties structure
    properties_ = new Properties(object_proxy_,
                                 base::Bind(&PropertyTest::OnPropertyChanged,
                                            base::Unretained(this)));
    properties_->ConnectSignals();
    properties_->GetAll();
  }

  virtual void TearDown() {
    bus_->ShutdownOnDBusThreadAndBlock();

    // Shut down the service.
    test_service_->ShutdownAndBlock();

    // Reset to the default.
    base::ThreadRestrictions::SetIOAllowed(true);

    // Stopping a thread is considered an IO operation, so do this after
    // allowing IO.
    test_service_->Stop();
  }

  // Generic callback, bind with a string |id| for passing to
  // WaitForCallback() to ensure the callback for the right method is
  // waited for.
  void PropertyCallback(const std::string& id, bool success) {
    last_callback_ = id;
    message_loop_.Quit();
  }

 protected:
  // Called when a property value is updated.
  void OnPropertyChanged(const std::string& name) {
    updated_properties_.push_back(name);
    message_loop_.Quit();
  }

  // Waits for the given number of updates.
  void WaitForUpdates(size_t num_updates) {
    while (updated_properties_.size() < num_updates)
      message_loop_.Run();
    for (size_t i = 0; i < num_updates; ++i)
      updated_properties_.erase(updated_properties_.begin());
  }

  // Name, Version, Methods, Objects
  static const int kExpectedSignalUpdates = 4;

  // Waits for initial values to be set.
  void WaitForGetAll() {
    WaitForUpdates(kExpectedSignalUpdates);
  }

  // Waits for the callback. |id| is the string bound to the callback when
  // the method call is made that identifies it and distinguishes from any
  // other; you can set this to whatever you wish.
  void WaitForCallback(const std::string& id) {
    while (last_callback_ != id) {
      message_loop_.Run();
    }
  }

  MessageLoop message_loop_;
  scoped_ptr<base::Thread> dbus_thread_;
  scoped_refptr<dbus::Bus> bus_;
  dbus::ObjectProxy* object_proxy_;
  Properties* properties_;
  scoped_ptr<dbus::TestService> test_service_;
  // Properties updated.
  std::vector<std::string> updated_properties_;
  // Last callback received.
  std::string last_callback_;
};

TEST_F(PropertyTest, InitialValues) {
  WaitForGetAll();

  EXPECT_EQ("TestService", properties_->name.value());
  EXPECT_EQ(10, properties_->version.value());

  std::vector<std::string> methods = properties_->methods.value();
  ASSERT_EQ(4U, methods.size());
  EXPECT_EQ("Echo", methods[0]);
  EXPECT_EQ("SlowEcho", methods[1]);
  EXPECT_EQ("AsyncEcho", methods[2]);
  EXPECT_EQ("BrokenMethod", methods[3]);

  std::vector<dbus::ObjectPath> objects = properties_->objects.value();
  ASSERT_EQ(1U, objects.size());
  EXPECT_EQ(dbus::ObjectPath("/TestObjectPath"), objects[0]);
}

TEST_F(PropertyTest, UpdatedValues) {
  WaitForGetAll();

  // Update the value of the "Name" property, this value should not change.
  properties_->name.Get(base::Bind(&PropertyTest::PropertyCallback,
                                   base::Unretained(this),
                                   "Name"));
  WaitForCallback("Name");
  WaitForUpdates(1);

  EXPECT_EQ("TestService", properties_->name.value());

  // Update the value of the "Version" property, this value should be changed.
  properties_->version.Get(base::Bind(&PropertyTest::PropertyCallback,
                                      base::Unretained(this),
                                      "Version"));
  WaitForCallback("Version");
  WaitForUpdates(1);

  EXPECT_EQ(20, properties_->version.value());

  // Update the value of the "Methods" property, this value should not change
  // and should not grow to contain duplicate entries.
  properties_->methods.Get(base::Bind(&PropertyTest::PropertyCallback,
                                      base::Unretained(this),
                                      "Methods"));
  WaitForCallback("Methods");
  WaitForUpdates(1);

  std::vector<std::string> methods = properties_->methods.value();
  ASSERT_EQ(4U, methods.size());
  EXPECT_EQ("Echo", methods[0]);
  EXPECT_EQ("SlowEcho", methods[1]);
  EXPECT_EQ("AsyncEcho", methods[2]);
  EXPECT_EQ("BrokenMethod", methods[3]);

  // Update the value of the "Objects" property, this value should not change
  // and should not grow to contain duplicate entries.
  properties_->objects.Get(base::Bind(&PropertyTest::PropertyCallback,
                                      base::Unretained(this),
                                      "Objects"));
  WaitForCallback("Objects");
  WaitForUpdates(1);

  std::vector<dbus::ObjectPath> objects = properties_->objects.value();
  ASSERT_EQ(1U, objects.size());
  EXPECT_EQ(dbus::ObjectPath("/TestObjectPath"), objects[0]);
}

TEST_F(PropertyTest, Get) {
  WaitForGetAll();

  // Ask for the new Version property.
  properties_->version.Get(base::Bind(&PropertyTest::PropertyCallback,
                                      base::Unretained(this),
                                      "Get"));
  WaitForCallback("Get");

  // Make sure we got a property update too.
  WaitForUpdates(1);

  EXPECT_EQ(20, properties_->version.value());
}

TEST_F(PropertyTest, Set) {
  WaitForGetAll();

  // Set a new name.
  properties_->name.Set("NewService",
                        base::Bind(&PropertyTest::PropertyCallback,
                                   base::Unretained(this),
                                   "Set"));
  WaitForCallback("Set");

  // TestService sends a property update.
  WaitForUpdates(1);

  EXPECT_EQ("NewService", properties_->name.value());
}
