<?php

class CultureFeedActivityConfigDetail extends CultureFeedActivityConfigBase {

  public $type = CultureFeed_Activity::TYPE_DETAIL;

  /**
   * Constructor to load default values.
   */
  public function __construct() {

    parent::__construct();
    $this->allowedTypes = array('actor', 'event', 'production', 'book', 'page');

    $this->subject = 'Bekeken door';
    $this->viewPrefix = 'heeft';
    $this->viewSuffix = 'bekeken';
  }

}