<?php
/**
 * Holds the commerce_sendcloud admin form.
 *
 * @package commerce_sendcloud
 *
 * @version 1
 */

/**
 * Implements hook_form().
 */
function commerce_sendcloud_admin_form($form, &$form_state) {

  $form = array();

  // Create tabs and form elements.
  $form['vertical_tabs'] = array(
    '#type' => 'vertical_tabs',
    '#default_tab' => 'edit-tab1',
  );

  $form['tab1'] = array(
    '#type' => 'fieldset',
    '#title' => t('User Credentials'),
    '#collapsible' => TRUE,
    '#group' => 'vertical_tabs',
  );

  $form['tab2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced'),
    '#collapsible' => TRUE,
    '#group' => 'vertical_tabs',
  );

  $form['tab3'] = array(
    '#type' => 'fieldset',
    '#title' => t('Labels'),
    '#collapsible' => TRUE,
    '#group' => 'vertical_tabs',
  );

  $form['tab1']['commerce_sendcloud_key'] = array(
    '#type' => 'textfield',
    '#title' => t('SendCloud Key'),
    '#default_value' => variable_get('commerce_sendcloud_key'),
    '#required' => TRUE,
    '#size' => 32,
    '#maxlength' => 255,
  );

  $form['tab1']['commerce_sendcloud_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('SendCloud Secret'),
    '#default_value' => variable_get('commerce_sendcloud_secret'),
    '#required' => TRUE,
    '#size' => 32,
    '#maxlength' => 255,
  );

  $form['tab2']['commerce_sendcloud_testmode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable testmode'),
    '#description' => t('Enables test-mode. Because of API limitations, this means setting the shipping method to 
        <strong>Unstamped Letter</strong>, the only shipping method that does not require payment information. After
        sending a test, you should delete the parcel from the SendCloud control panel manually!'),
    '#default_value' => variable_get('commerce_sendcloud_testmode', '1'),
  );

  $form['tab2']['commerce_sendcloud_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Debugging'),
    '#default_value' => variable_get('commerce_sendcloud_debug', '1'),
  );

  $form['tab3']['commerce_sendcloud_download_labels'] = array(
    '#type' => 'checkbox',
    '#title' => t('Download labels'),
    '#default_value' => variable_get('commerce_sendcloud_download_labels', '1'),
  );

  /*$form['tab2']['commerce_sendcloud_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Debugging'),
    '#default_value' => variable_get('commerce_sendcloud_debug', '1'),
  );*/

  // Render the form.
  return system_settings_form($form);
}


/**
 * Builds the form for adding and editing flat rate services.
 */
function commerce_sendcloud_service_form($form, &$form_state, $shipping_service) {
  // Store the initial shipping service in the form state.
  $form_state['shipping_service'] = $shipping_service;

  $form['commerce_sendcloud_rate'] = array(
    '#tree' => TRUE,
  );

  $form['commerce_sendcloud_rate']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $shipping_service['title'],
    '#description' => t('The administrative title of this rate. It is recommended that this title begin with a capital letter and contain only letters, numbers, and spaces.'),
    '#required' => TRUE,
    '#size' => 32,
    '#maxlength' => 255,
    '#field_suffix' => ' <small id="edit-flat-rate-title-suffix">' . t('Machine name: @name', array('@name' => $shipping_service['name'])) . '</small>',
  );

  if (empty($shipping_service['name'])) {
    $form['commerce_sendcloud_rate']['name'] = array(
      '#type' => 'machine_name',
      '#title' => t('Machine name'),
      '#default_value' => $shipping_service['name'],
      '#maxlength' => 32,
      '#required' => TRUE,
      '#machine_name' => array(
        'exists' => 'commerce_shipping_service_load',
        'source' => array('commerce_sendcloud_rate', 'title'),
      ),
      '#description' => t('The machine-name of this rate. This name must contain only lowercase letters, numbers, and underscores. It must be unique.'),
    );
  }
  else {
    $form['commerce_sendcloud_rate']['name'] = array(
      '#type' => 'value',
      '#value' => $shipping_service['name'],
    );
  }

  $form['commerce_sendcloud_rate']['display_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Display title'),
    '#default_value' => $shipping_service['display_title'],
    '#description' => t('The front end display title of this rate shown to customers. Leave blank to default to the <em>Title</em> from above.'),
    '#size' => 32,
  );

  $form['commerce_sendcloud_rate']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Describe this flat rate if necessary. The text will be displayed in the flat rate services overview table.'),
    '#default_value' => $shipping_service['description'],
    '#rows' => 3,
  );

  if ($availablemethods = commerce_sendcloud_shipping_methods()) {
    foreach ($availablemethods as $sm) {
      $methods[$sm['id']] = $sm['name'];
    }

    $form['commerce_sendcloud_rate']['sendcloud_type'] = array(
      '#type' => 'select',
      '#title' => t('Available Sendcloud methods'),
      '#description' => t('Select what SendCloud service to use'),
      '#default_value' => !empty($shipping_service['data']['sendcloud_type']) ? $shipping_service['data']['sendcloud_type'] : 0,
      '#options' => $methods,
    );
    $disablebutton = FALSE;
  }
  else {
    $form['notification'] = array(
      '#markup' => t('Invalid configuration. Please check the options of sendcloud'),
    );
    $disablebutton = TRUE;
  }

  $form['commerce_sendcloud_rate']['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Price'),
    '#default_value' => commerce_currency_amount_to_decimal($shipping_service['base_rate']['amount'], $shipping_service['base_rate']['currency_code']),
    '#required' => TRUE,
    '#size' => 10,
    '#prefix' => '<div class="commerce-flat-rate-base-rate">',
  );

  // Build a currency options list from all enabled currencies.
  $options = commerce_currency_get_code(TRUE);

  // If the current currency value is not available, add it now with a
  // message in the help text explaining it.
  if (empty($options[$shipping_service['base_rate']['currency_code']])) {
    $options[$shipping_service['base_rate']['currency_code']] = check_plain($shipping_service['base_rate']['currency_code']);

    $description = t('The currency set for this rate is not currently enabled. If you change it now, you will not be able to set it back.');
  }
  else {
    $description = '';
  }

  // If only one currency option is available, don't use a select list.
  if (count($options) == 1) {
    $currency_code = key($options);

    $form['commerce_sendcloud_rate']['amount']['#field_suffix'] = check_plain($currency_code);
    $form['commerce_sendcloud_rate']['amount']['#suffix'] = '</div>';

    $form['commerce_sendcloud_rate']['currency_code'] = array(
      '#type' => 'value',
      '#default_value' => $currency_code,
    );
  }
  else {
    $form['commerce_sendcloud_rate']['currency_code'] = array(
      '#type' => 'select',
      '#description' => $description,
      '#options' => $options,
      '#default_value' => $shipping_service['base_rate']['currency_code'],
      '#suffix' => '</div>',
    );
  }

  // Add support for base rates including tax.
  if (module_exists('commerce_tax')) {
    // Build an array of tax types that are display inclusive.
    $inclusive_types = array();

    foreach (commerce_tax_types() as $name => $tax_type) {
      if ($tax_type['display_inclusive']) {
        $inclusive_types[$name] = $tax_type['title'];
      }
    }

    // Build an options array of tax rates of these types.
    $options = array();

    foreach (commerce_tax_rates() as $name => $tax_rate) {
      if (in_array($tax_rate['type'], array_keys($inclusive_types))) {
        $options[$inclusive_types[$tax_rate['type']]][$name] = t('Including @title', array('@title' => $tax_rate['title']));
      }
    }

    if (!empty($options)) {
      // Find the default value for the tax included element.
      $default = '';

      if (!empty($shipping_service['data']['include_tax'])) {
        $default = $shipping_service['data']['include_tax'];
      }

      $form['commerce_sendcloud_rate']['currency_code']['#title'] = '&nbsp;';

      $form['commerce_sendcloud_rate']['include_tax'] = array(
        '#type' => 'select',
        '#title' => t('Include tax in this rate'),
        '#description' => t('Saving a rate tax inclusive will bypass later calculations for the specified tax.'),
        '#options' => count($options) == 1 ? reset($options) : $options,
        '#default_value' => $default,
        '#required' => FALSE,
        '#empty_value' => '',
      );
    }
  }

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 40,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save sendcloud rate'),
    '#disabled' => $disablebutton,
  );

  if (!empty($form_state['shipping_service']['name'])) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete SendCloud rate'),
      '#suffix' => l(t('Cancel'), 'admin/commerce/config/shipping/services/sendcloud-api-shipping-method'),
      '#submit' => array('commerce_sendcloud_service_delete_form_submit'),
      '#weight' => 45,
    );
  }
  else {
    $form['actions']['submit']['#suffix'] = l(t('Cancel'), 'admin/commerce/config/shipping/services/sendcloud-api-shipping-method');
  }

  return $form;
}

/**
 * Validate handler: ensures a valid base rate was entered for the flat rate.
 */
function commerce_sendcloud_service_form_validate($form, &$form_state) {
  // Ensure the rate amount is numeric.
  if (!is_numeric($form_state['values']['commerce_sendcloud_rate']['amount'])) {
    form_set_error(
      'commerce_sendcloud',
      t('You must enter a numeric value for the base rate amount.')
    );
  }
  else {
    // Convert the decimal amount value entered to an
    // integer based amount value.
    form_set_value(
      $form['commerce_sendcloud_rate']['amount'],
      commerce_currency_decimal_to_amount($form_state['values']['commerce_sendcloud_rate']['amount'],
      $form_state['values']['commerce_sendcloud_rate']['currency_code']),
      $form_state
    );
  }
}

/**
 * Submit handler: saves the new or updated flat rate service.
 */
function commerce_sendcloud_service_form_submit($form, &$form_state) {

  $shipping_service = $form_state['shipping_service'];

  $looparray = array(
    'name',
    'title',
    'display_title',
    'description',
    'amount',
    'currency_code',
  );

  // Update the shipping service array with values from the form.
  foreach ($looparray as $key) {
    $shipping_service[$key] = $form_state['values']['commerce_sendcloud_rate'][$key];
  }

  // If a tax was specified for inclusion, add it to the data array.
  if (!empty($form_state['values']['commerce_sendcloud_rate']['include_tax'])) {
    $shipping_service['data']['include_tax'] = $form_state['values']['commerce_sendcloud_rate']['include_tax'];
  }
  elseif (!empty($shipping_service['data']['include_tax'])) {
    unset($shipping_service['data']['include_tax']);
  }

  // Set the rate.
  if (!empty($form_state['values']['commerce_sendcloud_rate']['sendcloud_type'])) {
    $shipping_service['data']['sendcloud_type'] = $form_state['values']['commerce_sendcloud_rate']['sendcloud_type'];
  }
  elseif (!empty($shipping_service['data']['sendcloud_type'])) {
    unset($shipping_service['data']['sendcloud_type']);
  }

  // Save the shipping service.
  unset($shipping_service['base_rate']);
  $op = commerce_sendcloud_service_save($shipping_service);

  // Check if we made it.
  if (!$op) {
    drupal_set_message(t('The SendCloud rate service failed to save properly. Please review the form and try again.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
  else {
    drupal_set_message(t('Sendcloud service saved.'));
    $form_state['redirect'] = 'admin/commerce/config/shipping';
  }

}

/**
 * Submit handler: redirects to the flat rate service delete confirmation form.
 */
function commerce_sendcloud_service_form_delete_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/commerce/config/shipping/services/sendcloud-rate-' . strtr($form_state['shipping_service']['name'], '_', '-') . '/delete';
}

/**
 * Displays the edit form for an existing flat rate service.
 */
function commerce_sendcloud_service_edit_page($name) {
  return drupal_get_form('commerce_sendcloud_service_form', commerce_shipping_service_load($name));
}

/**
 * Builds the form for deleting flat rate services.
 */
function commerce_sendcloud_service_delete_form($form, &$form_state, $shipping_service) {
  $form_state['shipping_service'] = $shipping_service;

  $form = confirm_form($form,
    t('Are you sure you want to delete the <em>%title</em> sendcloud service?', array('%title' => $shipping_service['title'])),
    'admin/commerce/config/shipping/services/sendcloud-api-shipping-method',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit callback for commerce_sendcloud_rate_service_delete_form().
 */
function commerce_sendcloud_service_delete_form_submit($form, &$form_state) {
  $shipping_service = $form_state['shipping_service'];

  commerce_sendcloud_service_delete($shipping_service['name']);

  drupal_set_message(t('The sendcloud service <em>%title</em> has been deleted.', array('%title' => $shipping_service['title'])));
  watchdog('commerce_sendcloud_rate', 'Deleted sendcloud service <em>%title</em>.', array('%title' => $shipping_service['title']), WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/commerce/config/shipping/services/sendcloud-api-shipping-method';
}

/**
 * Displays the delete confirmation form for an existing flat rate service.
 */
function commerce_sendcloud_service_delete_page($name) {
  return drupal_get_form('commerce_sendcloud_service_delete_form', commerce_shipping_service_load($name));
}
