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
  //todo add unique, machine name validaiton
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
  if (!$name) return;
  $id = db_result(db_query("SELECT domain_id from {domain} where machine_name = '%s' and domain_id <> 0",$name));
  if (empty($id)) return;
  return (object) domain_lookup($id);
}

/**
* Load a single myobj.
*/
function domain_export_domain_save(&$domain) {
  
  $values = (array) $domain; 
  
  $domain->domain_id = db_result(
    db_query("SELECT domain_id from {domain} where machine_name = '%s' and domain_id <> 0 ",$domain->machine_name)
  );
  $update = (isset($domain->domain_id) && is_numeric($domain->domain_id)) ? array('domain_id') : array();
  if (!empty($update)) {
    $action = 'update' ;  
  } else {
    $action = 'create' ;
  }

  drupal_write_record('domain', $domain, $update);
  module_invoke_all('domain_export_save', $domain);

  // Let other modules act.
  $domain = domain_lookup(NULL, $values['subdomain']);
  $form_state['values'] = $values;
  module_invoke_all('domainupdate', $action, $domain, $form_state);
  $myobj = (object) $form_state['values']; 
}


/*
 * Revert a domain to default verison
 */
function domain_export_revert_domain(&$form_state, $domain) {
  $form['machine_name'] = array('#type' => 'hidden', '#value' => $domain->machine_name);
  return confirm_form($form, 
    t('Are you sure you want to revert the domain @name?', array('@name' => $domain->machine_name)),
     'admin/build/domain', 
     '', 
     t('Revert'), 
     t('Cancel'));
}
/**
  * Revert a domain 
  **/
function domain_export_revert_domain_submit($form, &$form_state) {
  $exported_domains = module_invoke_all('default_domain');
  domain_export_domain_save($exported_domains[$form_state['values']['machine_name']]) ;
  $form_state['redirect'] = 'admin/build/domain';
  return;
}


function domain_export_domain_list() {
  $domains = array();
  $result = db_query("SELECT domain_id from {domain} where machine_name IS NOT NULL and domain_id <> 0"); 
  while ($row = db_fetch_object($result)) {
    $domain = (object) domain_lookup($row->domain_id);
    if ($domain->machine_name) $domains[$domain->machine_name] = $domain ;
  }
  return $domains;
}
/**
* Export a myobj and display it in a form.
*/
function domain_export_export_domain($domain) {
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
  $additions2 = module_invoke_all('domain_export_additions', $domain);
  $output = ctools_export_object('domain', $domain, $indent,  NULL, array(), $additions2) ;
  return $output;
}

