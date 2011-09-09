<?php

/**
 * @file	Includes functions to implement a filtering search from the given list and based on the given search string.
 * 			The following search structure is used:
 * 			keyword1=['value11' and 'value12' or not(value13)] and keyword2=['value21' and 'value22' or not(value23)] or ....
 * 			note that and/or/not are case insencitive, even more, you can use &&/||/! and &/|/! accordingly
 * 			also, '' are not compulsory, but if you have a reserved word in the searched value (e.g. 'Big and Cool') 
 * 			you have to put the ''.
 * 			For the date a bit different syntax is used:
 * 			date=[start_date, end_date]. Any (but only one) limit date can be skipped provided that 
 * 			you put the comma in the correct place. In that case, the limits are set to +/-infinity. 
 * 			If there is one date without the comma, it will be used as start_date.
 * 
 * 			Basicly you need only user_list_filter_users to use outside this function.
 */


/**
 * 
 * Returns the filtered users array according to the given keyword.
 * @param array $users		initial list among wich the filtered list is created and returned.
 * @param string $haystack	searched string to use for the filter. OR and AND are used as logical operators.
 * @return				 	filtered list from the given list and based on the haystack.
 */
function user_list_filter_users($users, $haystack) {
	if (empty($haystack) || trim($haystack) == "") return $users;
	/*if (stripos($haystack, "=[") === FALSE) {
		return user_list_filter_users_by_value($users, $haystack);
	}*/
	
	$haystack = user_list_prerender($haystack); //important to make a correct search string

	return user_list_handle_search_string($users, $haystack, user_list_filter_users_by_keyword, null);
}

/**
 * 
 * Prepares the string for the search, in particular:
 * 1. Takes care about the quotes - ''
 * 2. Sets correct case for and/or/not, that is lower case inside the keyword=[...] and upper case outside.
 * 3. Adds % before and after the logical opeartors (e.g. %and%)
 * @param string $string	search string of the following form: key1=[val1 AND/OR val2...] AND/OR key2=[...]...
 * 							where all AND/OR can be in any letters, e.g. aNd, And, OR,... 
 * @return					new string of the form: key1=[val1 and/or val2...] AND/OR key2=[...]...
 * 							where all outer AND/OR are in capital letters and inners are in lower. 
 */
function user_list_prerender($string) {	
	$new_string = " ";
	$words = explode("\"", " ".$string);
	//
	foreach ($words as $i=>$w) {
		if ($i%2 === 1) {
			$new_string .= $w;
			continue;
		}
		$new_w = str_ireplace(array(" & ", " && ", " and "), " %and% ", $w);
		$new_w = str_ireplace(array(" | ", " || ", " or "), " %or% ", $new_w);
		$new_w = str_ireplace(array(" !(", " not("), " %not%(", $new_w);
		$new_string .= $new_w;
	}
	
	//replace the outer or/and/not by upper letters
	$exp1 = explode("]", $new_string);
	$exp1_replaced = array();
	foreach ($exp1 as $str) {
		$exp2 = explode("=[", $str);
		$exp2[0] = str_ireplace(" %and% ", " %AND% ", $exp2[0]);
		$exp2[0] = str_ireplace(" %or% ", " %OR% ", $exp2[0]);
		$exp2[0] = str_ireplace(" %not%(", " %NOT%(", $exp2[0]);
		$exp1_replaced[] = implode("=[", $exp2);
	}
	$new_string = implode("]", $exp1_replaced);
	
	return $new_string;
}


/**
 * 
 * Renders the given string based using the inner_search_function, 
 * which is either user_list_filter_users_by_keyword or user_list_filter_users_by_value. 
 * This function is used to avoid the copy-pasted code for the searches inside and outside the keyword=[...].
 * @param array $users						initial list among wich the filtered list is created and returned.
 * @param string $keyword					string to search for in the given list as a keyword.
 * @param function $inner_search_function	the function that actually implements the search. Possible values are 
 * 											user_list_filter_users_by_keyword and user_list_filter_users_by_value.
 * @param string $param						is used as a key in key=value syntax cases, 
 * 											otherwise it is not used and should be set NULL
 * @return									filtered list from the given list and based on the haystack.
 */
function user_list_handle_search_string($users, $keyword, $inner_search_function, $param) {
	$users_not = array(); //results to be excluded
	$exploded_by_nots = explode(user_list_get_search_deliminator($inner_search_function, "%NOT%("), $keyword);
	p("exploded_by_nots = "); p($exploded_by_nots);
	if (count($exploded_by_nots) > 1) {
		$skip = true; //Used to skip the first element of the array
		foreach ($exploded_by_nots as $not) {
			if($skip) { //check if it's the first element
				$skip = false;
				continue; //skip the first element
			}
			p("not = "); p($not);
			$not_keyword = substr($not, strlen("%NOT%("), strpos($not, ")")-strlen("%NOT%("));
			$users_not = array_merge($users_not, $inner_search_function($users, $not_keyword, null));
		}
	}
	$keywords_without_OR = explode(user_list_get_search_deliminator($inner_search_function, " %OR% "), $keyword);
	$users_filtered = array();
	foreach ($keywords_without_OR as $k_without_OR) {
		$keywords = explode(user_list_get_search_deliminator($inner_search_function, " %AND% "), $k_without_OR);
		$users_filtered_temp = array();
		$step = 0;
		foreach ($keywords as $k) {
			$matching_users = $inner_search_function($users, $k, $param);
			if ($step == 0) {
				$users_filtered_temp = $matching_users;
				$step++;
			}
			else {
				$users_filtered_temp = user_list_user_array_intersect_by_uid($users_filtered_temp, $matching_users);
			}
		}
		$users_filtered = array_merge($users_filtered, $users_filtered_temp);
	}
	/*if (count($users_filtered) == 0) { //return a full list if nothing is found
		$users_filtered = $users;
	}*/
	return user_list_user_array_diff_by_uid($users_filtered, $users_not);
}


/**
 * 
 * Filters and returns the users array according to only the given keyword. 
 * @param array $users		initial list among wich the filtered list is created and returned.
 * @param string $keyword	string to search for in the given list as a keyword.
 * @return					filtered list from the given list and based on the haystack.
 */
function user_list_filter_users_by_keyword($users, $keyword) {	
	$users_filtered = array();
	
	$exploded = explode("=", $keyword);
	$key = $exploded[0];
	$values = substr($exploded[1], strpos($exploded[1], "[")+1, strpos($exploded[1], "]")-strpos($exploded[1], "[")-1); //get the string inside [ ]  //strpos($exploded[1], "]") !== FALSE ? 1 : 0
	if (count($exploded) < 2) {	//genereal search is used
		$users_filtered = user_list_filter_users_by_value($users, $key, $keyword, false);
	}
	elseif (count($exploded) == 2) { //key=[values] syntax is used in the search
		$users_filtered = user_list_handle_search_string($users, $values, user_list_filter_users_by_value, $key);
	}
	else $users_filtered = array();
	
	return $users_filtered;
}

/**
 * 
 * Filters and returns the users array according to only the given value and the key.
 * Note that key can be only one of the fixed values "name", "role", "mail", "date" or "null" and is case insencitive.
 *  @param array $users		initial list among wich the filtered list is created and returned.
 *  @param string $value	string to search for in the fixed keywords as a value.
 *  @param string $key		string to be used as a keyword in the search.
 *  						if is set to null (or not passed ot the function), then $specified is set to false automatically.
 *  @param bool $specified	if a keyword is specified or not.
 *  @return					filtered list from the given list and based on the haystack.
 *  
 */
function user_list_filter_users_by_value($users, $value, $key=null, $specified=true) {
	$value = trim($value);
	if ($key === null) $specified = false;
	$key = strtoupper($key);
	$users_filtered = array();
	foreach ($users as $u) {
		if ((($key === "NAME" || !$specified) && stripos($u->name, $value) !== FALSE) || 
				(($key === "ROLE" || !$specified) && user_list_has_role($u->uid, $value)) ||
				(($key === "MAIL" || !$specified) && stripos($u->mail, $value) !== FALSE) ||
				(($key === "DATE" || !$specified) && user_list_date_is_in_interval($u->created, $value))
			) {
			array_push($users_filtered, $u);
		}
		$all_fields = user_list_get_all_fields();
		foreach ($all_fields as $field) {
			if ((strtoupper($key) === strtoupper(user_list_get_field_keyword($field)) || !$specified) && user_list_contains_array($value, user_list_user_fields($u->uid, user_list_get_fid_by_fname($field['field_name'])))) {
				array_push($users_filtered, $u);
			}
		}
	}
	return $users_filtered;
}


/**
 * 
 * Returns the correct case (lower or upper) of the given deliminator based on the used search function.
 * @param function $inner_search_function	the actual search function used. 2 fucntions are possible:
 * 											user_list_filter_users_by_value and user_list_filter_users_by_keyword
 * @param string $deliminator				the given logical operator (and/or/not) to switch to the correct case:
 * 											lower for user_list_filter_users_by_value and 
 * 											upper for user_list_filter_users_by_keyword.
 * @return									deliminator in the according case.
 */
function user_list_get_search_deliminator($inner_search_function, $deliminator) {
	if ($inner_search_function == user_list_filter_users_by_value) {
		return strtolower($deliminator);
	}
	elseif ($inner_search_function == user_list_filter_users_by_keyword) {
		return strtoupper($deliminator);
	}
	return FALSE;
}

/**
 * Returns the name of the table where the saved searches are stored.
 * @return	the name of the table where the saved searches are stored.
 */
function user_list_get_save_table_name() {
	return 'user_list_searches';
}

/**
 * 
 * Returns an array of the current user's saved searches.
 * @return	all the saved searched of the logged in user.
 */
function user_list_get_all_searches() {
	global $user;
	$save_schema = drupal_get_schema(user_list_get_save_table_name());
	$result = db_select($save_schema['name'], 's')
				->fields('s', array('sid', 'uid', 'name', 'search', 'url'))
				->condition("uid", $user->uid)
				->execute();

	$searches = array();
	foreach ($result as $row) {
		array_push($searches, $row);
	}
	return $searches;
}

/**
 * 
 * Returns an array with saved search details.
 * @param string $search_name	the name of the saved search.
 * @return						array with the following keys: 'sid', 'uid', 'name', 'search', 'url'.
 */
function user_list_get_search($search_name) {
	global $user;
	$save_schema = drupal_get_schema(user_list_get_save_table_name());
	$result = db_select($save_schema['name'], 's')
				->fields('s', array('sid', 'uid', 'name', 'search', 'url'))
				->condition("name", $search_name)
				->condition("uid", $user->uid)
				->execute();

	foreach ($result as $row) {
		return $row;
	}
}


/**
 * Returns a string composed of the values of the given string of form key=value
 * UNUSED
 */
/*function user_list_search_remove_key($text) {
	$keywords = explode(" ", $text);
	$ret = "";
	foreach ($keywords as $keyword) {
		$pair = explode("=", $keyword);
		$value = $pair[count($pair)];
		if (count($pair) == 1 && strtoupper($value) != "%OR%" && strtoupper($value) != " %AND% " && strtoupper($value) != " %NOT%( ") {
			$ret .= $pair[count($pair)];
		}
	}
	
	return $ret;
}*/ 


?>