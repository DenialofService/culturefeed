<?php

function culturefeed_ui_page_remove_account_form($form, &$form_state) {
  return confirm_form($form,
    t('Are you sure you want to delete your account?'),
    isset($_GET['destination']) ? $_GET['destination'] : 'culturefeed/account/edit',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function culturefeed_ui_page_remove_account_form_submit($form, &$form_state) {
  $cf_uid = DrupalCultureFeed::getLoggedInUserId();

  try {
    DrupalCultureFeed::deleteUser($cf_uid);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('An error occurred while deleting your account.'));
    return;
  }

  db_query("DELETE FROM {culturefeed_token} WHERE cf_uid = :cf_uid", array(':cf_uid' => $cf_uid));
  db_query("DELETE FROM {culturefeed_user} WHERE cf_uid = :cf_uid", array(':cf_uid' => $cf_uid));

  global $user;

  $user->status = 0;

  user_save($user);

  module_load_include('inc', 'user', 'user.pages');

  user_logout();
}

function culturefeed_ui_page_users_search($name = '') {
  return array(
    'form' => drupal_get_form('culturefeed_ui_users_search_form'),
    'results' => _culturefeed_ui_page_users_search_results($name)
  );
}

function _culturefeed_ui_page_users_search_results($name = NULL) {
  $pagelength = CULTUREFEED_UI_PAGE_USERS_SEARCH_COUNT;
  $page = isset($_GET['page']) ? $_GET['page'] : 0;

  $query = new CultureFeed_SearchUsersQuery();

  $query->max   = $pagelength;
  $query->start = $page * $pagelength;
  $query->sort  = CultureFeed_SearchUsersQuery::SORT_CREATIONDATE;
  $query->order = CultureFeed_SearchUsersQuery::SORT_ORDER_DESCENDING;

  if (!empty($name)) {
    $query->name = $name;
  }

  try {
    $users = DrupalCultureFeed::searchUsers($query);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred'), 'error');
    return;
  }

  pager_default_initialize($users->total, $pagelength);

  $items = culturefeed_ui_users_list($users);

  if (empty($items)) {
    $content['users'] = array(
      '#markup' => t('No results found.'),
    );
  }
  else {
    $content['users'] = array(
      '#theme' => 'item_list',
      '#items' => $items,
      '#attributes' => array('class' => 'user-list'),
    );

    $content['pager'] = array(
      '#theme' => 'pager',
    );
  }

  return $content;
}

function culturefeed_ui_page_users_most_active() {
  $query = new CultureFeed_SearchUsersQuery();
  $query->max = CULTUREFEED_UI_PAGE_USERS_MOST_ACTIVE_COUNT;
  $query->order = CultureFeed_SearchUsersQuery::SORT_ORDER_DESCENDING;
  $query->sort = variable_get('culturefeed_ui_block_most_active_user_sort', CultureFeed_SearchUsersQuery::SORT_NUMBEROFACTIVEACTIVITIES);

  try {
    $users = DrupalCultureFeed::searchUsers($query);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred'), 'error');
    return;
  }

  $items = array();

  $accounts = culturefeed_get_uids_for_users($users->objects);

  foreach ($users->objects as $user) {
    if (!isset($accounts[$user->id])) {
      drupal_set_message(t('Error occurred'), 'error');
      return;
    }

    $items[] = theme('culturefeed_ui_ranked_user_summary', array('user' => $user, 'uid' => $accounts[$user->id]));
  }

  if (empty($items)) {
    $content['users'] = array(
      '#markup' => t('No users found'),
    );
  }
  else {
    $content['users'] = array(
      '#theme' => 'item_list',
      '#items' => $items,
      '#type' => 'ul',
      '#attributes' => array('class' => 'user-activity-list'),
    );
  }

  return $content;
}

function culturefeed_ui_page_mboxverify_resend() {
  try {
    $cf_account = DrupalCultureFeed::getLoggedInUser();
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred, please try again later'), 'error');
  }

  if (!empty($cf_account) && !$cf_account->mboxVerified) {
    try {
      DrupalCultureFeed::resendMboxConfirmationForUser($cf_account->id);
      drupal_set_message(t('Confirmation mail succesfully sended'));
    }
    catch (Exception $e) {
      watchdog_exception('culturefeed_ui', $e);
      drupal_set_message(t('Error occurred, please try again later'), 'error');
    }
  }
  else {
    drupal_set_message(t('Email address of this account is already confirmed.'));
  }

  drupal_goto();
}

function culturefeed_ui_page_profile_edit_form() {

  culturefeed_ui_set_profile_breadcrumb();

  $form = array();

  try {
    $cf_account = DrupalCultureFeed::getLoggedInUser();
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred'), 'error');
    return;
  }

  $form['view-profile'] = array(
    '#prefix' => '<div id="view-profile">',
    '#markup' => l(t('View profile'), 'user/' . culturefeed_get_uid_for_cf_uid($cf_account->id, $cf_account->nick)),
    '#suffix' => '</div>',
  );

  // Firstname.
  $form['givenName'] = array(
    '#type' => 'textfield',
    '#title' => t('First name'),
    '#default_value' => $cf_account->givenName,
  );
  $form['givenNamePrivacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide \'first name\' in public profile'),
    '#default_value' => $cf_account->privacyConfig->givenName == CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE,
  );

  // Name.
  $form['familyName'] = array(
    '#type' => 'textfield',
    '#title' => t('Family name'),
    '#default_value' => $cf_account->familyName,
  );
  $form['familyNamePrivacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide \'family name\' in public profile'),
    '#default_value' => $cf_account->privacyConfig->familyName == CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE,
  );

  // Picture.
  $form['#attributes']['enctype'] = 'multipart/form-data';

  $form['current_picture'] = array(
    '#prefix' => '<div class="current-picture">',
    '#theme' => 'image',
    '#path' => $cf_account->depiction . '?maxwidth=50&maxheight=50&crop=auto',
    '#suffix' => '</div>',
  );

  $form['picture'] = array(
    '#type' => 'file',
    '#size' => 26,
    '#title' => t('Choose picture'),
  );

  // Gender.
  $form['gender'] = array(
    '#type' => 'radios',
    '#title' => 'Geslacht',
    '#options' => array('male' => t('Male'), 'female' => t('Female')),
    '#default_value' => $cf_account->gender,
  );
  $form['genderPrivacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide \'gender\' in public profile'),
    '#default_value' => $cf_account->privacyConfig->gender == CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE,
  );

  // Address
  $form['street'] = array(
    '#type' => 'textfield',
    '#title' => t('Street and number'),
    '#default_value' => $cf_account->street,
  );
  $form['zip'] = array(
    '#type' => 'textfield',
    '#title' => t('Zipcode'),
    '#default_value' => $cf_account->zip,
  );
  $form['city'] = array(
    '#type' => 'textfield',
    '#title' => t('City'),
    '#default_value' => $cf_account->city,
  );
  $form['country'] = array(
    '#type' => 'select',
    '#options' => country_get_list(),
    '#title' => t('Country'),
    '#default_value' => !empty($cf_account->country) ? $cf_account->country : 'BE',
  );
  $form['homeAddressPrivacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide \'address\' in public profile'),
    '#default_value' => $cf_account->privacyConfig->homeAddress == CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE,
  );

  // Date of birth.
  $form['dob'] = array(
    '#title' => t('Date of birth'),
    '#type' => 'textfield',
    '#default_value' => $cf_account->dob ? date('d/m/Y', $cf_account->dob) : '',
    '#description' => t('Format : dd/mm/yyyy'),
    '#size' => 10,
  );
  $form['dobPrivacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide \'date of birth\' in public profile'),
    '#default_value' => $cf_account->privacyConfig->dob == CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE,
  );

  // Bio
  $form['bio'] = array(
    '#type' => 'textarea',
    '#title' => t('Biography'),
    '#default_value' => $cf_account->bio,
    '#description' => t('Maximum 250 characters'),
  );
  $form['bioPrivacy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide \'biography\' in public profile'),
    '#default_value' => $cf_account->privacyConfig->bio == CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE,
  );

  // Default language.
  $form['preferredLanguage'] = array(
    '#type' => 'select',
    '#title' => t('Preferred language'),
    '#default_value' => !empty($cf_account->preferredLanguage) ? $cf_account->preferredLanguage : '',
    '#options' => array(
      'nl' => t('Dutch'),
      'fr' => t('French'),
      'en' => t('English'),
      'de' => t('German'),
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function culturefeed_ui_page_profile_edit_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  $success = TRUE;

  // Update profile information.
  $user_update = new CultureFeed_User();

  $user_update->id         = DrupalCultureFeed::getLoggedInUserId();
  $user_update->givenName  = $values['givenName'];
  $user_update->familyName = $values['familyName'];
  $user_update->gender     = $values['gender'];
  $user_update->bio        = $values['bio'];
  $user_update->street     = $values['street'];
  $user_update->zip        = $values['zip'];
  $user_update->city       = $values['city'];
  $user_update->country    = $values['country'];
  $user_update->preferredLanguage = $values['preferredLanguage'];

  if (empty($values['dob'])) {
    $user_update->dob = '';
  }
  else {
    $dob_parts = explode('/', $values['dob']);

    if (count($dob_parts) == 3) {
      list($day, $month, $year) = $dob_parts;

      if (is_numeric($day) && is_numeric($month) && is_numeric($year))  {
        if ($date = mktime(0, 0, 0, (int)$month, (int)$day, (int)$year)) {
          $user_update->dob = $date;
        }
      }
    }
  }

  $fields = array('id', 'givenName', 'familyName', 'gender', 'bio', 'street', 'zip', 'city', 'country', 'dob');

  try {
    DrupalCultureFeed::updateUser($user_update, $fields);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred while saving your personal data.'), 'error');

    $success = FALSE;
  }

  // Upload profile picture.
  if ($_FILES['files']['tmp_name']['picture']) {
    $picture_path = $_FILES['files']['tmp_name']['picture'];

    $picture = file_get_contents($picture_path);

    if ($picture) {
      try {
        DrupalCultureFeed::uploadUserDepiction(DrupalCultureFeed::getLoggedInUserId(), $picture);
      }
      catch (Exception $e) {
        watchdog_exception('culturefeed_ui', $e);
        drupal_set_message(t('Error occurred while saving your picture.'), 'error');

        $success = FALSE;
      }
    }
  }

  // Update field privacy status.
  $privacy_config = new CultureFeed_UserPrivacyConfig();

  $privacy_config->givenName   = $values['givenNamePrivacy'] ? CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE : CultureFeed_UserPrivacyConfig::PRIVACY_PUBLIC;
  $privacy_config->familyName  = $values['familyNamePrivacy'] ? CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE : CultureFeed_UserPrivacyConfig::PRIVACY_PUBLIC;
  $privacy_config->gender      = $values['genderPrivacy'] ? CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE : CultureFeed_UserPrivacyConfig::PRIVACY_PUBLIC;
  $privacy_config->homeAddress = $values['homeAddressPrivacy'] ? CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE : CultureFeed_UserPrivacyConfig::PRIVACY_PUBLIC;
  $privacy_config->dob         = $values['dobPrivacy'] ? CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE : CultureFeed_UserPrivacyConfig::PRIVACY_PUBLIC;
  $privacy_config->bio         = $values['bioPrivacy'] ? CultureFeed_UserPrivacyConfig::PRIVACY_PRIVATE : CultureFeed_UserPrivacyConfig::PRIVACY_PUBLIC;

  try {
    DrupalCultureFeed::updateUserPrivacy(DrupalCultureFeed::getLoggedInUserId(), $privacy_config);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred while saving your privacy settings.'), 'error');

    $success = FALSE;
  }

  // Setting status message.
  if ($success) {
    drupal_set_message(t('Changes succesfully saved.'));
  }
}

function culturefeed_ui_page_account_edit() {
  return array(
    'account' => array(
      '#prefix' => '<div id="account-edit-form">',
      drupal_get_form('culturefeed_ui_page_account_edit_form'),
      '#suffix' => '</div><hr />',
    ),
    'online_accounts' => array(
      '#prefix' => '<div id="online-accounts"><h3>' . t('Connected accounts') . '</h3>',
      culturefeed_ui_online_accounts(),
      '#suffix' => '</div><hr />',
    ),
    'consumers' => array(
      '#prefix' => '<div id="manage-consumers"><h3>' . t('Connected applications') . '</h3>',
      '#markup' => t('Manage all ') . l(t('websites and applications'), 'culturefeed/serviceconsumers') . t(' who uses your UiTiD profile.'),
      '#suffix' => '</div>',
    )
  );
}

function culturefeed_ui_page_account_edit_form() {

  culturefeed_ui_set_profile_breadcrumb();

  $form = array();

  try {
    $cf_account = DrupalCultureFeed::getLoggedInUser();
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('An error occurred while loading your account, please try again later.'));
    return;
  }

  $form['view-profile'] = array(
    '#prefix' => '<div id="view-profile">',
    '#markup' => l(t('View profile'), 'user/' . culturefeed_get_uid_for_cf_uid($cf_account->id, $cf_account->nick)),
    '#suffix' => '</div>',
  );

  $form['nick'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#disabled' => TRUE,
    '#value' => $cf_account->nick,
  );

  $form['mbox'] = array(
    '#type' => 'textfield',
    '#title' => t('Email address'),
    '#default_value' => $cf_account->mbox,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $destination = url('culturefeed/account/edit', array('absolute' => TRUE, 'query' => array('closepopup' => 'true')));

  $url = DrupalCultureFeed::getUrlChangePassword($cf_account->id, $destination);

  $options = array('attributes' => array('class' => array('culturefeedconnect')), 'query' => drupal_get_destination());

  $form['change_password'] = array(
    '#prefix' => '<div id="change-password">',
    '#markup' => l(t('Change password'), $url, $options),
    '#suffix' => '</div>',
  );

  $form['remove_account'] = array(
    '#prefix' => '<div id="remove-account">',
    '#markup' => l(t('Delete account'), 'culturefeed/removeaccount'),
    '#suffix' => '</div>',
  );

  return $form;
}

function culturefeed_ui_page_account_edit_form_validate($form, &$form_state) {
  if (!valid_email_address($form_state['values']['mbox'])) {
    form_set_error('mbox', t('Invalid email'));
  }
}

function culturefeed_ui_page_account_edit_form_submit($form, &$form_state) {
  $cf_account = new CultureFeed_User();
  $cf_account->id = DrupalCultureFeed::getLoggedInUserId();
  $cf_account->mbox = $form_state['values']['mbox'];

  try {
    DrupalCultureFeed::updateUser($cf_account);
    drupal_set_message(t('Changes succesfully saved.'));
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred'), 'error');
  }
}

function culturefeed_ui_online_accounts() {
  $online_account_types = array('twitter', 'facebook', 'google');

  $items = array();

  foreach ($online_account_types as $i => $online_account_type) {
    try {
      $online_account = DrupalCultureFeed::getUserOnlineAccount($online_account_type);
    }
    catch (Exception $e) {
      watchdog_exception('culturefeed_ui', $e);
      drupal_set_message(t('Error occurred'), 'error');
      return;
    }

    $data = culturefeed_ui_online_account($online_account_type, $online_account);

    $items[] = array(
      'data' => $data,
      'id' => 'onlineaccount-' . $online_account_type,
    );
  }

  return array(
    '#theme' => 'item_list',
    '#items' => $items,
  );
}

function culturefeed_ui_page_online_account_update_privacy($online_account_type, $id, $publish, $type = 'ajax') {
  $account = new CultureFeed_OnlineAccount();
  $account->publishActivities = $publish == '1' ? TRUE : FALSE;
  $account->accountName = $id;
  $account->accountType = $online_account_type;

  $success = TRUE;

  try {
    DrupalCultureFeed::updateUserOnlineAccount(DrupalCultureFeed::getLoggedInUserId(), $account);
    $online_account = DrupalCultureFeed::getUserOnlineAccount($online_account_type);
    $online_account->publishActivities = $account->publishActivities;
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    $success = FALSE;
  }

  if ($type == 'ajax') {
    $commands = array();

    if ($success) {
      $data = culturefeed_ui_online_account($online_account_type, $online_account);
      $commands[] = ajax_command_html('#onlineaccount-' . $online_account_type, $data);
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    if (!$success) {
      drupal_set_message(t('Error occurred'), 'error');
    }
    drupal_goto();
  }
}


function culturefeed_ui_page_online_account_toggle_facebook_privacy($online_account_type, $id, $publish, $type = 'ajax') {
  $account = new CultureFeed_OnlineAccount();
  $account->publishActivities = $publish == '1' ? TRUE : FALSE;
  $account->accountName = $id;
  $account->accountType = $online_account_type;

  $success = TRUE;

  try {
    DrupalCultureFeed::updateUserOnlineAccount(DrupalCultureFeed::getLoggedInUserId(), $account);
    $online_account = DrupalCultureFeed::getUserOnlineAccount($online_account_type);
    $online_account->publishActivities = $account->publishActivities;
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    $success = FALSE;
  }

  if ($type == 'ajax') {
    $commands = array();

    if ($success) {
      $data = _culturefeed_ui_facebook_privacy_toggle();
      $commands[] = ajax_command_html('#onlineaccount-' . $online_account_type, $data);
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    if (!$success) {
      drupal_set_message(t('Error occurred'), 'error');
    }
    drupal_goto();
  }
}

function culturefeed_ui_page_online_account_delete($online_account_type, $id, $type = 'ajax') {
  $cf_uid = DrupalCultureFeed::getLoggedInUserId();

  $success = TRUE;

  try {
    DrupalCultureFeed::deleteUserOnlineAccount($cf_uid, $online_account_type, $id);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    $success = FALSE;
  }

  if ($type == 'ajax') {
    $commands = array();

    if ($success) {
      $data = culturefeed_ui_online_account($online_account_type);
      $commands[] = ajax_command_html('#onlineaccount-' . $online_account_type, $data);
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    if (!$success) {
      drupal_set_message(t('Error occurred'), 'error');
    }
    drupal_goto();
  }
}

function culturefeed_ui_page_serviceconsumers() {
  try {
    $consumers = DrupalCultureFeed::getUserServiceConsumers(DrupalCultureFeed::getLoggedInUserId());
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    drupal_set_message(t('Error occurred'), 'error');
    return;
  }

  $items = array();

  foreach ($consumers as $consumer) {
    $revoke_link = '';

    if ($consumer->consumerKey != variable_get('culturefeed_api_application_key', '')) {
      $revoke_link = array(
        '#type' => 'link',
        '#title' => t('Delete'),
        '#href' => 'culturefeed/serviceconsumers/revoke/' . $consumer->id . '/nojs',
        '#attributes' => array('class' => 'revoke-link'),
        '#options' => array('query' => drupal_get_destination()),
        '#ajax' => array(),
      );

      $revoke_link = drupal_render($revoke_link);
    }

    $items[] = array(
      'data' => theme('culturefeed_ui_service_consumer_summary', array('consumer' => $consumer, 'revoke_link' => $revoke_link)),
      'id' => 'consumer-' . $consumer->id,
    );
  }

  if (empty($items)) {
    $content['consumers'] = array(
      '#markup' => t('No channels found.'),
    );
  }
  else {
    $content['consumers'] = array(
      '#theme' => 'item_list',
      '#items' => $items,
    );
  }

  return $content;
}

function culturefeed_ui_page_serviceconsumer_revoke($consumer_id, $type = 'ajax') {
  $cf_uid = DrupalCultureFeed::getLoggedInUserId();

  $success = TRUE;

  try {
    DrupalCultureFeed::revokeUserServiceConsumer($cf_uid, $consumer_id);
  }
  catch (Exception $e) {
    watchdog_exception('culturefeed_ui', $e);
    $success= FALSE;
  }

  if ($type == 'ajax') {
    $commands = array();

    if ($success) {
      $commands[] = ajax_command_invoke('#consumer-' . $consumer_id, 'fadeOut', array('slow'));
    }

    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
  }
  else {
    if (!$success) {
      drupal_set_message(t('Error occurred'), 'error');
    }
    drupal_goto();
  }
}