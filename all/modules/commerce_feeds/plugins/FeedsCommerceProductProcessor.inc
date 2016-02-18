<?php

/**
 * @file
 * Class definition of FeedsCommerceProductProcessor.
 */

/**
 * Creates products from feed items.
 */
class FeedsCommerceProductProcessor extends FeedsProcessor {
  /**
   * Define entity type.
   */
  public function entityType() {
    return 'commerce_product';
  }

  /**
   * Implements parent::entityInfo().
   */
  protected function entityInfo() {
    $info = parent::entityInfo();
    $info['label plural'] = t('Commerce Products');
    return $info;
  }

  /**
   * Creates a new product in memory and returns it.
   */
  protected function newEntity(FeedsSource $source) {
    $type = $this->config['product_type'];
    $product = commerce_product_new($type);
    $product->uid = $this->config['author'];
    return $product;
  }

  /**
   * Loads an existing product.
   */
  protected function entityLoad(FeedsSource $source, $product_id) {
    if ($this->config['update_existing'] == FEEDS_UPDATE_EXISTING) {
      $product = commerce_product_load_by_sku($product_id);
    }
    return $product;
  }

  /**
   * Save a product.
   */
  public function entitySave($entity) {
    commerce_product_save($entity);
  }

  /**
   * Delete a series of products.
   */
  protected function entityDeleteMultiple($nids) {
    commerce_product_delete_multiple($nids);
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $types = commerce_product_type_get_name();
    $type = isset($types['product']) ? 'product' : key($types);
    return array(
      'product_type' => $type,
      'author' => 0,
    ) + parent::configDefaults();
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $types = commerce_product_type_get_name();
    array_walk($types, 'check_plain');
    $form = parent::configForm($form_state);
    unset($form['input_format']);
    $form['product_type'] = array(
      '#type' => 'select',
      '#title' => t('Product type'),
      '#description' => t('Select the product type for the products to be created. <strong>Note:</strong> Users with "import !feed_id feeds" permissions will be able to <strong>import</strong> products of the content type selected here regardless of the product level permissions. Further, users with "clear !feed_id permissions" will be able to <strong>delete</strong> imported products regardless of their product level permissions.', array('!feed_id' => $this->id)),
      '#options' => $types,
      '#default_value' => $this->config['product_type'],
    );
    $author = user_load($this->config['author']);
    $form['author'] = array(
      '#type' => 'textfield',
      '#title' => t('Author'),
      '#description' => t('Select the author of the products to be created - leave empty to assign "anonymous".'),
      '#autocomplete_path' => 'user/autocomplete',
      '#default_value' => empty($author->name) ?  'anonymous' : check_plain($author->name),
    );
    $form['update_existing']['#options'] = array(
      FEEDS_SKIP_EXISTING => 'Do not update existing products',
      FEEDS_UPDATE_EXISTING => 'Update existing products',
    );
    return $form;
  }

  /**
   * Override parent::configFormValidate().
   */
  public function configFormValidate(&$values) {
    if ($author = user_load_by_name($values['author'])) {
      $values['author'] = $author->uid;
    }
    else {
      $values['author'] = 0;
    }
  }

  /**
   * Override setTargetElement to operate on a target item that is a commerce product.
   */
  public function setTargetElement(FeedsSource $source, $target_node, $target_element, $value) {
    parent::setTargetElement($source, $target_node, $target_element, $value);
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {    
    $targets = array(
      'sku' => array(
        'name' => t('Product SKU'),
        'description' => t('The product identifier. Must be unique.'),
        'optional_unique' => TRUE,
      ),
      'title' => array(
        'name' => t('Product title'),
        'description' => t('The product title.'),
      ),
      'status' => array(
        'name' => t('Product status'),
        'description' => t('Status of the product.'),
      ),
      'uid' => array(
        'name' => t('User ID'),
        'description' => t('The Drupal user ID of the product owner.'),
      ),

    );

    // Let other modules expose mapping targets.
    self::loadMappers();
    feeds_alter('feeds_processor_targets', $targets, 'commerce_product', $this->config['product_type']);

    return $targets;
  }

  /**
   * Get product_id of an existing product if available.
   */
  protected function existingEntityId(FeedsSource $source, FeedsParserResult $result) {
    // Iterate through all unique targets and test whether they do already
    // exist in the database.
    foreach ($this->uniqueTargets($source, $result) as $target => $value) {
      switch ($target) {
        case 'sku':
          $sku = db_query("SELECT sku FROM {commerce_product} WHERE sku = :sku", array(':sku' => $value))->fetchField();
          break;
      }
      if ($sku) {
        // Return with the first sku found.
        return $sku;
      }
    }
    return 0;
  }
  
}
