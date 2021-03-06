<?php
/**
 * Theming functions for entry api ui.
 */

/**
 * Theme the opening times table.
 */
function theme_entry_api_ui_opening_times_table($variables) {

  $container = $variables['element'];

  $total_opening_times = 1;
  $rows = array();
  foreach (element_children($container) as $key) {

    $day_element = $container[$key];
    $row = array($day_element['#title']);
    $opening_times = element_children($day_element);
    $total_opening_times = count($opening_times);

    foreach ($opening_times as $opening_time) {
      $row[] = render($day_element[$opening_time]['open_from']);
      $row[] = render($day_element[$opening_time]['open_till']);
    }

    $rows[] = $row;

  }

  $table_variables = array(
    'header' => array(
      'open',
    ),
    'rows' => $rows,
  );

  while ($total_opening_times > 0) {
    $table_variables['header'][] = 'van';
    $table_variables['header'][] = 'tot';
    $total_opening_times--;
  }

  return theme('table', $table_variables);

}