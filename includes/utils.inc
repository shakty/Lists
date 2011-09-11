<?php

/**
 * 
 * @file
 * This file contains several helping functions that are used for the module 
 */

/**
 * 
 * creates an embedded menu with letters, roles, "all" and "newest" items
 * @param array $args	list of arguments to use for the creation of the menu items
 * @return 				themed menu
 */
function _user_list_embedded_menu($args = array()) {
  
  $items['all'] = array(
    'path' => 'userlist',
    'title' => t('all'),
  );
  
  if (variable_get('user_list_newest', TRUE)) {
    $items['newest'] = array(
      'path' => 'userlist/newest',
      'title' => t('newest'),
    );
  }
  
  if (variable_get('user_list_alpha_listing', TRUE)) {
    $letter = 'a';
    do {
      $title = (variable_get('user_list_alpha_listing_caps', FALSE) ? drupal_strtoupper($letter) : $letter);
      $items[$letter] = array(
        'path' => 'userlist/'. $letter,
        'title' => t('@letter', array('@letter' => $title)),
      );
      if ($args[1] == $letter) {
        $active = $letter;
        $items[$letter]['active'] = TRUE;
      }
    } while ($letter++ < 'z');
    $items['other'] = array(
      'path' => 'userlist/other',
      'title' => t('other'),
    );
    if ($args[1] == 'other') {
      $active = 'other';
      $items['other']['active'] = TRUE;
    }
  }
  
  foreach (variable_get('user_list_content_types', array()) as $type) {
    //if ($name = node_get_types('type', $type)) {
    if ($name = _node_types_build()->types) { //Vahan 11.02.2011
      $items[$type] = array(
          'path' => 'userlist/content/'. $type,
          //'title' => $name->name,
          'title' => isset($name[$type]) ? t($name[$type]->name) : t(""), //Vahan, 04.03.2011 //TODO: NOTICE: Undefined offset, property of non-object
        );
      if ($args[1] == 'content' && $args[2] == $type) {
        $active = $type;
        $items[$type]['active'] = TRUE;
      }
    }
  }
  
  foreach (variable_get('user_list_role_types', array()) as $role) {
    if (_user_list_get_roles('base', $role)) {
      $items[$role] = array(
        'path' => 'userlist/roles/'. $role,
        'title' => _user_list_get_roles('name', $role),
      );
      if ($args[1] == 'roles' && $args[2] == $role) {
        $active = $role;
        $items[$role]['active'] = TRUE;
      }
    }
  }
  
  if (!isset($active)) {
    $items['all']['active'] = TRUE;
    $active = 'all';
  }
  
  return theme('user_list_menu', $items);
}

/**
 * 
 * Helper function, returns which user data to search 
 * @return 			data names wich are used for the search
 */
function user_list_get_user_data() {
	return array('uid', 'name', 'mail', 'status', 'data', 'created');
}

/**
 *  
 * Returns an array|object|boolean of role table
 * @param string $op
 * 				'get' => return a $role array for a $role_type or an array of roles from role table
 * 				'base' => return whether a role exists in role table
 * 				'name' => return just the name of the role specified in $role_type
 * @param string $role_type:
 *    			if role_type is null gets all roles, if role_type is specified grabs information 
 *    			for only that role from role table
 * @return 		role table
 */
function _user_list_get_roles($op = 'get', $role_type = NULL) {
  
  // Gather Data from {role} table
  if (!is_null($role_type)) {
    // need to make sure we translate back to spaces
    $role_type = str_replace('_', ' ', $role_type);
    $result = db_query("SELECT r.rid, r.name FROM {role} r WHERE r.name = :role", array(':role' => $role_type))->fetchAssoc(); //Vahan 11.02.2011, 18.03.2011
  }
  else {
    // Role: 1 anonymous
    // Role: 2 authenticated
    // Are Skipped    
    $result = db_query("SELECT r.rid, r.name FROM {role} r WHERE r.rid > 2")->fetchAll();
  }
  
  if (!isset($result)) {
    return FALSE;
  }
  
  switch($op) {
    
    case 'base':
      return TRUE;
      
    case 'get':
      
      if (!is_null($role_type)){
        return $result;
      }
      
      $roles = array();
      foreach ($result as $record) {      
        // take care of spaces for url's
        $rname = str_replace(' ', '_', $record->name);
        $roles[$rname] = array('rid'=>$record->rid,'name'=>$rname);
      }
      return $roles;

    case 'name':
      
      if (is_null($role_type)) {
        return FALSE;
      } 
      return $result['name']; 
  }
}

/**
 * 
 * Checks if the user has the role
 * @param int $uid		user's id
 * @param string $role	the role name to check
 * @return 				true if the user has the role
 */
function user_list_has_role($uid, $role) {
	$roles = user_load($uid)->roles;
	foreach ($roles as $r) {
		if (stripos($r, $role) !== FALSE) return true;
	}
	return false;
}

/**
 * 
 * @return	array containing all user data
 */
function user_list_get_all_user_data() {
	return array('name', 'mail', 'role', 'created', 'status');
}

/**
 * 
 * @param int $uid	the user's id
 * @param int $fid	id of the field in wich to search
 * @return 			field_id => label array of fields assosiated with the user
 */
function user_list_user_fields($uid, $fid) {
	$fields = array();
	$all_fields = user_list_get_all_fields();
	$user = user_load($uid);
	foreach ($all_fields as $fname => $field) {
		if ($fid != $field['field_id']) continue;
		$user_field = $user->$fname;
		$tid = null;
		if (count($user_field) > 0) {
			$tid = isset($user_field['und'][0]['tid']) ? $user_field['und'][0]['tid'] : null; //TODO: Notice: Undefined index: tid
		}
		if ($tid == null) continue;
		$term = taxonomy_term_load($tid); //Loads the term, from where we can get the appropriate vid 
		$taxonomy_tree = taxonomy_get_tree($term->vid);
		foreach ($taxonomy_tree as $tax) {
			if ($tid == $tax->tid) {
				$fields[$fname] = $tax->name;
			}
		}
	}
	
	return $fields;
}

/**
 * 
 * @param array $users	list of the users among whom the role is searched
 * @param int	$uid	id of the user whose role id is returned
 * @return 				corresponding role rid of the given user, FALSE if not found 
 * @author Vahe 02.05.2011
 */
function user_list_get_user_role_id($users, $uid) {
  $roles = array_map('check_plain', user_roles(TRUE));
  foreach ($users as $user) {
    $users_roles = array();
      $roles_result = db_query('SELECT rid, uid FROM {users_roles} WHERE uid = :uid', array(':uid' => $user->uid));
      foreach ($roles_result as $user_role) {
        if ($uid == $user_role->uid) {
          return $user_role->rid;
        }
      }
  }
  return FALSE;
}

/**
 * 
 * Shows whatever is returned in "Test" tab.
 */
function user_list_test() {
	return "<div id='map-div'></div>";
	
	//return print_r(variable_get('user_list_role_types', array()),true);
}

/**
 * 
 * Returns the intersection of arrays of users comparing them by their ids
 * @param array $array1	first array to intersect
 * @param array $array2	second array to intersect
 * @return 				the intersection of arrays of users
 */
function user_list_user_array_intersect_by_uid($array1, $array2) {
	if (count($array1) == 0 || count($array2) == 0) {
		return array();
	}
	$res = array();
	foreach ($array1 as $u1) {
		foreach ($array2 as $u2) {
			if ($u1->uid == $u2->uid) {
				$res[] = $u1;
			}
		}
	}
	return $res;
}

/**
 * 
 * Returns the difference of arrays of users comparing them by their ids
 * @param array $array1	from wich elements the is subtracted
 * @param array $array2	which elements are subtracted
 * @return 				the difference of arrays of users
 */
function user_list_user_array_diff_by_uid($array1, $array2) {
	
	$res = $array1;
	foreach ($array1 as $k1=>$u1) {
		foreach ($array2 as $u2) {
			if ($u1->uid == $u2->uid) {
				unset($res[$k1]);
			}
		}
	}	
	return $res;
}

/**
 * 
 * Returns true if any of the elements of $array contains $string, and false otherwise
 * @param array	$array		array of string elements
 * @param string $string	string to be searched in the array
 * @return 					true if string is found, false - otherwise
 */
function user_list_contains_array($string, $array) {
	foreach ($array as $val) {
		if (stripos($val, $string) !== FALSE) return true;
	}
	return false;
}

/**
 * 
 * @param string $timestamp	timestamp for the date
 * @param string $interval	string in start,end format, where start and end are both in any PHP supported date format
 * @return 		 true if the given $date is in $interval, otherwise false is returned
 */
function user_list_date_is_in_interval($timestamp, $interval) {
	//TODO: php does not support dd/mm/yyyy format, but this replacement can break other formats
	//$interval = str_replace(array("/","."), "-", $interval); 
	if (stripos($interval, ",") === FALSE) {
		$interval .= ",";
	}
	$date = getdate($timestamp);
	$start_end = explode(',', $interval);
	
	if (strtotime($start_end[0])) {
		$start = getdate(strtotime($start_end[0]));
	} elseif (strtotime($start_end[1]) === FALSE) {
		return false;
	}
	$end = getdate(strlen(strtotime($start_end[1])) > 0 ? strtotime($start_end[1]) : time());
		
	return  (user_list_compare_dates($date, $start) >= 0 && user_list_compare_dates($date, $end) <= 0);
}

/**
 * 
 * tired of typing the long line everytime
 */
function p($obj) {
	drupal_set_message(print_r($obj, true));
}

/**
 * user_list_autocomplete
 * Function determining the autocompletion procedure and printing the result as a json list
 * @param string $term the term to search for
 * @param string $search_type type of the search, which determines the db tables to search for the results
 */
function user_list_autocomplete($term = '', $search_type = 'generic') {
    // The table and the column to search in (used not for all cases)
    $table="";
    $column="";

    // the array of returned data
    $result=array();
    // In some cases result is ready before the case switching is done
    $resultReady = FALSE;

    switch ($search_type) {
      case 'role':
        $table = "role";
        $column = "name";
        break;
      case 'mail':
        $table = "users";
        $column = "mail";
        break;
      case 'name':
        $table = "users";
        $column = "name";
        break;
      case 'date':
        // This should bring a date box with date selection...
        return;
        break;
      case 'generic':
        return;
        break;
      default:
        // $search_type should match with the name of the field
        $result = taxonomy_get_field_autocomplete("field_".$search_type, $term);
        $resultReady = TRUE;
        break;
    }
    if (!$resultReady) {
        drupal_json_output(user_list_get_autocomplete_data($table, $column, $term));
    } else {
        drupal_json_output($result);
    }
    return;
}

// @TODO allow for additional conditions to be passed, in addition to table name and column
/**
 * user_list_get_autocomplete_data
 * Function to get the autocomplete data
 * @param string $table the name of the db table to get the data from
 * @param string $column the name of the table column
 * @param string $term the term to search for
 * @return 		 the array containing result strings
 */
function user_list_get_autocomplete_data($table, $column, $term) {
    $query = db_select($table, 't')
                ->fields("t", array($column))
                ->where("LOWER(t.".$column.") LIKE '%".$term."%'")
                ->orderBy('t.'.$column, 'ASC');
    $result = $query
          ->execute();
          
    $results = array();
    foreach ($result as $r) {
      if ($r->$column != "")
        $results[] = $r->$column;
    }
    return $results; 
}

/**
 * taxonomy_get_field_autocomplete
 * Function to get the taxonomy field names as an array, given the field name
 * @param string $field_name
 * @param string $tag
 * @return
 */
function taxonomy_get_field_autocomplete($field_name, $tag = '') {
  try {
    $field = field_info_field($field_name);

    // Part of the criteria for the query come from the field's own settings.
    $vids = array();
    $vocabularies = taxonomy_vocabulary_get_names();
    foreach ($field['settings']['allowed_values'] as $tree) {
      $vids[] = $vocabularies[$tree['vocabulary']]->vid;
    }

    $query = db_select('taxonomy_term_data', 't');
    $query->addTag('translatable');
    $query->addTag('term_access');

    // Select rows that match by term name.
    $tags_return = $query
      ->fields('t', array('tid', 'name'))
      ->condition('t.vid', $vids)
      ->condition('t.name', '%' . db_like($tag) . '%', 'LIKE')
      ->range(0, 10)
      ->execute()
      ->fetchAllKeyed();

    $term_matches = array();
    foreach ($tags_return as $tid => $name) {
      $n = $name;
      // Term names containing commas or quotes must be wrapped in quotes.
      if (strpos($name, ',') !== FALSE || strpos($name, '"') !== FALSE) {
        $n = '"' . str_replace('"', '""', $name) . '"';
      }
      else {
        $term_matches[] = check_plain($name);
      }
    }
    return $term_matches;
  } catch (Exception $e) {
    // If there is no such field name, or in case of any other exception
    return array();
  }
}

/**
 * 
 * Compares the given date arrays
 * @param array $date1	first date to be compared
 * @param array $date2	second date to be compared
 * @return 				1 if $date1 > $date2, 0 if $date1 == $date2, -1 otherwise
 */
function user_list_compare_dates($date1, $date2) {
	if ($date1['year'] > $date2['year']) return 1;
	elseif ($date1['year'] == $date2['year'] && $date1['mon'] > $date2['mon']) return 1;
	elseif ($date1['year'] == $date2['year'] && $date1['mon'] == $date2['mon'] && $date1['mday'] > $date2['mday']) return 1;
	elseif ($date1['year'] == $date2['year'] && $date1['mon'] == $date2['mon'] && $date1['mday'] == $date2['mday']) return 0;
	else return -1;
}

/**
 * 
 * Returns the field_id of the field biven by its name
 * @param string $fname the field's name to check
 * @return 				the field's id
 */
function user_list_get_fid_by_fname($fname) {
	$all_fields = user_list_get_all_fields();
	foreach ($all_fields as $field) {
		if ($field['field_name'] == $fname) {
			return $field['field_id'];
		}
	}
}

/**
 * 
 * Returns the keyword for the given field
 * @param array $field	field for which a keyword is generated
 * @return 				keyword for the given field
 */
function user_list_get_field_keyword($field) {
	return strtoupper(substr($field['field_name'], strlen('field_')));
}

/** 
 * 
 * @return	true if Clean Url is enables, otherwise false
 */
function user_list_is_clean_url() {
	return strpos(request_uri(), "q=userlist") === FALSE;
}


/**
 * Ste: this is not working at all
 * Vahan: should we delete it? 
 * Ste: no we should fix it, later on...
 * Ste: this should be moved inside .module again, once fixed.
 * 
 */
function user_list_block($op = 'list', $delta = 'all', $edit = array()) {
  switch ($op) {
    
    case 'list':
      $blocks['all'] = array('info' => t('User List: All users'));
      $blocks['newest'] = array('info' => t('User List: Newest users'));
      foreach (variable_get('user_list_content_types', array()) as $type) {
        //if ($name = node_get_types('name', $type)) {
        if ($name = _node_types_build()->names) { //Vahan 11.02.2011
          $blocks[$type] = array('info' => t('User List: Users with @types', array('@type' => $name)));
        }
      }
      foreach (variable_get('user_list_role_types', array()) as $role) {
        if (_user_list_get_roles('base', $key)) {
          $blocks[$type] = array('info' => t('User List: Users with %role', array('%role' => _user_list_get_roles('name', $role))));
        }
      }
      return $blocks;
      
    case 'configure':
      if ($delta == 'all') {
        $default_header = t('All users');
      }
      else if ($delta == 'newest') {
        $default_header = t('Recent users');
      }
      else if (in_array($delta, variable_get('user_list_content_types', array()))) {
        //$default_header = t('Users who have contributed at least one @content', array('@content' => node_get_types('name', $delta)));
        $default_header = t('Users who have contributed at least one @content', array('@content' => _node_types_build()->names)); //Vahan 11.02.2011
      }
      else if (array_key_exist($delta, variable_get('user_list_role_types', array()))) {
        $default_header = t('Users of the %role role', array('%role' => _user_list_get_roles('name', $delta)));
      }
      $form['header'] = array(
        '#type' => 'textfield',
        '#title' => t('Header'),
        '#default_value' => variable_get('user_list_block_header_'. $delta, $default_header),
        '#description' => t('The block header as it will appear to the user.'),
      );
      $form['embedded_menu'] = array(
        '#type' => 'checkbox',
        '#title' => t('Embed Menu'),
        '#default_value' => variable_get('user_list_block_embed_'. $delta, FALSE),
        '#description' => t('If checked, an embedded menu of the various user listing pages will be included in the block.'),
      );
      $form['number'] = array(
        '#type' => 'select',
        '#title' => t('Number of users to list'),
        '#default_value' => variable_get('user_list_block_number_'. $delta, variable_get('user_list_default_number', 10)),
        '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 25, 30, 40, 50, 100, 125, 150, 200, 250, 500)),
        '#description' => t('Choose the number of users to list on this block.'),
      );
      $form['include_more'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include a more link'),
        '#default_value' => variable_get('user_list_block_more_'. $delta, TRUE),
        '#description' => t('If checked and the corresponding user list page is available, then this block will include a link to that page at the end, if there are more users than are listed in the block.'),
      );
      $form['include_more_msg'] = array(
        '#type' => 'textfield',
        '#title' => t('View more link text'),
        '#default_value' => variable_get('user_list_block_more_msg_'. $delta, t('View More')),
        '#description' => t('If there are more users than listed in the block, and we make a link available to view them on another page, then this is the text for that link.'),
      );
      return $form;
      
    case 'save':
      variable_set('user_list_block_header_'. $delta, $edit['header']);
      variable_set('user_list_block_embed_'. $delta, $edit['embedded_menu']);
      variable_set('user_list_block_number_'. $delta, $edit['number']);
      variable_set('user_list_block_more_'. $delta, $edit['include_more']);
      variable_set('user_list_block_more_msg_'. $delta, $edit['include_more_msg']);
      break;
      
    case 'view':
      if (user_access('access user lists')) {
        $number = variable_get('user_list_block_number_'. $delta, variable_get('user_list_default_number', 10));
        $include_more = variable_get('user_list_block_more_'. $delta, TRUE);
        $embed_menu = variable_get('user_list_block_embed_'. $delta, FALSE);
        if ($delta == 'all') {
          if ($include_more) {
            $include_more = l(variable_get('user_list_block_more_msg_'. $delta, t('View More')), 'userlist');
          }
          $block['subject'] = variable_get('user_list_block_header_'. $delta, t('All Users'));
          $block['content'] = _user_list('all', $number, FALSE, $include_more, '', NULL, NULL, $embed_menu, TRUE);
        }
        else if ($delta == 'newest') {
          if ($include_more) {
            $include_more = l(variable_get('user_list_block_more_msg_'. $delta, t('View More')), 'userlist/newest');
          }
          $block['subject'] = variable_get('user_list_block_header_'. $delta, t('Recent Users'));
          $block['content'] = _user_list('newest', $number, FALSE, $include_more, '', NULL, NULL, $embed_menu, TRUE);
        }
        else if (in_array($delta, variable_get('user_list_content_types', array()))) {
          if ($include_more) {
            $include_more = l(variable_get('user_list_block_more_msg_'. $delta, t('View More')), 'userlist/content/'. $delta);
          }
          //$block['subject'] = variable_get('user_list_block_header_'. $delta, t('Users who have contributed at least one @content', array('@content' => node_get_types('name', $delta))));
          $block['subject'] = variable_get('user_list_block_header_'. $delta, t('Users who have contributed at least one @content', array('@content' => _node_types_build()->names))); //Vahan 11.02.2011
          $block['content'] = _user_list('content', $number, FALSE, $include_more, '', NULL, $delta, $embed_menu, TRUE);
        }
        else if (array_key_exist($delta, variable_get('user_list_role_types', array()))) {
          if ($include_more) {
            $include_more = l(variable_get('user_list_block_more_msg_'. $delta, t('View More')), 'userlist/roles/'. $delta);
          }
          $block['subject'] = variable_get('user_list_block_header_'. $delta, t('Users of the %role role', array('%role' => _user_list_get_roles('name', $delta))));
          $block['content'] = _user_list('roles', $number, FALSE, $include_more, '', NULL, NULL, $embed_menu, TRUE, NULL, $delta);
        }
      }
      return $block;
  }
}
