<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the Shipping UI module.
 */

/**
 * Builds the shipping settings page using the Rules UI overview table filtered
 *   to display shipping method rules.
 */
function commerce_shipping_ui_admin_page() {
  RulesPluginUI::$basePath = 'admin/commerce/config/shipping-methods';
  $content['enabled']['title']['#markup'] = '<h3>' . t('Enabled Shipping rules') . '</h3>';

  $conditions = array('event' => 'commerce_shipping_methods', 'plugin' => 'reaction rule', 'active' => TRUE);
  $content['enabled']['rules'] = RulesPluginUI::overviewTable($conditions);
  $content['enabled']['rules']['#empty'] = t('There are no active shipping methods.');

  $content['disabled']['title']['#markup'] = '<h3>' . t('Disabled shipping rules') . '</h3>';

  $conditions['active'] = FALSE;
  $content['disabled']['rules'] = RulesPluginUI::overviewTable($conditions);
  $content['disabled']['rules']['#empty'] = t('There are no disabled shipping methods.');

  return $content;
}
