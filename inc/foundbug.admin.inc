<?php

/**
 * @file
 * Functions for admin menu
 *  Form builder function
 *
 * settings block
 */
function foundbug_admin_form($form, &$form_state) {
  $form = array();

  $pics = array();
  $results = file_scan_directory(drupal_get_path('module', 'foundbug'), '(bug.png)');
  foreach ($results as $pic) {
    $pics[file_create_url($pic->uri)] = '<img class = "radio_bugs" src="' . file_create_url($pic->uri) . '">';

  }
  $custom_fid = variable_get('foundbug_upload_fid', 0);
  $picpath = '';
  $file = file_load($custom_fid);
  if ((!empty($file)) && file_exists($file->uri)) {
    $picpath = file_create_url(file_load($custom_fid)->uri);
    $pics[$picpath] = '<img class = "radio_bugs"  src="'
      . $picpath
      . '" '
      . '>';
  }
  else {
    $picpath = t('Alternative');
    $pics['Alternative'] = $picpath;
  }


  $form['foundbug_imgpath'] = array(
    '#type' => 'radios',
    '#title' => t('Image for block'),
    '#default_value' => variable_get('foundbug_imgpath', '<img src="' . file_create_url(drupal_get_path('module', 'foundbug') . "/image/bug.png") . '">'),
    '#options' => $pics,
    '#description' => t('Choose pictures'),

  );
  $form['foundbug_file_picture'] = array(

    '#type' => 'managed_file',
    '#description' => t('Load your picture'),
    '#upload_location' => 'public://foundbug_files/bugs',
//.drupal_get_path('module','foundbug'),
    '#upload_validators' => array(
      'file_validate_extensions' => array('png jpg gif'),
      'file_validate_image_resolution' => array('400x400', '48x48'),


    ),
    '#submit' => array(
      'foundbug_file_exist' => array(),
    ),
    '#states' => array(

// Hide the settings when the cancel notify checkbox is disabled.

      'visible' => array(

        ':input[name="foundbug_imgpath"]' => array('value' => $picpath)
      ),
    ),

  );

  $form['foundbug_is_notice_need'] = array(
    '#title' => t('E-mail for sending notices'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('foundbug_is_notice_need', 1),
  );

  $form['foundbug_mail_to_send'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('foundbug_mail_to_send', variable_get('site_mail', 'null')),
    '#required' => TRUE,
    '#states' => array(

// Hide the settings when the cancel notify checkbox is disabled.

      'visible' => array(

        ':input[name="foundbug_is_notice_need"]' => array('checked' => TRUE)
      ),
    ),
  );

  $form['foundbug_position_side'] = array(
    '#type' => 'radios',
    '#title' => t('Position of the block'),
    '#options' => array(
      'left' => t('Letf'),
      'right' => t('Right'),

    ),
    '#default_value' => variable_get('foundbug_position_side', 'left'),
  );
  $form['foundbug_position_tdside'] = array(
    '#type' => 'radios',
    '#options' => array(
      'top' => t('Top'),
      'bottom' => t('Bottom'),

    ),
    '#default_value' => variable_get('foundbug_position_tdside', 'top'),
    '#description' => t("The side position of the block."),
  );
  $form['foundbug_offset_y'] = array(
    '#type' => 'textfield',
    '#size' => 6,
    '#maxlength' => 6,
    '#description' => t("The Y offset of the block."),
    '#default_value' => variable_get('foundbug_offset_y', '50'),
    '#required' => TRUE,

  );

  $form['foundbug_blockcolor'] = array(
    '#type' => 'textfield',
    '#title' => t('Color of the block'),
    '#default_value' => variable_get('foundbug_blockcolor', '00FF00'),
    '#size' => 6,
    '#maxlength' => 6,
    '#description' => t("The code of block color."),
    '#required' => TRUE,
  );
  $form['#submit'][] = 'foundbug_admin_form_submit';
//$form['#validate'][] = 'foundbug_custom_admin_validate';


  return system_settings_form($form);

}

/**
 * Submit from admin form
 * @param $form
 * @param $form_state
 */
function foundbug_admin_form_submit($form, &$form_state) {


  if (($form_state['values']['foundbug_file_picture']['fid'] != 0)) {
    variable_set('foundbug_upload_fid', $form_state['values']['foundbug_file_picture']['fid']);
    variable_set('foundbug_imgpath', '<img src="' . file_create_url(file_load($form_state['values']['foundbug_file_picture']['fid'])->uri) . '">');
    $form_state['values']['foundbug_imgpath'] = file_create_url(file_load($form_state['values']['foundbug_file_picture']['fid'])->uri);
  }
  $file = file_load($form_state['values']['foundbug_file_picture']['fid']);
  if (!empty($file)) {
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);
  }

  if ($form_state['values']['foundbug_offset_y'] > 50) {
    $form_state['values']['foundbug_offset_y'] = 50;
  }
  if ($form_state['values']['foundbug_offset_y'] < 0) {
    $form_state['values']['foundbug_offset_y'] = 0;
  }

}

/**
 * Validate admin form
 * @param $form
 * @param $form_state
 */
function foundbug_admin_form_validate($form, &$form_state) {


  if ($form_state['values']['foundbug_imgpath'] == 'Alternative' && empty($form_state['values']['foundbug_file_picture']['fid'])) {
    form_set_error('file_picture', t('You have not upload file.'));
  }

  $percent = $form_state['values']['foundbug_offset_y'];
  if (!is_numeric($percent)) {
    form_set_error('foundbug_offset_y', t('Incorrect offset parameter.'));
  }
  if(!preg_match('/([a-f]|[A-F]|[0-9]){3}(([a-f]|[A-F]|[0-9]){3})?\b/' , $form_state['values']['foundbug_blockcolor'])){
    form_set_error('foundbug_blockcolor', t('Incorrect color.'));
  }

}