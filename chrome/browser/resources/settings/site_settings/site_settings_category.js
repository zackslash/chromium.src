// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Define a global boolean for notifications (only enabled in the test class).
cr.define('settings_test', function() {
  var siteSettingsCategoryOptions =
      settings_test.siteSettingsCategoryOptions || {
    /**
     * True if property changes should fire events for testing purposes.
     * @type {boolean}
     */
    notifyPropertyChangesForTest: false,
  };
  return {siteSettingsCategoryOptions: siteSettingsCategoryOptions};
});

/**
 * @fileoverview
 * 'site-settings-category' is the polymer element for showing a certain
 * category under Site Settings.
 *
 * Example:
 *
 *   <site-settings-category prefs="{{prefs}}">
 *   </site-settings-category>
 *   ... other pages ...
 *
 * @group Chrome Settings Elements
 * @element site-settings-category
 */
Polymer({
  is: 'site-settings-category',

  behaviors: [SiteSettingsBehavior],

  properties: {
    /**
     * Preferences state.
     */
    prefs: {
      type: Object,
      notify: true,
    },

    /**
     * The current active route.
     */
    currentRoute: {
      type: Object,
      notify: true,
    },

    /**
     * Represents the state of the main toggle shown for the category. For
     * example, the Location category can be set to Block/Ask so false, in that
     * case, represents Block and true represents Ask.
     */
    categoryEnabled: {
      type: Boolean,
      notify: settings_test.siteSettingsCategoryOptions.
          notifyPropertyChangesForTest,
    },

    /**
     * The origin that was selected by the user in the dropdown list.
     */
    selectedOrigin: {
      type: String,
      notify: true,
    },

    /**
     * Whether to show the '(recommended)' label prefix for permissions.
     */
    showRecommendation: {
      type: Boolean,
      value: true,
    },
  },

  observers: [
    'onCategoryChanged_(prefs.profile.default_content_setting_values.*, ' +
        'category)',
  ],

  ready: function() {
    this.$.blockList.categorySubtype = settings.PermissionValues.BLOCK;
    this.$.allowList.categorySubtype = settings.PermissionValues.ALLOW;
  },

  /**
   * A handler for flipping the toggle value.
   * @private
   */
  onToggleChange_: function(event) {
    var prefsProxy = settings.SiteSettingsPrefsBrowserProxy.getInstance();
    switch (this.category) {
      case settings.ContentSettingsTypes.COOKIES:
      case settings.ContentSettingsTypes.JAVASCRIPT:
      case settings.ContentSettingsTypes.POPUPS:
        // "Allowed" vs "Blocked".
        prefsProxy.setDefaultValueForContentType(
            this.category,
            this.categoryEnabled ?
                settings.PermissionValues.ALLOW :
                settings.PermissionValues.BLOCK);
        break;
      case settings.ContentSettingsTypes.NOTIFICATIONS:
      case settings.ContentSettingsTypes.GEOLOCATION:
      case settings.ContentSettingsTypes.CAMERA:
      case settings.ContentSettingsTypes.MIC:
        // "Ask" vs "Blocked".
        prefsProxy.setDefaultValueForContentType(
            this.category,
            this.categoryEnabled ?
                settings.PermissionValues.ASK :
                settings.PermissionValues.BLOCK);
        break;
      case settings.ContentSettingsTypes.FULLSCREEN:
        // "Allowed" vs. "Ask first".
        prefsProxy.setDefaultValueForContentType(
          this.category,
          this.categoryEnabled ?
              settings.PermissionValues.ALLOW :
              settings.PermissionValues.ASK);
        break;
      default:
        assertNotReached();
    }
  },

  /**
   * Handles changes to the category pref and the |category| member variable.
   * @private
   */
  onCategoryChanged_: function() {
    var prefsProxy = settings.SiteSettingsPrefsBrowserProxy.getInstance();
    prefsProxy.getDefaultValueForContentType(
        this.category).then(function(enabled) {
          this.categoryEnabled = enabled;
        }.bind(this));
  },
});
