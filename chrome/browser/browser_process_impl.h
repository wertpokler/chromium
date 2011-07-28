// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// When each service is created, we set a flag indicating this. At this point,
// the service initialization could fail or succeed. This allows us to remember
// if we tried to create a service, and not try creating it over and over if
// the creation failed.

#ifndef CHROME_BROWSER_BROWSER_PROCESS_IMPL_H_
#define CHROME_BROWSER_BROWSER_PROCESS_IMPL_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/threading/non_thread_safe.h"
#include "base/timer.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/download/download_status_updater.h"
#include "chrome/browser/prefs/pref_change_registrar.h"
#include "chrome/browser/prefs/pref_member.h"
#include "chrome/browser/tab_contents/thumbnail_generator.h"
#include "content/common/notification_observer.h"
#include "content/common/notification_registrar.h"
#include "ipc/ipc_message.h"

class BrowserOnlineStateObserver;
class ChromeNetLog;
class ChromeResourceDispatcherHostDelegate;
class CommandLine;
class DevToolsProtocolHandler;
class FilePath;
class NotificationService;
class PluginDataRemover;
class RemoteDebuggingServer;
class TabCloseableStateWatcher;

namespace policy{
class BrowserPolicyConnector;
};

// Real implementation of BrowserProcess that creates and returns the services.
class BrowserProcessImpl : public BrowserProcess,
                           public base::NonThreadSafe,
                           public NotificationObserver {
 public:
  explicit BrowserProcessImpl(const CommandLine& command_line);
  virtual ~BrowserProcessImpl();

  virtual void EndSession();

  // BrowserProcess methods
  virtual ResourceDispatcherHost* resource_dispatcher_host();
  virtual MetricsService* metrics_service();
  virtual IOThread* io_thread();
  virtual base::Thread* file_thread();
  virtual base::Thread* db_thread();
  virtual base::Thread* process_launcher_thread();
  virtual base::Thread* cache_thread();
  virtual WatchDogThread* watchdog_thread();
#if defined(OS_CHROMEOS)
  virtual base::Thread* web_socket_proxy_thread();
#endif
  virtual ProfileManager* profile_manager();
  virtual PrefService* local_state();
  virtual DevToolsManager* devtools_manager();
  virtual SidebarManager* sidebar_manager();
  virtual ui::Clipboard* clipboard();
  virtual net::URLRequestContextGetter* system_request_context();
#if defined(OS_CHROMEOS)
  virtual chromeos::ProxyConfigServiceImpl*
      chromeos_proxy_config_service_impl();
#endif  // defined(OS_CHROMEOS)
  virtual ExtensionEventRouterForwarder* extension_event_router_forwarder();
  virtual NotificationUIManager* notification_ui_manager();
  virtual policy::BrowserPolicyConnector* browser_policy_connector();
  virtual IconManager* icon_manager();
  virtual ThumbnailGenerator* GetThumbnailGenerator();
  virtual AutomationProviderList* InitAutomationProviderList();
  virtual void InitDevToolsHttpProtocolHandler(
      Profile* profile,
      const std::string& ip,
      int port,
      const std::string& frontend_url);
  virtual void InitDevToolsLegacyProtocolHandler(int port);
  virtual unsigned int AddRefModule();
  virtual unsigned int ReleaseModule();
  virtual bool IsShuttingDown();
  virtual printing::PrintJobManager* print_job_manager();
  virtual printing::PrintPreviewTabController* print_preview_tab_controller();
  virtual printing::BackgroundPrintingManager* background_printing_manager();
  virtual GoogleURLTracker* google_url_tracker();
  virtual IntranetRedirectDetector* intranet_redirect_detector();
  virtual const std::string& GetApplicationLocale();
  virtual void SetApplicationLocale(const std::string& locale);
  virtual DownloadStatusUpdater* download_status_updater();
  virtual TabCloseableStateWatcher* tab_closeable_state_watcher();
  virtual BackgroundModeManager* background_mode_manager();
  virtual StatusTray* status_tray();
  virtual SafeBrowsingService* safe_browsing_service();
  virtual safe_browsing::ClientSideDetectionService*
      safe_browsing_detection_service();
  virtual bool plugin_finder_disabled() const;

  // NotificationObserver methods
  virtual void Observe(int type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

#if (defined(OS_WIN) || defined(OS_LINUX)) && !defined(OS_CHROMEOS)
  virtual void StartAutoupdateTimer();
#endif

  virtual ChromeNetLog* net_log();

  virtual prerender::PrerenderTracker* prerender_tracker();

#if defined(IPC_MESSAGE_LOG_ENABLED)
  virtual void SetIPCLoggingEnabled(bool enable);
#endif

  virtual MHTMLGenerationManager* mhtml_generation_manager();

  virtual GpuBlacklistUpdater* gpu_blacklist_updater();

 private:
  void CreateResourceDispatcherHost();
  void CreateMetricsService();

  void CreateIOThread();
  static void CleanupOnIOThread();

  void CreateFileThread();
  void CreateDBThread();
  void CreateProcessLauncherThread();
  void CreateCacheThread();
  void CreateGpuThread();
  void CreateWatchdogThread();
#if defined(OS_CHROMEOS)
  void CreateWebSocketProxyThread();
#endif
  void CreateTemplateURLService();
  void CreateProfileManager();
  void CreateWebDataService();
  void CreateLocalState();
  void CreateViewedPageTracker();
  void CreateIconManager();
  void CreateDevToolsManager();
  void CreateSidebarManager();
  void CreateGoogleURLTracker();
  void CreateIntranetRedirectDetector();
  void CreateNotificationUIManager();
  void CreateStatusTrayManager();
  void CreateTabCloseableStateWatcher();
  void CreatePrintPreviewTabController();
  void CreateBackgroundPrintingManager();
  void CreateSafeBrowsingService();
  void CreateSafeBrowsingDetectionService();
  void CreateStatusTray();
  void CreateBackgroundModeManager();

  bool IsSafeBrowsingDetectionServiceEnabled();

  void ApplyDisabledSchemesPolicy();
  void ApplyAllowCrossOriginAuthPromptPolicy();

#if defined(IPC_MESSAGE_LOG_ENABLED)
  void SetIPCLoggingEnabledForChildProcesses(bool enabled);
#endif

  bool created_resource_dispatcher_host_;
  scoped_ptr<ResourceDispatcherHost> resource_dispatcher_host_;

  bool created_metrics_service_;
  scoped_ptr<MetricsService> metrics_service_;

  bool created_io_thread_;
  scoped_ptr<IOThread> io_thread_;
#if defined(USE_X11)
  // This shares a created flag with the IO thread.
  scoped_ptr<base::Thread> background_x11_thread_;
#endif

  bool created_file_thread_;
  scoped_ptr<base::Thread> file_thread_;

  bool created_db_thread_;
  scoped_ptr<base::Thread> db_thread_;

  bool created_process_launcher_thread_;
  scoped_ptr<base::Thread> process_launcher_thread_;

  bool created_cache_thread_;
  scoped_ptr<base::Thread> cache_thread_;

  bool created_watchdog_thread_;
  scoped_ptr<WatchDogThread> watchdog_thread_;

#if defined(OS_CHROMEOS)
  bool created_web_socket_proxy_thread_;
  scoped_ptr<base::Thread> web_socket_proxy_thread_;
#endif

  bool created_profile_manager_;
  scoped_ptr<ProfileManager> profile_manager_;

  bool created_local_state_;
  scoped_ptr<PrefService> local_state_;

  bool created_icon_manager_;
  scoped_ptr<IconManager> icon_manager_;

  scoped_refptr<ExtensionEventRouterForwarder>
      extension_event_router_forwarder_;

  scoped_ptr<RemoteDebuggingServer> remote_debugging_server_;

  scoped_refptr<DevToolsProtocolHandler> devtools_legacy_handler_;

  bool created_devtools_manager_;
  scoped_refptr<DevToolsManager> devtools_manager_;

  bool created_sidebar_manager_;
  scoped_refptr<SidebarManager> sidebar_manager_;

  bool created_browser_policy_connector_;
  scoped_ptr<policy::BrowserPolicyConnector> browser_policy_connector_;

  scoped_refptr<printing::PrintPreviewTabController>
      print_preview_tab_controller_;

  scoped_ptr<printing::BackgroundPrintingManager> background_printing_manager_;

  scoped_ptr<ui::Clipboard> clipboard_;

  // Manager for desktop notification UI.
  bool created_notification_ui_manager_;
  scoped_ptr<NotificationUIManager> notification_ui_manager_;

  scoped_ptr<AutomationProviderList> automation_provider_list_;

  scoped_ptr<GoogleURLTracker> google_url_tracker_;
  scoped_ptr<IntranetRedirectDetector> intranet_redirect_detector_;

  scoped_ptr<NotificationService> main_notification_service_;

  scoped_ptr<TabCloseableStateWatcher> tab_closeable_state_watcher_;

  scoped_ptr<BackgroundModeManager> background_mode_manager_;

  scoped_ptr<StatusTray> status_tray_;

  bool created_safe_browsing_service_;
  scoped_refptr<SafeBrowsingService> safe_browsing_service_;

  bool created_safe_browsing_detection_service_;
  scoped_ptr<safe_browsing::ClientSideDetectionService>
     safe_browsing_detection_service_;

  unsigned int module_ref_count_;
  bool did_start_;

  // Ensures that all the print jobs are finished before closing the browser.
  scoped_ptr<printing::PrintJobManager> print_job_manager_;

  std::string locale_;

  bool checked_for_new_frames_;
  bool using_new_frames_;

  // This service just sits around and makes thumbnails for tabs. It does
  // nothing in the constructor so we don't have to worry about lazy init.
  ThumbnailGenerator thumbnail_generator_;

  // Download status updates (like a changing application icon on dock/taskbar)
  // are global per-application. DownloadStatusUpdater does no work in the ctor
  // so we don't have to worry about lazy initialization.
  DownloadStatusUpdater download_status_updater_;

  // Ensures that the observers of plugin/print disable/enable state
  // notifications are properly added and removed.
  PrefChangeRegistrar pref_change_registrar_;

  // Lives here so can safely log events on shutdown.
  scoped_ptr<ChromeNetLog> net_log_;

  // Ordered before resource_dispatcher_host_delegate_ due to destruction
  // ordering.
  scoped_ptr<prerender::PrerenderTracker> prerender_tracker_;

  scoped_ptr<ChromeResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  NotificationRegistrar notification_registrar_;

  scoped_refptr<MHTMLGenerationManager> mhtml_generation_manager_;

  // Monitors the state of the 'DisablePluginFinder' policy.
  BooleanPrefMember plugin_finder_disabled_pref_;

#if (defined(OS_WIN) || defined(OS_LINUX)) && !defined(OS_CHROMEOS)
  base::RepeatingTimer<BrowserProcessImpl> autoupdate_timer_;

  // Gets called by autoupdate timer to see if browser needs restart and can be
  // restarted, and if that's the case, restarts the browser.
  void OnAutoupdateTimer();
  bool CanAutorestartForUpdate() const;
  void RestartPersistentInstance();
#endif  // defined(OS_WIN) || defined(OS_LINUX)

#if defined(OS_CHROMEOS)
  scoped_refptr<chromeos::ProxyConfigServiceImpl>
      chromeos_proxy_config_service_impl_;
#endif

  // Per-process listener for online state changes.
  scoped_ptr<BrowserOnlineStateObserver> online_state_observer_;

  scoped_refptr<GpuBlacklistUpdater> gpu_blacklist_updater_;

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessImpl);
};

#endif  // CHROME_BROWSER_BROWSER_PROCESS_IMPL_H_
