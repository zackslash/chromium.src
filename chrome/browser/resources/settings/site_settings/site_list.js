// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'settings-site-list' shows a list of Allowed and Blocked sites for a given
 * category.
 *
 * Example:
 *    <settings-site-list prefs="{{prefs}}"
 *        category="[[category]]">
 *    </settings-site-list>
 *
 * @group Chrome Settings Elements
 * @element settings-site-list
 */
Polymer({
  is: 'settings-site-list',

  behaviors: [PrefsBehavior, SiteSettingsBehavior],

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
     * The origin that was selected by the user in the dropdown list.
     */
    selectedOrigin: {
      type: String,
      notify: true,
    },

    /**
     * Array of sites to display in the widget.
     */
    sites_: {
      type: Array,
      value: function() { return []; },
      observer: 'onDataChanged_',
    },

    /**
     * Whether this list is for the All Sites category.
     */
    allSites: {
      type: Boolean,
      value: false,
    },

    /**
      * The type of category this widget is displaying data for. Normally
      * either ALLOW or BLOCK, representing which sites are allowed or blocked
      * respectively.
      */
    categorySubtype: {
      type: Number,
      value: -1,
    },

    /**
     * Represents the state of the main toggle shown for the category. For
     * example, the Location category can be set to Block/Ask so false, in that
     * case, represents Block and true represents Ask.
     */
    categoryEnabled: {
      type: Boolean,
      observer: 'onDataChanged_',
      value: true,
    },

    /**
     * Whether to show the Allow action in the action menu.
     */
    showAllowAction_: Boolean,

    /**
     * Whether to show the Block action in the action menu.
     */
    showBlockAction_: Boolean,

    /**
     * All possible actions in the action menu.
     */
    actions_: {
      readOnly: true,
      type: Object,
      values: {
        ALLOW: 'Allow',
        BLOCK: 'Block',
        RESET: 'Reset',
      }
    },

    i18n_: {
      readOnly: true,
      type: Object,
      value: function() {
        return {
          allowAction: loadTimeData.getString('siteSettingsActionAllow'),
          blockAction: loadTimeData.getString('siteSettingsActionBlock'),
          resetAction: loadTimeData.getString('siteSettingsActionReset'),
        };
      },
    },
  },

  observers: [
    'initialize_(prefs.profile.content_settings.exceptions.*,' +
        'category, categorySubtype, allSites)'
  ],

  /**
   * One-time initialization routines for this class.
   * @private
   */
  initialize_: function() {
    CrSettingsPrefs.initialized.then(function() {
      this.setUpActionMenu_();
      this.ensureOpened_();
    }.bind(this));

    this.populateList_();
  },

  /**
   * Ensures the widget is |opened| when needed when displayed initially.
   */
  ensureOpened_: function() {
    // Allowed list is always shown opened by default and All Sites is presented
    // all in one list (nothing closed by default).
    if (this.allSites ||
        this.categorySubtype == settings.PermissionValues.ALLOW) {
      this.$.category.opened = true;
      return;
    }

    // Block list should only be shown opened if there is nothing to show in
    // the allowed list.
    var pref = this.getPref(
        this.computeCategoryExceptionsPrefName(this.category));
    var sites = pref.value;
    for (var origin in sites) {
      var site = /** @type {{setting: number}} */(sites[origin]);
      if (site.setting == settings.PermissionValues.ALLOW)
        return;
    }

    this.$.category.opened = true;
  },

  /**
   * Handles the data changing, for example when the category is flipped from
   * ALLOW to BLOCK or sites are added to the list.
   * @private
   */
  onDataChanged_: function(newValue, oldValue) {
    this.$.category.hidden =
        !this.showSiteList_(this.sites_, this.categoryEnabled);
  },

  /**
   * Handles the expanding and collapsing of the sites list.
   * @private
   */
  onToggle_: function(e) {
    if (this.$.category.opened)
      this.$.icon.icon = 'icons:expand-less';
    else
      this.$.icon.icon = 'icons:expand-more';
  },

  /**
   * Populate the sites list for display.
   * @private
   */
  populateList_: function() {
    if (this.allSites) {
      this.sites_ = this.toSiteArray_(this.getAllSitesList_());
    } else {
      var sites = new Set();
      this.sites_ = this.toSiteArray_(
          this.appendSiteList(sites, this.category, this.categorySubtype));
    }
  },

  /**
   * Retrieves a set of all known sites (any category/setting).
   * @private
   */
  getAllSitesList_: function() {
    var sites = new Set();
    for (var type in settings.ContentSettingsTypes) {
      sites = this.appendSiteList(sites,
                                  settings.ContentSettingsTypes[type],
                                  settings.PermissionValues.ALLOW);
      sites = this.appendSiteList(sites,
                                  settings.ContentSettingsTypes[type],
                                  settings.PermissionValues.BLOCK);
    }
    return sites;
  },

  /**
   * Appends to |list| the sites for a given category and subtype.
   * @param {!Set<string>} list The site list to add to.
   * @param {number} category The category to look up.
   * @param {number} categorySubtype The category subtype to look up.
   * @return {!Set<string>} The list of sites found.
   */
  appendSiteList: function(list, category, categorySubtype) {
    var pref = this.getPref(
        this.computeCategoryExceptionsPrefName(category));
    var sites = pref.value;
    for (var origin in sites) {
      var site = /** @type {{setting: number}} */(sites[origin]);
      if (site.setting == categorySubtype) {
        var tokens = origin.split(',');
        list.add(tokens[0]);
      }
    }
    return list;
  },

  /**
   * Converts a set of sites to an ordered array, sorted by site name then
   * protocol.
   * @param {!Set<string>} sites A set of sites to sort and convert to an array.
   * @private
   */
  toSiteArray_: function(sites) {
    var list = [...sites];
    list.sort(function(a, b) {
      var url1 = /** @type {{host: string}} */(new URL(a));
      var url2 = /** @type {{host: string}} */(new URL(b));
      var result = url1.host.localeCompare(url2.host);
      if (result == 0)
        return url1.protocol.localeCompare(url2.protocol);
      return result;
    });
    return list;
  },

  /**
   * Setup the values to use for the action menu.
   * @private
   */
  setUpActionMenu_: function() {
    this.showAllowAction_ =
        this.categorySubtype == settings.PermissionValues.BLOCK;
    this.showBlockAction_ =
        this.categorySubtype == settings.PermissionValues.ALLOW &&
        this.category != settings.ContentSettingsTypes.FULLSCREEN;
  },

  /**
   * A handler for selecting a site (by clicking on the origin).
   * @private
   */
  onOriginTap_: function(event) {
    this.selectedOrigin = event.model.item;
    var categorySelected =
        this.allSites ?
        'all-sites' :
        'site-settings-category-' + this.computeCategoryTextId(this.category);
    this.currentRoute = {
      page: this.currentRoute.page,
      section: 'privacy',
      subpage: ['site-settings', categorySelected, 'site-details'],
    };
  },

  /**
   * A handler for activating one of the menu action items.
   * @param {!{model: !{item: string},
   *           target: !{selectedItems: !{textContent: string}}}} event
   * @private
   */
  onActionMenuIronSelect_: function(event) {
    var origin = event.model.item;
    var action = event.target.selectedItems[0].textContent;
    if (action == this.i18n_.resetAction) {
      this.resetCategoryPermissionForOrigin(origin, this.category);
    } else {
      var value = (action == this.i18n_.allowAction) ?
          settings.PermissionValues.ALLOW :
          settings.PermissionValues.BLOCK;
      this.setCategoryPermissionForOrigin(origin, value, this.category);
    }
  },

  /**
   * Returns the appropriate header value for display.
   * @param {Array<string>} siteList The list of all sites to display for this
   *     category subtype.
   * @param {boolean} toggleState The state of the global toggle for this
   *     category.
   * @private
   */
  computeSiteListHeader_: function(siteList, toggleState) {
    if (this.categorySubtype == settings.PermissionValues.ALLOW) {
      return loadTimeData.getStringF(
          'titleAndCount',
          loadTimeData.getString(
              toggleState ? 'siteSettingsAllow' : 'siteSettingsExceptions'),
          siteList.length);
    } else {
      return loadTimeData.getStringF(
          'titleAndCount',
          loadTimeData.getString('siteSettingsBlock'),
          siteList.length);
    }
  },

  /**
   * Returns true if this widget is showing the allow list.
   * @private
   */
  isAllowList_: function() {
    return this.categorySubtype == settings.PermissionValues.ALLOW;
  },

  /**
   * Returns whether to show the site list.
   * @param {Array} siteList The list of all sites to display for this category
   *     subtype.
   * @param {boolean} toggleState The state of the global toggle for this
   *     category.
   * @private
   */
  showSiteList_: function(siteList, toggleState) {
    if (siteList.length == 0)
      return false;
    // The Block list is only shown when the category is set to Allow since it
    // is redundant to also list all the sites that are blocked.
    if (this.isAllowList_())
      return true;

    return toggleState;
  },

  /**
   * Returns the icon to use for a given site.
   * @param {string} url The url of the site to fetch the icon for.
   * @private
   */
  computeSiteIcon_: function(url) {
    // TODO(finnur): For now, we're returning a placeholder image for each site
    // but the actual favicon for each site will need to be returned.
    return 'communication:message';
  },
});
