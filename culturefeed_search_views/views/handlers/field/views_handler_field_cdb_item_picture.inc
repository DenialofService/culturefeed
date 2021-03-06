<?php

/**
 * A handler to provide proper displays for the picture of a cdb item.
 */
class views_handler_field_cdb_item_picture extends views_handler_field {

  /**
   * @see views_handler_field::option_definition()
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['width'] = array('default' => 100);
    $options['height'] = array('default' => 100);

    return $options;
  }

  /**
   * @see views_handler_field::options_form()
   */
  function options_form(&$form, &$form_state) {

    $form['width'] = array(
      '#type' => 'textfield',
      '#title' => t('Width'),
      '#default_value' => isset($this->options['width']) ? $this->options['width'] : '',
      '#required' => TRUE,
    );

    $form['height'] = array(
      '#type' => 'textfield',
      '#title' => t('Height'),
      '#default_value' => isset($this->options['height']) ? $this->options['height'] : '',
      '#required' => TRUE,
    );

    parent::options_form($form, $form_state);

  }

  /**
   * @see views_handler_field::render()
   */
  function render($extended_entity) {

    $detail = $extended_entity->getEntity()->getDetails()->getDetailByLanguage(culturefeed_search_get_preferred_language());
    if (!$detail) {
      return;
    }

    $main_picture = '';
    $pictures = $detail->getMedia()->byMediaType(CultureFeed_Cdb_Data_File::MEDIA_TYPE_PHOTO);
    foreach ($pictures as $picture) {
      if ($picture->isMain()) {
        $main_picture = $picture->getHLink();
      }
    }

    if (empty($main_picture) && $pictures && $pictures->count() > 0) {
      $pictures->rewind();
      $picture = $pictures->current();
      $main_picture = $picture->getHLink();
    }

    if (!empty($main_picture)) {
      return theme('image', array('path' => $main_picture . '?crop=auto&width=' . $this->options['width'] . '&height=' . $this->options['height']));
    }

  }

}
