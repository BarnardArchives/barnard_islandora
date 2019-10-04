<?php

/**
 * Uncorrected transcript edit form
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object.
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   The Drupal form definition.
 */
function bc_islandora_oralhistories_uncorrected_transcript_edit_form(array $form, array &$form_state, AbstractObject $object) {
  if (isset($object['UNCORRECTED_TRANSCRIPT'])) {
    $transcript = $object['UNCORRECTED_TRANSCRIPT'];

    $form = array();
    $form_state['object'] = $object;
    $form_state['uc_transcript'] = $transcript;
    $text = $transcript->content;
    $form['text-replace'] = array(
      '#title' => 'Transcript Text',
      '#type' => 'textarea',
      '#value' => $text,
    );

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );

    return $form;
  }
}

/**
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function bc_islandora_oralhistories_uncorrected_transcript_edit_form_submit(array $form, array &$form_state) {
  $object = $form_state['object'];
  $transcript = $form_state['uc_transcript'];
  $new_transcript = $form_state['text-replace'];
  kpr($form);
  kpr($form_state);

//  try {
//    $transcript->setContentFromString($new_transcript);
//    drupal_set_message(t('The @dsid datastream has been updated', array('@dsid' => $transcript->id)), 'status');
//    $form_state['redirect'] = "islandora/object/$object->id";
//  }
//  catch (Exception $e) {
//    drupal_set_message(t('Error updating datastream @dsid: @msg', array('@dsid' => $transcript->id, '@msg' => $e->getMessage())), 'error');
//  }

}
