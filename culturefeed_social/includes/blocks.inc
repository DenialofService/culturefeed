<?php
/**
 * @file
 * Block callbacks for the social module.
 */

function culturefeed_social_block_user_activities($account) {

  if (empty($account->culturefeed_uid)) {
    return;
  }

  $query = new CultureFeed_SearchActivitiesQuery();
  $query->max = CULTUREFEED_SOCIAL_BLOCK_USER_ACTIVITIES_COUNT;
  $query->type = array(
    CultureFeed_Activity::TYPE_LIKE,
    CultureFeed_Activity::TYPE_FACEBOOK,
    CultureFeed_Activity::TYPE_TWITTER,
    CultureFeed_Activity::TYPE_IK_GA,
  );
  $query->userId = $account->culturefeed_uid;
  $query->private = FALSE;

  try {
    $activities = DrupalCultureFeed::searchActivities($query);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    return;
  }

  if (empty($activities->objects)) {
    $content['activities'] = array(
      '#markup' => 'Deze gebruiker heeft nog geen acties gedaan.',
    );
  }
  else {

    $activity_filters = culturefeed_social_get_activity_filter_options(TRUE);
    if (!empty($activity_filters)) {
      $content['filter'] = drupal_get_form('culturefeed_social_user_activity_filter_form', 'all', TRUE, $account);
    }

    $content['activities'] = array(
      '#theme' => 'culturefeed_social_user_activity_list',
      '#activities' => $activities,
      '#class' => 'user-activity-list',
      '#attached' => array('js' => array(drupal_get_path('module', 'culturefeed_social') . '/js/activities.js')),
    );

    if ($activities->total > count($activities->objects)) {
      $content['activities']['#pager_path'] = url('culturefeed/ajax/user/activities', array('query' => array('culturefeed_uid' => $account->culturefeed_uid)));
    }

    if ($account->culturefeed_uid == DrupalCultureFeed::getLoggedInUserId()) {
      $content['edit'] = array(
        '#markup' => l('Beheer jouw acties', 'culturefeed/activities', array('attributes' => array('class' => array('manage-activities-link')))),
      );

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
        $content['facebook'] = array(
          '#markup' => drupal_render($item_list),
        );
      }

    }

  }

  $block['subject'] = 'Recente acties';
  $block['content'] = $content;

  return $block;

}