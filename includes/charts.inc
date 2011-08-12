<?php

/**
 * 
 * Returns a drupal chart form field according to the given data
 * @param array $data data to draw the chart
 */
function user_list_charts() {
	$data = array('data1'=>1, 'data2'=>2);
	
	$form = array();	
	
	$chart = array(
		'#chart_id' => 'test_chart',
		'#title' => t('Servings'),
		'#type' => CHART_TYPE_PIE_3D,
	);

	foreach ($data as $k => $v) {
		$chart['#data'][$k] = $v;
	}
	
	$form['chart'] = $chart;
		
	return $form;
}







?>