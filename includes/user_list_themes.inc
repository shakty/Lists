<?php

drupal_add_css(drupal_get_path('module', 'user_list') .'/user_list.css');
  
function theme_user_list_menu($items) {
  drupal_add_css(drupal_get_path('module', 'user_list') .'/user_list.css');
  $output = '';
  foreach ($items as $item) {
    $output .= '  <li class="leaf'. (isset($item['active']) ? ' active' : '') .'">'. l(t($item['title']), $item['path']) ."</li>\n";
  }
  return "<div class=\"menu user_list_menu\">\n<ul>\n". $output .'</ul></div>';
}

//function theme_user_list_list($op = '', $header = '', $users = array(), $empty_msg = 'There are currently no users in this category to list.', $paginated = FALSE, $number = 10, $element = 0, $include_more = NULL, $embedded_menu = '') {
function theme_user_list_list($vars = array()) { 
  //Vahan 11.03.2011
  $op = $vars[0] ? $vars[0] : '';
  $header = $vars[1] ? $vars[1] : '';
  $users = $vars[2] ? $vars[2] : array();
  $empty_msg = $vars[3] ? $vars[3] : 'There are currently no users in this category to list.';
  $paginated = $vars[4] ? $vars[4] : FALSE;
  $number = $vars[5] ? $vars[5] : 10;
  $element = $vars[6] ? $vars[6] : 0;
  $include_more = $vars[7] ? $vars[7] : NULL;
  $embedded_menu = $vars[8] ? $vars[8] : '';
  
  
  $form_id = 'user_list_form';
  $form = drupal_get_form($form_id, $users);
  //Vahan end
  
  drupal_add_css(drupal_get_path('module', 'user_list') .'/user_list.css');
  $output = '';
  $output .= "<div id=\"user_list_$op\" class=\"user_list\">\n";
  if ($header) {
    $output .= '  <h3>'. $header ."</h3>\n  ";
  }
  $output .= "  <div class=\"content\">\n    ";
  $output .= $embedded_menu;
  
  if (empty($users)) {
    $output .= $empty_msg;
  }
  else {
    $output .= drupal_render($form);
  }
  $output .= "  </div>\n  ";
  if ($include_more) {
    $output .= "  <div class=\"more\">$include_more</div>\n";
  }
  if ($paginated) {
    //$output .= theme('pager', NULL, $number, $element);
    $output .= theme('pager', array(NULL, $number, $element)); //Vahan 11.03.2011
  }
  $output .= "</div>\n";
  return $output;
}

/**
 * Creates the form including users list, a checkbox for each user and buttons to contact(email, skype) the selected users
 * Vahan, 25.03.2011
 */
function user_list_form($form, $form_state, $extra) {
	
  $form = array();
	
  $form['submit_email'] = array(
    '#type' => 'submit',
    '#value' => t('Send Email'),
  );
  
  return $form;
 
}

?>