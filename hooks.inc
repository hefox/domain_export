<?php


/**
 * Implementation of hook_domain_export_save().
 */
function domain_alias_domain_export_save($domain) {
  if (!empty($domain->aliases)) {
    foreach($domain->aliases as $alias) {
      $alias['domain_id'] = $domain->domain_id;
      $alias['alias_id'] = db_result(db_query("SELECT alias_id from {domain_alias} where pattern = '%s'", $alias['pattern']));
      $update = (isset($alias['alias_id']) && is_numeric($alias['alias_id'])) ? array('alias_id') : array();
      drupal_write_record('domain_alias', $alias, $update);
    }  
  }
}

/**
 * Implementation of hook_domain_export_save().
 */
function domain_conf_domain_export_save($domain) {
  if (!empty($domain->settings)) {
    $settings['domain_id'] = $domain->domain_id;
    $settings['settings'] = serialize($domain->settings);
    if (db_result(db_query("SELECT domain_id from {domain_conf} where domain_id = %d",$domain->domain_id))) {
      $update = array('domain_id');
    } else {
      $update = array();
    }
    drupal_write_record('domain_conf', $settings, $update);    
  }
}


/**
 * Implementation of hook_domain_export_additions().
 */
function domain_alias_domain_export_additions($domain) {
  if (empty($domain->aliases)) {
    $domain->aliases = domain_alias_list($domain->domain_id);
  }
  
  //cleanup aliases
  if (!empty($domain->aliases)) {
    foreach($domain->aliases as $alias) {
      $aliases[$alias['pattern']] = array(
        'pattern'=> $alias['pattern'], 
        'redirect' => $alias['redirect']
      );
    }
  }
  $additions2['aliases'] = $aliases;  
  return $additions2;
}

/**
 * Implementation of hook_domain_export_additions().
 */
function domain_conf_domain_export_additions($domain) {
  $settings = domain_unserialize(db_result(db_query("SELECT settings FROM {domain_conf} WHERE domain_id = %d", $domain->domain_id)));
  if ($settings) $additions2['settings'] = $settings; 
  return $additions2; 
}