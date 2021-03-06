<?php

/**
 * 
 * @file
 * An include file for user_list module
 * Containing functions for creating, validating and submitting forms of users lists.
 */

/**
 * 
 * Creates the form including users list, a checkbox for each user 
 * and a button to pass to the second page for entering the message.
 * After submition, a new page for entering the message opens.
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 * @param array $users    list of users to be shown in the form
 * @return           the created form
 * @author Vahan, 25.03.2011
 */
function user_list_form($form, &$form_state, $users) {
  global $user;

  if (isset($form_state['page_num']) && $form_state['page_num'] == 2) {
      return user_list_form_message($form, $form_state, $users);
  }
  // Otherwise we build page 1.
  
  $def_value = "";
  //search load
  if (isset($_GET['s'])) {
    $search = user_list_get_search($_GET['s']);
    if ($search->uid != $user->uid) exit(0); //Security check =)
    $url_combine = user_list_is_clean_url() ? "?" : "&";
    $url = "http://".$_SERVER['HTTP_HOST'].substr(request_uri(), 0, strrpos(request_uri(), "userlist")+strlen("userlist"));
    drupal_goto($url.(!empty($search->url) ? "/" : "").$search->url.$url_combine."search=".urlencode($search->search));
  }
  
  if (isset($_GET['search'])) {
    $def_value = urldecode($_GET['search']);
    $form_state['values']['text'] = $def_value; //make the search be refined after loading
  }
  //end of search load
  
  $form_state['page_num'] = 1;
  
  $form['search'] = array(
    '#title' => t('Refine Search'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  
  
  $form['search']['text'] = array(
    '#title' => t(''),
    '#type' => 'textfield',
    '#ajax' => array(
      // #ajax has two required keys: callback and wrapper.
      // 'callback' is a function that will be called when this element changes.
      'callback' => 'user_list_search_callback',
      // 'wrapper' is the HTML id of the page element that will be replaced.
      'wrapper' => 'users_div',
      // There are also several optional keys - see ajax_example_autocheckboxes
      // below for details on 'method', 'effect' and 'speed' and
      // ajax_example_dependent_dropdown for 'event'.
      'event' => 'die', //bug 1, see user_list.module. Kill all events to add only ENTER button
      'keypress' => TRUE, //Search when ENTER is pressed
       ),
       '#default_value' => $def_value,
       '#maxlength' => 1024,
       '#prefix' => "<table id='search-box-table'><tr><td style='padding-right: 3px;'>",
       '#suffix' => "<div id='datepicker'></div></td>"
  );
  
  $form['search']['search_button'] = array(
    '#value' => t('Go'),
    '#type' => 'button',
    '#ajax' => array(
      'callback' => 'user_list_search_callback',
      'wrapper' => 'users_div',
       ),
       '#submit' => array(),
    '#validate' => array(),
       '#prefix' => '<td style="padding-left: 0; width: 100%">',
       '#suffix' => '</td>',
  );

  $form['search']['help'] = array(
    '#type' => 'item',
    '#title' => l(t("Help"), "admin/help/user_list"),
       '#prefix' => '<td>',
       '#suffix' => '</td></tr></table>',
  );
  
  
  $form['search']['save'] = array(
    '#title' => t('Save or Load'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#prefix' => '<div id="save">',
    '#suffix' => '</div>',
  );
  
  $form['search']['save']['search_name'] = array(
    '#type' => 'textfield',
    '#title' => t(''),
    '#size' => '20',
       '#maxlength' => 64,
    '#prefix' => '<table id="save-load-table"><tr><td>',
    '#suffix' => '</td>',
  );
  
  $form['search']['save']['button_save'] = array(
    '#value' => t('Save Search'),
    '#type' => 'button',
    '#ajax' => array(
      'callback' => 'user_list_search_save_callback',
      'wrapper' => 'save',
       ),
       '#submit' => array(),
    '#validate' => array(),
       '#prefix' => '<td style="width: 100%">',
    '#suffix' => '</td><td><nobr>or load:</nobr></td>',
  );
  
  $options = array(0 => 'select');
  $searches = user_list_get_all_searches();
  foreach ($searches as $search) {
    $options[$search->name] = t($search->name);
  }
  
  $is_clean_url = user_list_is_clean_url() ? 1 : 0;
  
  $form['search']['save']['load'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#attributes' => array ('onchange' => '{
      if (this.selectedIndex == 0) return;
      if ('.$is_clean_url.' > 0) {
        window.location = window.location.href.substring(0, window.location.href.indexOf("?",0))+"?s="+this.options[this.selectedIndex].value;
      } else {
        window.location = window.location.href+"&s="+this.options[this.selectedIndex].value;
      }
    }'),
    '#prefix' => '<td>',
    '#suffix' => '</td></tr></table>',
  );
  
  
/*  
 * VAHE: WROTE the below without knowing of existance of _user_list_get_roles, 
 * but this way it might be faster, so need to check before deleting
 */
/*
  $user_role_list = db_select('users_roles', 'u_r')
            ->fields('u_r')
            ->extend('PagerDefault')
            ->execute();
    $rid_list = array();

    foreach ($user_role_list as $user_role) {
      $rid_list[] = $user_role->rid;
    }
  
  $role_list = db_select('role', 'r')
            ->fields('r')
            ->condition('rid', $rid_list, 'IN')
            ->execute();
  
  $roles = array();
  foreach ($role_list as $role) {
    $roles[$role->rid] = $role->name;
  }
*/  

// Replacing Everything with one single field    
  
//  $roles_extended = _user_list_get_roles('get');
//    foreach($roles_extended as $role) {
//      $roles[$role['rid']] = $role['name'];
//    }
    

//  $form['search']['role'] = array(
//      '#title' => t('Role'),
//    '#type' => 'select',
//       '#multiple' => TRUE,
//       '#options' => $roles,
//       '#size' => min(5, count($roles)),
//      '#prefix' => "<div id='search-more-options' />",
//      '#suffix' => "</div>",
//  );  
//  
//  $form['search']['more-options-link'] = array(
//      '#markup' => '<a id="more-options-link">more options</a>',
//  );
  
  // Commented by Vahe 02.05.2011 (creating the mutifield search)  
/*  
  $form['search'] = array(
    '#title' => 'Refine Search',
    '#type' => 'textfield',
    '#ajax' => array(
      // #ajax has two required keys: callback and wrapper.
      // 'callback' is a function that will be called when this element changes.
      'callback' => 'user_list_search_callback',
      // 'wrapper' is the HTML id of the page element that will be replaced.
      'wrapper' => 'users_div',
      // There are also several optional keys - see ajax_example_autocheckboxes
      // below for details on 'method', 'effect' and 'speed' and
      // ajax_example_dependent_dropdown for 'event'.
      'event' => 'die', //bug 1, see user_list.module
      'keypress' => TRUE,
      //perform search after each time ENTER is pressed
       ),
  );
*/
  
  $form['users'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Found Users'),
  );
  $form['users']['list'] = user_list_get_user_list($users);
  
  $all_fields = user_list_get_all_fields();
  $options_fields = array();
  foreach ($all_fields as $field) {
    $options_fields[$field['field_id']] = $field['label'];
  }  
  $all_user_data = user_list_get_all_user_data();
  $options_user_data = array();
  foreach ($all_user_data as $user_data) {
    $options_user_data[$user_data] = $user_data;
  }
  
  $form['users']['fields'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Choose fields to show'),
  );
  
  $form['users']['fields']['user_data'] = array(
    '#type' => 'checkboxes',
    '#title' => t(''),
    '#options' => $options_user_data,
      '#default_value' => array_keys(user_list_get_shown_user_data()),
    '#ajax' => array(
      'callback' => 'user_list_search_callback',
      'wrapper' => 'users_div',
      'progress' => array(),
       ),
       '#prefix' => '<div style="max-height: 300px; overflow: auto"><table style="border: 0;"><tr><td valign="top" style="border: 0;">',
       '#suffix' => '</td>',
  );
  
  $form['users']['fields']['field_list'] = array(
    '#type' => 'checkboxes',
    '#title' => t(''),
    '#options' => $options_fields,
      '#default_value' => array_keys(user_list_get_shown_fields()),
    '#ajax' => array(
      'callback' => 'user_list_search_callback',
      'wrapper' => 'users_div',
      'progress' => array(),
       ),
       '#prefix' => '<td valign="top" style="border: 0;">',
       '#suffix' => '</td></tr></table></div>',
  );
  
//  $users_selected = array();
//  $form['users_selected'] = array(
//    '#type' => 'fieldset',
//    '#collapsible' => TRUE,
//    '#collapsed' => FALSE,
//    '#title' => t('Selected Users'),
//    '#prefix' => '<div id="users_selected_div">',
//    '#suffix' => '</div>',
//  );
//  
//  $form['users_selected']['list_selected'] = user_list_get_user_list($users_selected);
  
  //$form['pager'] = array('#markup' => theme('pager')); //TODO: maybe needed
  
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Enter the message >>'),
      '#submit' => array('user_list_form_submit'),
      '#validate' => array('user_list_form_validate'),
    '#prefix' => '<table id="actions-table"><tr><td>',
  );

  /*$form['export'] = array(
    '#type' => 'submit',
    '#value' => t('Export to CSV'),
    '#submit' => array('user_list_export_submit'),
    '#validate' => array('user_list_form_validate'),
  );*/
  
  $form['export'] = array(
      '#type' => 'submit',
      '#value' => t('Export to CSV'),
      '#submit' => array('user_list_export_ste'),
      '#validate' => array('user_list_form_validate'),
    '#suffix' => '</td>',
  );
  
  $form['settings'] = array(
    '#type' => 'item',
    '#title' => l(t("Settings"),"admin/settings/userlist"),
    '#prefix' => '<td>',
    '#suffix' => '</td></tr></table>',
  );
    
  $found_users = $users;
  if (isset($form_state['values']['user_data'])) {
    user_list_save_hidden_user_data($form_state['values']['user_data']);
    $form['users']['list'] = user_list_get_user_list($found_users);
  }
  if (isset($form_state['values']['field_list'])) {
    user_list_save_hidden_fields($form_state['values']['field_list']);
    $form['users']['list'] = user_list_get_user_list($found_users);
  }
  
    // An AJAX request calls the form builder function for every change.
  // We can change how we build the form based on $form_state.
  if (!empty($form_state['values']['text'])) {
    $text = trim($form_state['values']['text']);
    $found_users = user_list_search($text); //search results
    //$found_users =  user_list_filter_users($users, $text); //search results from command search
// Only single-field search
//    $found_users_2 = user_list_filter_users_by_keywords($users, array('username'=>user_list_search_remove_key($text),'role'=>$form_state['values']['role'])); //search results from gui search
//    $found_users = user_list_get_user_list(user_list_user_array_intersect_by_uid($found_users_1, $found_users_2));
    //drupal_set_message(print_r($found_users, true), "warning");
    $form['users']['list'] = user_list_get_user_list($found_users);
  }

  //redraw selected users' list according to selected users -----------------------------------------------------------------
//  if(!empty($form_state['values']['list'])) {
//    $users = $form_state['values']['list'];
//    foreach($users as $uid->$checked) {
//        if ($checked !== 0) {
//          $selected_users[] = $u;
//      }
//    }
//    $form['users_selected']['list_selected'] = user_list_get_user_list($selected_users);
//  }

  return $form; 
}

/**
 * 
 * Ajax callback for search saving 
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 * @return           modified form 
 */
function user_list_search_save_callback($form, $form_state) {
  global $user;
  $search_name = $form_state['values']['search_name'] != '' ? $form_state['values']['search_name'] : 'default';
  $search_text = $form_state['values']['text'];
  
  $search_url = isset($_SESSION['userlist']['op']) ? $_SESSION['userlist']['op'] : "";
  
  $save_schema = drupal_get_schema(user_list_get_save_table_name());
  $result = user_list_get_search($search_name);

  if (count($result) == 0) {  
    db_query("INSERT INTO {".$save_schema['name']."} (uid, name, search, url) VALUES ($user->uid, '$search_name', '$search_text', '$search_url')");
  } else {
    db_query("UPDATE ".$save_schema['name']." SET search='$search_text', url='$search_url' WHERE name='$search_name'");
  }
  
  return $form['users']['save'];
}

/**
 * 
 * Ajax callback for the search.
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 * @return          renderable array
 */
function user_list_search_callback($form, $form_state) {
  $_GET["q"] = "userlist";
  return $form['users']['list'];
}

/**
 * user_list_filter_users
 * Returns the filtered users array according to the given keyword
 * @param $users the list of all users
 * @param keywords an associate array of keywords
 * @author Vahe
 */
// Doing Everything with 1 single field
/*
function user_list_filter_users_by_keywords($users, $keywords) {
  $users_filtered = array();
  
  $conditions = array(); // The filter conditions
  $criteria = array_keys($keywords); // username, roles, etc...
  
  // Checks to see if the criteria were specified
  foreach ($criteria as $criterion) {
    $conditions[$criterion] = TRUE;
    if ($keywords[$criterion] == "" || (is_array($keywords[$criterion]) && count($keywords[$criterion]) == 0)) {
      $conditions[$criterion] = FALSE;
    }
  }
  
  // $keywords['roles'] comes as an array
  foreach ($users as $u) {
      // get_user_role($users, $u->uid) returns the name instead of the uid!!
      // If the criteria were not specified, or were specified and match, keep the user in the filtered list
    if ((!$conditions['username'] || stripos($u->name, $keywords['username']) === 0) && 
        (!$conditions['role'] || array_key_exists(user_list_get_user_role_id($users, $u->uid), $keywords['role']))) {
      array_push($users_filtered, $u);
    }
  }
  
  return $users_filtered;
}
*/

/**
 * 
 * Returns an array of users ready to be shown in the form
 * @param $users  array of user object
 * @return      form renderable array of users in #tableselect element
 * @author Vahan
 */
function user_list_get_user_list($users) {
    
  $statuses = array('blocked', 'active');
  $options = array();
  $field_columns = array();
  
  /*$header = array(
        'username' => array('data' => t('Username'), 'field' => 'u.name'),
        'mail' => array('data' => t('Mail'), 'field' => 'u.mail'),
        'role' => array('data' => t('Role'), 'field' => 'u.role'),
      'created' => array('data' => t('Created'), 'field' => 'u.created'),
      'status' => array('data' => t('Status'), 'field' => 'u.status'),
    );*/
    
  $user_data = user_list_get_shown_user_data();
  $header = array();
  foreach ($user_data as $data) {
    $header[$data] = array("data" => $data, 'field' => 'u.'.$data);
    $user_data_columns[$data] = $data;
  }
  
  $fields = user_list_get_shown_fields();
    
  foreach ($fields as $field) {
    $header[$field['label']] = array("data" => $field['label'], 'field' => 'u.data');
    $field_columns[$field['field_id']] = $field['label'];
  }
  
  foreach ($users as $user) {
    $u = user_load($user->uid);
    
    $options[$user->uid] = array( //TODO: use user_list_get_all_user_data() instead hard coding!
        'name' => $user->name,
        'mail' =>  $user->mail,
        'role' => implode("; ", user_load($user->uid)->roles),
      'created' => format_date($user->created, 'custom', 'd-m-Y'),
        'status' =>  $statuses[$user->status],
      );
      
      foreach ($field_columns as $fid => $fname) {
        $options[$user->uid][$fname] = implode(",", user_list_user_fields($user->uid, $fid));
      }
  }

  return array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
    '#ajax' => array(
        'callback' => 'user_list_checked_callback',
        'wrapper' => 'users_selected_div',
        'event' => 'click',
       ),
      '#empty' => t('No people available.'),
    '#tree' => TRUE,
       '#multiple' => TRUE,
       '#js_select' => TRUE,
    '#input'=>true,
    '#prefix' => '<div id="users_div">',
    '#suffix' => '</div>',
  );
}

/**
 * 
 * Ajax callback for checking users
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 * @return           'users_selected' element of the form
 */
function user_list_checked_callback($form, $form_state) {
  return $form['users_selected'];
}

/**
 * 
 * The second page to enter the message and send
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 * @param array $users    TODO:UNUSED! 
 * @return array      modified form  
 */
function user_list_form_message($form, &$form_state, $users)
{
  $page_one_values = $form_state['page_values'][1];
  $users = $page_one_values['list']; //complete list of users
  
  $user_list = array(); //list of users, to whom the message will be sent
  foreach ($users as $uid => $checked) {
    if ($checked == 0) continue;
    $user_list[$uid] = user_load($uid);
  }
  
  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('mail subject'),
  );
  
  $form['message'] = array (
    '#type' => 'textarea',
    '#title' => t('Enter the message here'),
    '#suffix' => theme_user_list(array('users'=>$user_list, 'title'=>t('List of Users'))),
  );
  
  $form['submit_mail'] = array(
    '#type' => 'submit',
      '#value' => t('Send >>'),
      '#submit' => array('user_list_form_message_submit'),
      '#validate' => array('user_list_form_message_validate')
  );
  return $form;
}

/**
 * 
 * Submit handler for user_list_form message button.
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 */
function user_list_form_submit($form, &$form_state) {  
  $form_state['page_values'][1] = $form_state['values'];

  if (!empty($form_state['page_values'][2])) {
    $form_state['values'] = $form_state['page_values'][2];
  }

  // When form rebuilds, it will look at this to figure which page to build.
  $form_state['page_num'] = 2;
  $form_state['rebuild'] = TRUE;
  
  $form_state['redirect'] = 'userlist/export';
}


/**
 * 
 * Validate handler for the message button on first page.
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 */
function user_list_form_validate($form, &$form_state)
{
  $users = $form_state['values']['list'];
  foreach($users as $u) {
      if ($u !== 0) {
        return;
    }
  }
    form_set_error('values', 'Select at least one user');
}

/**
 * 
 * The page 2 submit handler.
 * This is the final submit handler. Send the entered message to the selected users
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 */
function user_list_form_message_submit($form, &$form_state) {
  $page_one_values = $form_state['page_values'][1];
  $page_two_values = $form_state['values'];
  
  $users = $page_one_values['list'];
  $subject = $page_one_values['subject'];
  $message = $page_two_values['message'];
  
  $batch = array(
    'operations' => array(array('user_list_send_message', array($users, $subject, $message))),
    'title' => t('Your messages are being sent'),
  );
  
  batch_set($batch);
}

/**
 * 
 * Sends the $message to the $users.
 * @param array $users    list of users to receive the message.
 * @param string $message  the message to be sent.
 */
function user_list_send_message($users, $subject, $message) {
  $successes = array();
  $failures = array();
  foreach($users as $uid => $checked) {
    if ($checked === 0) continue;
    $user = user_load($uid);
    $params = array('message' => $message, 'user' => $user, 'subject' => $subject);

    $result = drupal_mail('user_list', 'msg', $user->mail, user_preferred_language($user), $params, '', true);
        
    if ($result['result']) {
      array_push($successes, $result);
    }
    else {
      array_push($failures, $result);
    }
  }
  
  if (count($failures) == 0) {
    drupal_set_message('All message have been correctly sent.','status');
  }
  else {
    foreach ($failures as $f) {
      $emails .= $f['to'] . "/n";
    }
    drupal_set_message('Error while sending the messages. The following emails have not been reached: \n'.$emails,'error');
  }
}

/*
  $message = array(
    'id' => $module . '_' . $key, 
    'module' => $module, 
    'key' => $key, 
    'to' => $to, 
    'from' => isset($from) ? $from : $default_from, 
    'language' => $language, 
    'params' => $params, 
    'subject' => '', 
    'body' => array(),
  );
*/
/**
 * TODO: UNUSED!
 */
function user_list_mail ($key, &$message, $params) {
  $data['user'] = $params['user'];
  $options['language'] = $message['language'];
  user_mail_tokens($variables, $data, $options);
  
  $body = $params['message'];
  
  switch($key) {
    case 'msg':
      $langcode = $message['language']->language;
      $message['subject'] = t('Notification from !site', $variables, array('langcode' => $langcode));
      $message['body'][] = t("Dear !username\n\n$body", $variables, array('langcode' => $langcode));
      break;
  }
  
}

/**
 * 
 * Validates the entered message and if it's empty throws an error.
 * @param array $form    the form for wich this function is used.
 * @param array $form_state refernce on the array representing the state of the form
 */
function user_list_form_message_validate($form, &$form_state) {
  if(!isset($form_state['values']['message'])) {
    form_set_error('message', 'Type some text');
  }
}

/**
 * 
 * Redirects to userlist page
 */
function user_list_redirect_to_userlist() {
  drupal_goto('userlist');
}

?>