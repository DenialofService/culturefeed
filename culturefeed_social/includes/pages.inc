<?php
/**
 * @file
 * Page callbacks for culturefeed social.
 */

/**
 * Page callback to get the activities for current requested page.
 */
function culturefeed_social_page_ajax_get_activities() {

  $requested_page = isset($_GET['page']) ? $_GET['page'] : 0;

  $query = new CultureFeed_SearchActivitiesQuery();
  $query->max = CULTUREFEED_SOCIAL_BLOCK_USER_ACTIVITIES_COUNT;
  $query->start = $requested_page * CULTUREFEED_SOCIAL_BLOCK_USER_ACTIVITIES_COUNT;

  if (isset($_GET['culturefeed_uid'])) {
    $query->userId = $_GET['culturefeed_uid'];
  }

  // Filter on type.
  if (isset($_GET['type'])) {
    $query->type = array($_GET['type']);
  }
  else {
    $query->type = array(
      CultureFeed_Activity::TYPE_LIKE,
      CultureFeed_Activity::TYPE_FACEBOOK,
      CultureFeed_Activity::TYPE_TWITTER,
      CultureFeed_Activity::TYPE_IK_GA,
    );
  }

  $query->private = FALSE;

  $result = new stdClass();
  $result->results = array();
  $result->show_pager = FALSE;
  try {

    $activities = DrupalCultureFeed::searchActivities($query);
    $accounts = culturefeed_get_uids_for_activities($activities->objects);

    foreach ($activities->objects as $activity) {
      $result->results[] = theme('culturefeed_social_user_activity_summary', array('activity' => $activity, 'uid' => $accounts[$activity->userId]));
    }

    $result->show_pager = ($query->max  + $query->start) < $activities->total;

  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
  }

  // If we are applying, and we have no results. Make sure we show a no results message.
  if (isset($_GET['new_filter']) && empty($result->results)) {
    $result->results[] = 'Geen activiteiten gevonden';
  }

  drupal_json_output($result);

}

/**
 * Page callback for the management of the activities of current user.
 */
function culturefeed_social_page_activities($type = NULL) {

  $types = culturefeed_social_get_activity_filter_options(TRUE);

  // If a type was passed, check if it's valid.
  if (!empty($type) && !in_array($type, array_keys($types))) {
    drupal_not_found();
  }

  // Set page and pagelength.
  $pagelength = CULTUREFEED_SOCIAL_PAGE_USER_ACTIVITIES_COUNT;
  $page = isset($_GET['page']) ? $_GET['page'] : 0;

  // Build the query.
  $query = new CultureFeed_SearchActivitiesQuery();

  $query->userId      = DrupalCultureFeed::getLoggedInUserId();
  $query->private     = TRUE;
  $query->max         = $pagelength;
  $query->start       = $page * $pagelength;

  if (!empty($type)) {
    $query->type = $type;
  }

  $cf_account = DrupalCultureFeed::getLoggedInUser();
  $build['view-profile'] = array(
    '#prefix' => '<div id="view-profile">',
    '#markup' => l('Bekijk profiel', 'user/' . culturefeed_get_uid_for_cf_uid($cf_account->id, $cf_account->nick)),
    '#suffix' => '</div>',
  );

  // Do the query.
  try {
    $activities = DrupalCultureFeed::searchActivities($query);
  }
  catch (Exception $e) {
    drupal_set_message('Er is een fout opgetreden.', 'error');
    return $build;
  }

  // Initialize the pager.
  pager_default_initialize($activities->total, $pagelength);

  $rows = array();

  foreach ($activities->objects as $activity) {
    $delete = array(
      '#type' => 'link',
      '#title' => 'Verwijderen',
      '#href' => 'culturefeed/activity/delete/' . $activity->id . '/nojs',
      '#attributes' => array('class' => 'remove-link'),
      '#options' => array('query' => drupal_get_destination()),
      '#ajax' => array(),
    );

    $privacy = array(
      '#type' => 'link',
      '#title' => $activity->private ? 'Privé' : 'Publiek',
      '#href' => 'culturefeed/activity/privacy/' . $activity->id . '/' . ($activity->private ? '0' : '1') . '/nojs',
      '#attributes' => array('id' => 'activity-privacy-' . $activity->id, 'class' => 'privacy-link ' . ($activity->private ? 'status-prive' : 'status-publiek'), 'title' => ($activity->private ? 'Zet publiek' : 'Maak privé')),
      '#options' => array('query' => drupal_get_destination()),
      '#ajax' => array(),
    );

    $type_text = isset($types[$activity->type]) ? $types[$activity->type] : '';

    if ($activity->contentType == 'page') {
      $data = l($activity->nodeTitle, 'node/' . $activity->nodeId);
    }
    elseif (in_array($activity->contentType, array('actor', 'event', 'production'))) {
      $data = culturefeed_search_detail_l($activity->contentType, $activity->nodeId, $activity->nodeTitle);
    }

    $rows[] = array(
      'id' => 'activity-' . $activity->id,
      'data' => array(
        $data,
        $type_text,
        format_interval(REQUEST_TIME - $activity->creationDate) . ' geleden <br />op ' . $activity->createdVia,
        drupal_render($privacy),
        drupal_render($delete),
      ),
    );
  }

  $activities = array(
    '#theme' => 'table',
    '#header' => array('Pagina', 'Actie', 'Tijdstip en publicatiekanaal', 'Publiek', ''),
    '#rows' => $rows,
    '#empty' => 'Je hebt nog geen acties gedaan.',
  );

  $facebook_privacy_toggle = NULL;
  $facebook = _culturefeed_ui_facebook_privacy_toggle();
  if ($facebook) {
    $item_list = array(
      '#theme' => 'item_list',
      '#items' => array(
        array(
          'data' => $facebook,
          'id' => 'onlineaccount-facebook',
        ),
      ),
    );
    $facebook_privacy_toggle = array(
      '#markup' => drupal_render($item_list),
    );
  }

  $build['filter'] = drupal_get_form('culturefeed_social_user_activity_filter_form', $type);
  $build['facebook_privacy_toggle'] = $facebook_privacy_toggle;
  $build['activities'] = $activities;
  $build['pager'] = array(
    '#theme' => 'pager',
  );

  return $build;

}

/**
 * Page callback to delete an activity.
 */
function culturefeed_social_page_activity_delete($id, $type = 'ajax') {

  $success = TRUE;

  try {
    DrupalCultureFeed::deleteActivity($id);
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_social', $e);
  }

  if ($type == 'ajax') {
    $commands = array();

    if ($success) {
      $commands[] = ajax_command_invoke('#activity-' . $id, 'fadeOut', array('slow'));
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    drupal_goto();
  }

}

/**
 * Page callback to change the privacy settings of 1 activity.
 */
function culturefeed_social_page_activity_privacy($id, $privacy, $type = 'ajax') {

  $success = TRUE;

  try {
    DrupalCultureFeed::updateActivity($id, $privacy == 1);
  }
  catch (Exception $e) {
    $success = FALSE;
    watchdog_exception('culturefeed_ui', $e);
  }

  if ($type == 'ajax') {
    if ($success) {
      $commands = array();
      $privacy = array(
        '#type' => 'link',
        '#title' => $privacy == 1 ? 'Privé' : 'Publiek',
        '#href' => 'culturefeed/activity/privacy/' . $id . '/' . ($privacy == 1 ? '0' : '1') . '/nojs',
        '#attributes' => array('id' => 'activity-privacy-' . $id, 'class' => 'privacy-link ' . ($privacy == 1 ? 'status-prive' : 'status-publiek'), 'title' => ($privacy == 1 ? 'Zet publiek' : 'Maak privé')),
        '#options' => array('query' => drupal_get_destination()),
        '#ajax' => array(),
      );

      $commands[] = ajax_command_replace('#activity-privacy-' . $id, drupal_render($privacy));
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    drupal_goto();
  }

}
