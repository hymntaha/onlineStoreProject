<?php

/**
 * @file
 * On behalf implementation of Feeds mapping API for commerce_product_reference.module.
 */

/**
 * Implements hook_feeds_processor_targets_alter().
 *
 * @see FeedsNodeProcessor::getMappingTargets().
 */
function _commerce_product_reference_feeds_processor_targets_alter(&$targets, $entity_type, $bundle_name) {
  foreach (field_info_instances($entity_type, $bundle_name) as $name => $instance) {
    $info = field_info_field($name);
    if ($info['type'] == 'commerce_product_reference') {
      if (array_key_exists('product_id', $info['columns'])) {
        $targets[$name . ":product_id"] = array(
          'name' => $instance['label'] . ': Product ID',
          'callback' => 'commerce_product_reference_feeds_set_target',
          'description' => t('The product id for the @name field. NOTE: use this feature with care, products ids are usually assigned by Drupal.', array('@name' => $instance['label'])),
          'real_target' => $name,
        );
        $targets[$name . ":sku"] = array(
          'name' => $instance['label'] . ': SKU',
          'callback' => 'commerce_product_reference_feeds_set_target',
          'description' => t('The SKU reference for the @name field. NOTE: the product entity needs to exist.', array('@name' => $instance['label'])),
          'real_target' => $name,
        );
      }
    }
  }
}

/**
 * Callback for mapping. Here is where the actual mapping happens.
 *
 * When the callback is invoked, $target contains the name of the field the
 * user has decided to map to and $value contains the value of the feed item
 * element the user has picked as a source.
 */
function commerce_product_reference_feeds_set_target($source, $entity, $target, $value) {
  if (empty($value)) {
    return;
  }

  // Handle non-multiple value fields.
  if (!is_array($value)) {
    $value = array($value);
  }

  // Iterate over all values.
  $i = 0;
  $info = field_info_field($target);
  list($field_name, $sub_field) = explode(':', $target);
  foreach ($value as $v) {
    if (!is_array($v) && !is_object($v)) {
      // always put the default currency
      if (strstr($target, 'product_id')) {
        $field[LANGUAGE_NONE][$i]['product_id'] = $v;
      }
      elseif (strstr($target, 'sku')) {
        if ($product = commerce_product_load_by_sku($v)) {
          $field[LANGUAGE_NONE][$i]['product_id'] = $product->product_id;
        } else {
          drupal_set_message(t('A product with SKU %sku could not be found. Please check that the product exists or import it first.', array('%sku' => $v)), 'error');
        }
      }
    }
    if ($info['cardinality'] == 1) {
      break;
    }
    $i++;
  }
  $entity->{$field_name} = $field;
}
