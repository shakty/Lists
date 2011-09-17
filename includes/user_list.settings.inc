<?php

/**
 * 
 * Implements hook_help()
 * 
 */
function user_list_help($path, $arg) {
  switch ($path) {
    case 'admin/help#user_list':
      return t(
        '<p>User lists may be displayed alphabetically, beginning with A-Z, by newest, or as a list of users who have posted content of a certain type.</p>
        <p>Go to !admin to change the global settings for user lists, and to !block to configure specific user listing blocks if desired. Note that all blocks provided by this module are appended with "User List:", so they may be differentiated by similar blocks provided by other modules.</p>
        <p>Note that you must enable access to the user listings with the \'access user lists\' permission at !access, which controls viewing both blocks and pages. If enabled and allowed, the main user listing page may be viewed at !user_list.</p>
      	<p><h4>Search</h4></p>
      	<p><b>Search Box</b><br/>
		Use <i>Refine Search</i> for filtering the list according to the given criteria. The search syntax is the following:</br>
			FIELD_NAME_1=[TERM_11 AND TERM_12 OR TERM_13...] OR FIELD_NAME_2=[TERM_21] AND ...
		FIELD_NAME corresponds either to the name of the field in the database, declared as field_FIELD_NAME, or one of the following fields of the user table
			Name, email, role, date* (of registration)
		The same FIELD_NAME can be used several times. For AND and OR "&&", "&" and "||", "|" can be also used accordingly. The search and also the logical operators are not case sensitive. 
		*For the date a special syntax is used: date=[START_DATE, END_DATE]. If the START_DATE is skipped, i.e. date=[,END_DATE], there won\'t be a START_DATE used, and if END_DATE is skipped, i.e. date=[START_DATE,], as the upper limit the current date will be used. If only one date is given, i.e. date=[DATE], then it will be used as the starting date. All the PHP supported syntaxes can be used to specify a date.
		NOT logical operation can be also used with the terms, in the following form:
			NOT(TERM)
		While typing in the search box, smart autocomplete is provided. After typing FIELD_NAME=, "[" is added automatically. In the list provided by the autocomplete, multiple options can be selected by checking (SPACEBAR must be used) the checkboxes in front of the terms. To confirm the selection press ENTER. The terms are added with AND logical operator. For the "date" a calendar is provided.
		<br/><b>Save/Load</b><br/>
		In the Save field you can save the current save or load a previously saved search. Type the name of the search in the given text field and press Save Search button to save the current search in the database. To load a previously saved search, just select it from the given drop-down list.
		<br><b>Found Users</b></br>
		In this field a list of the users found by the search is given. You can choose which fields to show/hide by checking/unchecking the appropriate checkbox in the Choose fields to show tab.
		<br/><b>Operations with the selected users</b><br/>
		You can select users from the list and do several operations with them. Send an email by clicking the Enter the message button; you can enter the message in the next page. Export to CSV button exports the list of the selected users to a CSV file.
		<br/><b>Settings</b><br/>
		You can use the Settings link to go to the admin page and change the settings.
		</p>',
		array('!admin' => l('admin/settings/userlist', 'admin/settings/userlist'), '!block' => l('admin/structure/block', 'admin/structure/block'), '!access' => l('admin/people/permissions', 'admin/people/permissions'), '!user_list' => l('userlist', 'userlist'))
      );
  }
}

/**
 * Implementes hook_perm()
 * 
 */
function user_list_perm() {
  return array('access user lists', 'administer user lists');
}


function user_list_admin_settings() {
  $form = array();

  $form['user_list_default_number'] = array(
    '#type' => 'select',
    '#title' => t('Default Number of Users to List'),
    '#default_value' => variable_get('user_list_default_number', 10),
    '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 25, 30, 40, 50, 100, 125, 150, 200, 250, 500)),
    '#description' => t('Choose the default number of users to list on the various user list pages. Certain blocks, modules, or themes may override this value.'),
  );

  $form['user_list_types'] = array(
    '#type' => 'fieldset',
    '#title' => t('User List Types'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['user_list_types']['user_list_newest'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Newest Users List'),
    '#default' => variable_get('user_list_newest', TRUE),
    '#description' => t('If checked, then a listing of newest users will be offered, ordered by most recently registered.'),
  );

  $form['user_list_types']['alpha_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Alphabetical User Lists'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['user_list_types']['alpha_options']['user_list_alpha_listing'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Alphabetical Listings'),
    '#default_value' => variable_get('user_list_alpha_listing', TRUE),
    '#description' => t('If checked, then you may view users whose usernames begin with a specific letter of the alphabet, such as !a, !m, and !other for other than A-Z.', array('!a' => l('userlist/a', 'userlist/a'), '!m' => l('userlist/m', 'userlist/m'), '!other' => l('userlist/other', 'userlist/other'))),
  );
  $form['user_list_types']['alpha_options']['user_list_alpha_listing_caps'] = array(
    '#type' => 'checkbox',
    '#title' => t('Capitalize Letter in Menu'),
    '#default_value' => variable_get('user_list_alpha_listing_caps', FALSE),
    '#description' => t('If checked, then letters listed in the embedded or tabbed menu for links to alphabetized listings will be capitalized.'),
  );

  $form['user_list_types']['content_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content-Based User Lists'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $options = array();
  //foreach (node_get_types() as $type) {
  $types = _node_types_build()->types;
  foreach ($types as $type) { //Vahan 11.02.2011
    $options[$type->type] = $type->name;
  }
  $form['user_list_types']['content_options']['user_list_content_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Content Type User Lists'),
    '#default_value' => variable_get('user_list_content_types', array()),
    '#options' => $options,
    '#multiple' => TRUE,
    '#description' => t('Select the node types you wish to allow user lists for. For instance, you may have a list of users who have written at least one blog entry at !blog, or who have contributed at least one image at !image.', array('!blog' => l('userlist/content/blog', 'userlist/content/blog'), '!image' => l('userlist/content/image', 'userlist/content/image'))),
  );

  $form['user_list_types']['role_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Role-Based User Lists'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  
  $options = array();
  $roles = _user_list_get_roles();
  foreach ($roles as $role) { 
    $options[$role['name']] = $role['name'];
  }
  
  $form['user_list_types']['role_options']['user_list_role_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Role User Lists'),
    '#default_value' => variable_get('user_list_role_types', array()),
    '#options' => $options,
    '#multiple' => TRUE,
    '#description' => t('Select the roles you wish to allow user lists for. For instance, you may have a list of users who are of the %role role at !role_dest. NOTE: Spaces in role names are converted to underscore, but does not affect the role table.', array('%role' => current($options), '!role_dest' => l('userlist/roles/'. current($options), 'userlist/roles/'. current($options)))),
  );

  $form['user_list_menu_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Menu Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['user_list_menu_settings']['user_list_provide_page'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide User Listing Page'),
    '#default_value' => variable_get('user_list_provide_page', TRUE),
    '#description' => t('If checked, a user listing page will be provided, along with an item in the menu (which may be renamed, moved, or disabled at !admin_menu). Note that when enabled, the profile module also provides a basic user listing page with the same title of \'user list\', although its path is different and it is not listed in the menu by default. This module\'s path is !userlist, while that of the profile module is !profile.', array('!admin_menu' => l('admin/build/menu', 'admin/build/menu'), '!userlist' => l('userlist', 'userlist'), '!profile' => l('profile', 'profile'))),
  );
  $form['user_list_menu_settings']['user_list_embed_menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Embedded Menu'),
    '#default_value' => variable_get('user_list_embed_menu', TRUE),
    '#description' => t('If checked, a menu will be embedded on the user list page. Note that this bypasses the default menu tab system in Drupal 4.7, and provides its own theme which is easier in most cases to customize.'),
  );
  $form['user_list_menu_settings']['user_list_provide_menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Menu Tab'),
    '#default_value' => variable_get('user_list_provide_menu', FALSE),
    '#description' => t('If checked, a menu tab will be displayed on the user list page. Note that if the alphabetical listings are provided, this menu tab will be quite long (at least 28 available options), and will most certainly require custom theming. Customizing the theme for menu tabs in Drupal 4.7 is a tricky prospect, so unless you require this functionality and are prepared to do the custom work, it is suggested you leave this item disabled and enable the embedded menu instead, which comes with a more manageable theme and is easier to customize.'),
  );
  menu_rebuild();
  return system_settings_form($form);
}

