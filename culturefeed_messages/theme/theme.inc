<?php
/**
 * @file
 * Theming / preprocess functions for culturefeed messages.
 */

/**
 * Preprocess a messages list.
 * @see culturefeed-messages-message-list.tpl.php
 */
function culturefeed_messages_preprocess_culturefeed_messages_message_list(&$variables) {

  $items = array();
  if ($variables['messages']->total == 0) {
    $items[] = 'U hebt geen berichten';
  }
  else {
    foreach ($variables['messages']->objects as $message) {
      $items[] = theme('culturefeed_messages_message_list_item', array('message' => $message));
    }
  }

  $variables['items'] = $items;

}

/**
 * Preprocess a message list item.
 * @see culturefeed-messages-message-list-item.tpl.php
 */
function culturefeed_messages_preprocess_culturefeed_messages_message_list_item(&$variables) {

  _culturefeed_messages_preprocess_message($variables);

  if (arg(0) == 'pages') {
    $variables['url'] = url('pages/' . arg(1) . '/messages/' . $variables['message']->id);
  }
  else {
    $variables['url'] = url('culturefeed/messages/' . $variables['message']->id);
  }

}

/**
 * Preprocess a message thread.
 * @see culturefeed-messages-thread.tpl.php
 */
function culturefeed_messages_preprocess_culturefeed_messages_thread(&$variables) {

  $thread = $variables['thread'];

  $variables['message_count'] = 1;
  $variables['sender'] = $thread->sender->nick;
  if (!empty($thread->senderPage)) {
    $variables['sender'] .= ' van ' . $thread->senderPage->getName();
  }

  $variables['messages'] = array();
  $variables['messages'][] = theme('culturefeed_messages_message', array('message' => $thread));

  $thread_children = $thread->children;
  if (!empty($thread_children)) {
    $variables['message_count'] += count($thread_children);
    foreach ($thread_children as $message) {
      $variables['messages'][] = theme('culturefeed_messages_message', array('message' => $message));
    }
  }

  $delete_options = array(
    'attributes' => array(
      'role' => 'button',
      'data-toggle' => 'modal',
      'data-target' => '#thread-delete-confirm',
      'data-remote' => url('culturefeed/messages/' . $thread->id . '/delete/ajax'),
    ),
  );
  $variables['delete_link'] = l('Verwijder conversatie', 'culturefeed/messages/' . $thread->id . '/delete/nojs', $delete_options);

}

/**
 * Preprocess a message item.
 * @see culturefeed-messages-message.tpl.php
 */
function culturefeed_messages_preprocess_culturefeed_messages_message(&$variables) {

  _culturefeed_messages_preprocess_message($variables);

  $message = $variables['message'];

  // @todo move to helper function
  $recipients = array();
  // Recipients from a page.
  if (!empty($message->recipientPage)) {

    try {
      $user_list = DrupalCultureFeed::getLoggedInUserInstance()->pages()
      ->getUserList($message->recipientPage->getId(), array($message->role));
    }
    catch (Exception $e) {
      watchdog_exception('culturefeed_messages', $e);
    }

  }
  else {
    $recipients[$message->sender->id] = $message->sender->nick;
  }

  $accounts = culturefeed_get_uids_for_cf_uids($recipients);
  $recipient_links = array();
  foreach ($variables['recipients'] as $uid => $recipient) {

    if (!isset($accounts[$uid])) {
      continue;
    }

    $recipient_links[] = l($recipient, 'user/' . $accounts[$uid]);

  }
  $variables['recipient_links'] = implode(' ', $recipient_links);

}

/**
 * Helper function to preprocess the general variables for a message.
 */
function _culturefeed_messages_preprocess_message(&$variables) {

  $message = $variables['message'];

  $variables['id'] = $message->id;
  $variables['status'] = $message->status;
  $variables['subject'] = $message->subject;
  $variables['body'] = $message->body;

  $variables['sender'] = $message->sender->nick;
  if (!empty($message->senderPage)) {
    $variables['sender'] .= ' van ' . $message->senderPage->getName();
  }

  $date = empty($message->lastReply) ? $message->creationDate : $message->lastReply;

  // Check if we should show x minutes ago, or not.
  $time_ago = $_SERVER['REQUEST_TIME'] - $date;
  if ($time_ago < (60 * 60)) {
    $variables['date'] = $minutes_ago . ' minuten geleden';
  }
  else {

    $time_zone  = new DateTimeZone('Europe/Brussels');
    $last_reply = new DateTime();
    $last_reply->setTimestamp($date);
    $last_reply->setTimezone($time_zone);

    $days_ago = $last_reply->diff(new DateTime('now', $time_zone))->d;
    if ($days_ago == 1) {
      $variables['date'] = 'Gisteren om ' . $last_reply->format('H:i') . ' uur';
    }
    elseif ($days_ago == 0) {
      $variables['date'] = 'Vandaag om ' . $last_reply->format('H:i') . ' uur';
    }
    else {
      $variables['date'] = format_date($date, 'custom', 'd M \o\m H:i') . ' uur';
    }

  }

  $variables['picture'] = !empty($message->user->depiction) ? theme('image', array('path' => $message->user->depiction . '?maxwidth=100')) : '';

}