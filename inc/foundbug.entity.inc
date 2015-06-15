<?php
/**
 * @file
 * Entity functions
 */

/*
 * load page with bugreports
 *
 * @return content with table
 */
function foundbug_list() {
  if (!user_access('administer bugreport entities')) {
    drupal_access_denied();
    return;
  }
  $content = array();
  // Load all of our entities.
  $entities = bugreport_load_multiple();
  if (!empty($entities)) {
    drupal_add_library('system', 'drupal.ajax');
    drupal_add_library('system', 'jquery.form');
    foreach ($entities as $entity) {
      // Create tabular rows for our entities.
      $fix = 'fix';
      if ($entity->fixed == 0) {
        $fix = t('<div id=' . $entity->id . ' class="foundbug_fix">fix</div>');
      }
      else {
        $fix = t('<div  id=' . $entity->id . ' class="foundbug_fixed">fixed</div>');
      }

      $rows[] = array(
        'data' => array(
          'id' => $entity->id,
          'body' => l($entity->body, 'foundbug/report/' . $entity->id),
          'fix' => l($fix, 'foundbug/report/' . $entity->id . '/fix/js', array(
            'attributes' => array('class' => array('use-ajax')),
            'html' => TRUE
          )),
          'delete' => l('<div class ="foundbug_delete">delete</div>', 'foundbug/report/' . $entity->id . '/delete', array(
            'attributes' => array('class' => array('use-ajax')),
            'html' => TRUE
          ))
        ),
        'id' => array('id_' . $entity->id),
      );
    }
    // Put our entities into a themed table. See theme_table() for details.
    $content['entity_table'] = array(
      '#theme' => 'table',
      '#rows' => $rows,
      '#header' => array(t('ID'), t('Body'), t('Fix'), t('Delete')),
    );
  }
  else {
    // There were no entities. Tell the user.
    $content[] = array(
      '#type' => 'item',
      '#markup' => t('No bugreport entities currently exist.'),
    );
  }
  return $content;
}
/*
 * implement hook_entity_info
 */
function foundbug_entity_info() {

  $info = array();

  $info['bugreport'] = array(
    'label' => t('Bugreport'),
    'base table' => 'bugreports',
    'entity class' => 'Bugreport',
    'controller class' => 'BugreportController',
    //'creation callback' => 'bugreport_create',
    'load hook' => 'bugreport_load',
    //'fieldable' => TRUE,
    'uri callback' => 'bugreport_uri',
    'entity keys' => array(
      'id' => 'id',
    ),
    'view modes' => array(
      'full' => array(
        'label' => t('Default'),
        'custom settings' => FALSE,
      ),
    ),
    'module' => 'foundbug',
    'bundles' => array(
      'first_example_bundle' => array(
        'label' => t('First example bundle'),
        // 'admin' key is used by the Field UI to provide field and
        // display UI pages.
        'admin' => array(
          'path' => 'admin/structure/foundbug/manage',
          'access arguments' => array('administer bugreport entities'),
        ),
      ),
    ),
  );

  return $info;
}

/*
 * Fetch a bugreport object.
 * function for load entity. Use bugreport_load_multiple.
 *
 * @param int $basic_id
 *   Integer specifying the entity id.
 * @param bool $reset
 *   A boolean indicating that the internal cache should be reset.
 * @return object
 *   A fully-loaded $basic object or FALSE if it cannot be loaded.
 *
 */
function bugreport_load($basic_id = NULL, $reset = FALSE) {
  $basic_ids = (isset($basic_id) ? array($basic_id) : array());
  $basic = bugreport_load_multiple($basic_ids, array(), $reset);
  return $basic ? reset($basic) : FALSE;
}

/*
 * Use for multiple load entities
 */
function bugreport_load_multiple($basic_ids = FALSE, $conditions = array(), $reset = FALSE) {
  return entity_load('bugreport', $basic_ids, $conditions, $reset);
}

class Bugreport extends Entity {

  public $body = " ";
  public $email = " ";
  public $weight = 0;
  public $fixed = 0;

  public function __construct($values = array()) {
    if (in_array('body', $values)) {
      $this->body = $values['body'];
    }
    if (in_array('email', $values)) {
      $this->email = $values['email'];
    }
    parent::__construct($values, 'bugreport');
  }

  protected function defaultLabel() {
    return $this->$body;
  }


}

/*
 * class controler for bugreport
 */

class BugreportController extends EntityAPIController {
  public function __construct($entityType) {
    parent::__construct($entityType);
  }

  /*
   * create entity
   *
   * @param array values
   *  array of values to create
   */
  public function create(array $values = array()) {
    // Add values that are specific to our Entity

    $values += array(
      'is_new' => TRUE,
      'body' => $values['body'],
      'email' => $values['email'],
      'created' => time(),
      'name' => $values['name'],
      'env' => $values['env'],
    );

    $ent = parent::create($values);
    return $ent;
  }


  /*
   *  delete entity
   */
  public function delete($entity) {
    $this->deleteMultiple(array($entity));
  }

  /*
   * delete multiple entities
   */
  public function deleteMultiple($entities) {
    $ids = array();
    if (!empty($entities)) {
      $transaction = db_transaction();
      try {
        foreach ($entities as $entity) {
          // Invoke hook_entity_delete().
          module_invoke_all('entity_delete', $entity, 'bugreport');
          field_attach_delete('bugreport', $entity);
          $ids[] = $entity->id;
        }
        db_delete('bugreports')
          ->condition('id', $ids, 'IN')
          ->execute();
      } catch (Exception $e) {
        $transaction->rollback();
        watchdog_exception('bugreport', $e);
        throw $e;
      }
    }
  }
}

/**
 * Implements the uri callback.
 */
function bugreport_uri($entity) {
  return array(
    'path' => 'foundbug/report/' . $entity->id,
  );
}

/*
 * the representation of entity
*/
function bugreport_view($entity, $view_mode = 'tweaky') {

  $entity_type = 'bugreport';
  $entity->content = array(
    '#view_mode' => $view_mode,
  );
  //field_attach_prepare_view($entity_type, array($entity->id => $entity),$view_mode);
  //entity_prepare_view($entity_type, array($entity->id => $entity));

  if (!empty($entity->path)) {
    $entity->content['image'] = array(
      '#type' => 'item',
      '#markup' => '<img class="bugreport-image" src=' . file_create_url($entity->path) . '>',
    );
  }

  $entity->content['bugreport'] = array(
    '#type' => 'item',
    '#title' => t('Bugreport'),
    '#markup' => $entity->body,
  );
  $entity->content['name'] = array(
    '#type' => 'item',
    '#id' => 'PS',
    '#markup' => "Name: " . $entity->name,
  );
  $entity->content['email'] = array(
    '#type' => 'item',
    '#id' => 'PS',
    '#markup' => "Email: " . $entity->email,
  );
  $entity->content['created'] = array(
    '#type' => 'item',
    '#id' => 'PS',
    '#markup' => "Created: " . format_date($entity->created),
  );
  $entity->content['env'] = array(
    '#type' => 'item',
    '#id' => 'PS',
    '#markup' => "Cookie: " . $entity->env,
  );
  global $language;
  $langcode = $language->language;
  module_invoke_all('entity_view', $entity, $entity_type, $view_mode,
    $langcode);
  drupal_alter(array('bugreport_view', 'entity_view'),
    $entity->content, $entity_type);

  // And finally return the content.
  return $entity->content;
}

/*
 * function for change state fix of bugreport
 *
 * @param object $entity
 *  the entity to fix
 * @param bool $ajax
 *  is ajax is turned on
 */
function bugreport_fix($entity, $ajax) {

  //permission check need
    $result = array('#type' => 'ajax');
    if ($entity->fixed == 0) {
      $entity->fixed = 1;
      $result['#commands'][] = ajax_command_replace('.foundbug_fix#' . $entity->id, '<div id=' . $entity->id . ' class="foundbug_fixed">fixed</div>');
    }
    else {
      $entity->fixed = 0;
      $result['#commands'][] = ajax_command_replace('.foundbug_fixed#' . $entity->id, '<div id=' . $entity->id . ' class="foundbug_fix">fix</div>');
    }
    entity_get_controller('bugreport')->save($entity);


  return $result;
}

/*
 * function for delete of bugreport
 *
 * @param object $entity
 *  the entity to delete
 * @param bool $ajax
 *  is ajax is turned on
 */
function bugreport_delete($entity) {
  entity_get_controller('bugreport')->delete($entity);

  $result = array('#type' => 'ajax');


  $result['#commands'][] = ajax_command_invoke('#id_' . $entity->id, 'hide');

  return $result;


}
