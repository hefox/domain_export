<?php

/*
 * Functions specific to domain table
 */
 
function domain_export_flush_caches() {
  // hackish, check for exported domains where the fuck should this go?
  $exported_domains = module_invoke_all('default_domain');
  foreach($exported_domains as $machine_name => $domain) {
    if (!db_result(db_query("SELECT domain_id from {domain} where machine_name='%s'",$machine_name))) {
      domain_export_domain_save($domain) ;
    }
  }
}


/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 */
function domain_export_form_domain_form_alter(&$form, &$form_state) {
  $form['domain']['machine_name'] = array(
    '#type' => 'textfield',
    '#title' => t("Machine name."),
    '#description' => t('For use with domain export'),
    '#required' => TRUE,
    '#default_value' => db_result(db_query("SELECT machine_name from {domain} where domain_id = %d", $form['domain_id']['#value'])),
    '#weight' => -10,
  );
}

function domain_export_domainupdate($action, $domain, $form_state) {
  if ($form_state['values']['machine_name'] 
    && $form_state['values']['machine_name'] != $domain['machine_name']) {
    db_query("UPDATE {domain} set machine_name = '%s' where domain_id = %d", $form_state['values']['machine_name'], $domain['domain_id']);
  }
}
function domain_export_domainload(&$domain) {
  if ($domain['domain_id']) {
    $domain['machine_name'] = db_result(
      db_query("SELECT machine_name from {domain} where domain_id = %d", $domain['domain_id'])
    );
  }
}

/**
* Load a single domain.
*/
function domain_export_domain_load($name) {
  $id = db_query("SELECT domain_id from {domain} where machine_name = '%s'",$name);
  return (object) domain_lookup($id);
}

/**
* Load a single myobj.
*/
function domain_export_domain_save(&$myobj) {
  
  $values = (array) $myobj; 
  
  $myobj->domain_id = db_result(
    db_query("SELECT domain_id from {domain} where machine_name = %s",$values['machine_name'])
  );
  $update = (isset($myobj->domain_id) && is_numeric($myobj->domain_id)) ? array('domain_id') : array();
  if (!empty($update)) {
    $action = 'update' ;  
  } else {
    $action = 'create' ;
  }
  drupal_write_record('domain', $myobj, $update);

  // Let other modules act.
  $domain = domain_lookup(NULL, $values['subdomain']);
  $form_state['values'] = $values;
  module_invoke_all('domainupdate', $action, $domain, $form_state);
  $myobj = (object) $form_state['values']; 
}


/**
* Export a myobj and display it in a form.
*/
function domain_export_export_domain($domain) {
  $domain = (object)$domain;
  drupal_set_title(check_plain($domain->sitename));
  $code = domain_export_domain_export($domain);
  return drupal_get_form('ctools_export_form', $code, check_plain($domain->sitename));
}

/**
* Export a domain.
*
* By declaring this function in the schema, anyone can easily export a domain
* just by knowing that myobj exists.
*/
function domain_export_domain_export($domain, $indent = '') {
  ctools_include('export');
  $output = ctools_export_object('domain', $domain, $indent);
  return $output;
}
