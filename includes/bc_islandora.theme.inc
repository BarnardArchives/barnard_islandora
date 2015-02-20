<?php

/**
 * Modified version of theme_islandora_newspaper_issue_navigator.
 */
function theme_bc_islandora_newspaper_issue_navigator(&$vars) {
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
  $output = '';
  $object = $vars['object'];
  if ($page_links = _bc_islandora_np_page_pager($object)) {
    $output .= "<p><strong>Pages: </strong>{$page_links}</p>";
  }
  if ($dl_links = _bc_islandora_dl_links($object)) {
    $output .= '<p><strong>Download: </strong>' . $dl_links . '</p>'; 
  }
  $newspaper = islandora_newspaper_get_newspaper($object);
  $newspaper = $newspaper ? islandora_object_load($newspaper) : FALSE;
  $issues = $newspaper ? islandora_newspaper_get_issues($newspaper) : array();
  ksort($issues);
  //dpm($issues);
  $issues = array_keys($issues);
  if (empty($issues)) {
    $issues[] = $object->id;
  }
  $index = array_search($object->id, $issues);
  $previous_issue = isset($issues[$index - 1]) ? $issues[$index - 1] : NULL;
  $next_issue = isset($issues[$index + 1]) ? $issues[$index + 1] : NULL;
  $links = array();
  if (isset($issues[$index - 1])) {
    $previous_issue = $issues[$index - 1];
    $links[] = array(
      'title' => t('Previous issue'),
      'href' => url("islandora/object/{$previous_issue}", array('absolute' => TRUE)),
    );
  }
  if (isset($issues[$index + 1])) {
    $next_issue = $issues[$index + 1];
    $links[] = array(
      'title' => t('Next issue'),
      'href' => url("islandora/object/{$next_issue}", array('absolute' => TRUE)),
    );
  }
  if ($newspaper) {
    $links[] = array(
      'title' => t('All issues'),
      'href' => url('bulletin', array('absolute' => TRUE)),
    );
  }
  $attributes = array('class' => array('links', 'inline'));
  $output .= theme('links', array('links' => $links, 'attributes' => $attributes));
  return $output;
}

/**
 * Modified version of theme_islandora_newspaper_page_controls.
 */
function theme_bc_islandora_newspaper_page_controls(&$vars) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  global $base_url;
  $object = $vars['object'];
  $output = '';
  $page_pager = _bc_islandora_np_page_pager($object);
  if (!empty($page_pager)) {
    $output .= '<p><strong>Pages:</strong>&nbsp;';
    $output .= $page_pager;
    $output .= '</p>';
  }

  if ($dl_links = _bc_islandora_dl_links($object)) {
    $output .= '<p><strong>Download: </strong>' . $dl_links . '</p>'; 
  }
  return $output;
}

function _bc_islandora_np_page_pager($object) {
  $results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $result = reset($results);
  $parent = $result ? islandora_object_load($result['object']['value']) : islandora_object_load($object);
  $pages = $parent ? islandora_paged_content_get_pages($parent) : array();
  $pages = array_keys($pages);
  if (empty($pages)) {
    $pages[] = $object->id;
  }
  $index = array_search($object->id, $pages);
  $page_links = array();
  foreach ($pages as $i => $pid) {
    $title = $i + 1;
    if ($index !== FALSE && $i == $index) {
      $page_links[] = $title;
    }
    else {
      $page_links[] = l(t($title), "islandora/object/{$pid}");
    }
  }
  return implode($page_links, '&nbsp;');
}

function theme_bc_islandora_newspaper_issue(&$vars) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  
  $output = '';
  $object = $vars['object'];
  $context = !empty($vars['context']) ? $vars['context'] : 'issue';
  //$pages = $vars['pages'];
  $pages = $object ? islandora_paged_content_get_pages($object) : array();
  $start_page = $vars['start_page'];
  $identifiers = array();
  $thumbnails = array();
  foreach ($pages as $pid => $page) {
    $page_metadata = islandora_paged_content_get_page_metadata_from_djatoka($pid);
    $thumbnails[] = array('pid' => $pid, 'url' => "/islandora/object/{$pid}/datastream/TN/view");
    if ($start_page > 1 && $page['page'] < $start_page) {
      $identifiers[count($pages)+$page['page']-2] = $page_metadata['identifier'];
    }
    elseif ($start_page == 1) {
      $identifiers[] = $page_metadata['identifier'];
    }
    else {
      $identifiers[$page['page']-2] = $page_metadata['identifier'];
    }
  }
  ksort($identifiers);
  $osd_settings = $context == 'issue' ? array(
    'preserveViewport' => 'true',
    'showReferenceStrip' => 'true', // TODO only for newspaper issues
    'referenceStripSizeRatio' => '0.15',
  ) : array();
  $output .= theme('bc_islandora_openseadragon_viewer', array('uri' => $identifiers, 'settings' => $osd_settings));
  return $output;
}

function theme_bc_islandora_newspaper_page(&$vars) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $object = $vars['object'];
  $issue = islandora_object_load(islandora_newspaper_get_issue($object));
  $pid = $object->id;
  $page_metadata = islandora_paged_content_get_page_metadata_from_djatoka($pid);

  return theme('bc_islandora_openseadragon_viewer', array('uri' => $page_metadata['identifier'], 'settings' => array(), 'fedora_object' => $object));
}

function _bc_islandora_featured() {
  $output = '';
  // Get a random Featured Object node.
  $node_q = db_select('node', 'n')
    ->fields('n', array('nid'))
    ->condition('type', 'featured_object')
    ->condition('status', 1)
    ->range(0, 1)
    ->orderRandom();

  if ($nid = $node_q->execute()->fetchField()) {
    // Get featured object image.
    $img_q = db_select('file_managed', 'fm')
      ->fields('fm', array('uri'));
    $img_q->join('field_data_field_object_image', 'foi', 'fm.fid = foi.field_object_image_fid');
    $img_q->condition('foi.entity_id', $nid, '=');
    if ($img_uri = $img_q->execute()->fetchField()) {
      drupal_add_js(array('featured_img_path' => file_create_url($img_uri)), 'setting');
    }
    // Get caption.
    $cap_q = db_select('field_data_field_caption', 'c')
      ->fields('c', array('field_caption_value'))
      ->condition('entity_id', $nid, '=');
    if ($cap = $cap_q->execute()->fetchField()) {
      // Get pid. !
      $pid_q = db_select('field_data_field_fedora_pid', 'fp')
        ->fields('fp', array('field_fedora_pid_value'))
        ->condition('entity_id', $nid, '=');
      if ($pid = $pid_q->execute()->fetchField()) {
        $output .= l($cap, "islandora/object/{$pid}");
      }
      else {
        $output .= $cap;
      }
    }
  }

  return $output;
}

function theme_bc_islandora_openseadragon_viewer(&$variables) {
  // Variable fedora_object included in results, but not used.
  $library_path = libraries_get_path('openseadragon');
  $module_path = drupal_get_path('module', 'islandora_openseadragon');
  $variables['viewer_id'] = 'islandora-openseadragon';
  $settings = array_merge($variables['settings'] ? $variables['settings'] : array(), array(
    'id' => $variables['viewer_id'],
    'prefixUrl' => file_create_url("$library_path/images/"),
  ));

  module_load_include('inc', 'islandora_openseadragon', 'includes/utilities');
  $installed_version = islandora_openseadragon_get_installed_version();

  if (version_compare($installed_version, '0.9.82') <= 0) {
    // XXX: Version numbers don't quite line-up for the change in the nav image
    // prefix URL, so now, let the hacking begin!... Just for compatibility
    // with older versions.
    $types = array('zoomIn', 'zoomOut', 'home', 'fullpage', 'previous', 'next');
    $actions = array(
      'REST' => 'rest',
      'GROUP' => 'grouphover',
      'HOVER' => 'hover',
      'DOWN' => 'pressed',
    );
    foreach ($types as $type) {
      foreach ($actions as $action => $namepart) {
        $settings['navImages'][$type][$action] = format_string('!type_!action.png', array(
          '!type' => strtolower($type),
          '!action' => $namepart,
        ));
      }
    }
  }

  $settings = array_merge($settings, islandora_openseadragon_get_settings());
  foreach ($settings as $key => $val) {
    if (filter_var($val, FILTER_VALIDATE_FLOAT)) {
      $settings[$key] = (float) $val;
    }
    elseif (filter_var($val, FILTER_VALIDATE_INT)) {
      $settings[$key] = (int) $val;
    }
    elseif (filter_var($val, FILTER_VALIDATE_URL)) {
      $settings[$key] = check_url($val);
    }
  }

  // This is where we jam in some HOCR coordinates.
  if (isset($variables['fedora_object']) && isset($_GET['solr'])) {
    $params = $_GET['solr']['params'];
    $params['fq'][] = format_string('PID:"!pid"', array(
      '!pid' => $variables['fedora_object']->id,
    ));
    module_load_include('inc', 'islandora_paged_content', 'includes/highlighting');
    $highlighting_info = islandora_paged_content_perform_solr_highlighting_query($_GET['solr']['query'], $params);
    $highlights = array();

    foreach ($highlighting_info as $pid => $info) {
      foreach ($info['snippets'] as $snippet => $bounding_boxes) {
        foreach ($bounding_boxes as $bounding_box) {
          // The size on the x axis is used to calculate the proportional values
          // under the openseadragon coordinate system.
          $highlights[] = array(
            'x' => $bounding_box['left'] / $info['page']['width'],
            'y' => $bounding_box['top'] / $info['page']['width'],
            'width' => ($bounding_box['right'] - $bounding_box['left']) / $info['page']['width'],
            'height' => ($bounding_box['bottom'] - $bounding_box['top']) / $info['page']['width'],
            'className' => "islandora-openseadragon-highlight",
          );
        }
      }
    }
    $settings['overlays'] = $highlights;
  }
  drupal_add_js(array(
    'islandoraOpenSeadragon' => array(
      'pid' => isset($variables['fedora_object']) ? $variables['fedora_object']->id : NULL,
      'resourceUri' => $variables['uri'],
      'tileSize' => (int) variable_get('islandora_openseadragon_tile_size', '256'),
      'tileOverlap' => (int) variable_get('islandora_openseadragon_tile_overlap', '0'),
      'settings' => $settings,
    ),
  ), 'setting');

  $js_files = array(
    "$module_path/js/islandora_openseadragon.js",
    "$library_path/openseadragon.js",
    "$module_path/js/djtilesource.js",
  );
  foreach ($js_files as $filepath) {
    // Don't add js that is already in the queue.
    if (!in_array($filepath, array_keys(drupal_add_js()))) {
      drupal_add_js($filepath);
    }
  }
  drupal_add_css("$module_path/css/islandora_openseadragon.theme.css");
  //return "<div id=\"osd-toolbar\"></div>\n<div id=\"{$variables['viewer_id']}\" class=\"islandora-openseadragon\"></div>";
  return "<div id=\"{$variables['viewer_id']}\" class=\"islandora-openseadragon\"></div>";
}

function theme_bc_islandora_breadcrumb(&$vars) {
  $output = '';
  $bc = $vars['breadcrumb'];
  if (arg(2) && $object = islandora_object_load(arg(2))) {
    // Should this logic be based on the model of the object? Parent
    // collection? Both???
    $collection_results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
    $cm_results = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
    
    if (!empty($cm_results) && $cm_pid = $cm_results[0]['object']['value']) {
      $bc[] = l('Collections', 'collections');
      $cm = str_replace('islandora:', '', $cm_pid);
      switch ($cm) {
        case 'bookCModel':
          $is_book = TRUE;
          $collection_results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
          if (!empty($collection_results)) {
            foreach ($collection_results as $cr) {
              if ($cr['object']['value'] == variable_get('bc_islandora_documents_pid', 'islandora:95')) {
                $bc[] = l('Textual Materials', 'islandora/object/islandora:95');
                $is_book = FALSE;
                break;
              }
            }
          }
          if ($is_book) {
            $bc[] = l('Yearbook', 'yearbook');
          }
          break;
        // Book page.
        case 'pageCModel':
          $bc[] = l('Yearbook', 'yearbook');
          $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
          if ($book = islandora_object_load($results[0]['object']['value'])) {
            $bc[] = l($book->label, "islandora/object/{$book->id}");
          }
          //$variables['book_object_id'] = $results[0]['object']['value'];
          break;
        case 'newspaperIssueCModel':
          $bc[] = l('Newspaper', 'bulletin');
          break;
        case 'newspaperPageCModel':
          module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
          $bc[] = l('Newspaper', 'bulletin');
          $issue = islandora_newspaper_get_issue($object);
          $issue = $issue ? islandora_object_load($issue) : FALSE;
          if ($issue_link = l($issue->label, "islandora/object/{$issue}")) {
            //dpm("<p><strong>View issue: </strong>{$issue_link}</p>");
            $bc[] = $issue_link;
          }
          break;
        case 'sp_large_image_cmodel':
        case 'sp_basic_image':
          $photos_search = l('Photographs', 'islandora/search', array(
            'query' => array(
              'type' => 'dismax',
              'f[0]' => 'mods_genre_ms:photographs',
            ),
          ));
          $bc[] = $photos_search;
          break;
      }
    }
  }
  elseif (arg(0) == 'node' && is_numeric(arg(1))) {
    $node = menu_get_object();
    $is_exhibition = $node->type == 'exhibition';
    $alias = drupal_get_path_alias(implode('/', arg()));
    if (in_array($alias, array('bulletin', 'yearbook')) || $is_exhibition) {
      $bc[] = l('Collections', 'collections');
      if ($is_exhibition) {
        $bc[] = l('Exhibits', 'exhibits');
      }
    }
  }
  elseif (arg(0) == 'exhibits' && count(arg()) == 1) {
    $bc[] = l('Collections', 'collections');
  }
  if (count($bc) >= 1 && $title = menu_get_active_title()) {
    $title_parts = explode(',', $title);
    // yuk
    if ((preg_match('/^Barnard Bulletin/', $title) !== 0 && count($title_parts) == 4) ||
     (preg_match('/^Annual|Mortarboard/', $title) !== 0 && count($title_parts) == 2))  {
      $bc[] = array_pop($title_parts);
    }
    else {
      $bc[] = $title;
    }
  }
  $output .= '<div class="breadcrumb">' . implode(' » ', $bc) . '</div>';
  return $output;
}

function _bc_islandora_dl_links($obj, $datastreams = array('PDF', 'JPG')) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  $urls = array();
  foreach ($datastreams as $ds) {
    if (isset($obj[$ds])) {
      $dl_url = islandora_datastream_get_url($obj[$ds], 'download');
      $urls[] = l($ds, $dl_url);
    }
  }
  return implode('<span class="separator">&nbsp;|&nbsp;</span>', $urls);
}

function _bc_islandora_is_document($object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $output = '';
  if (is_object($object->relationships)) {
    $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
    if (!empty($results) && $parent_collection = $results[0]['object']['value']) {
      $collection = islandora_object_load($parent_collection);
      if ($collection->id == variable_get('bc_islandora_documents_pid', '')) {
        return TRUE;
      }
      else { 
        return FALSE;
      }
    }
  }
}

function _bc_islandora_is_genre($object, $genre = array()) {
  if ($mods = simplexml_load_string($object['MODS']->getContent(NULL))) {
    if (isset($mods->genre) && in_array((string) $mods->genre, $genre)) {
      return TRUE;
    }
  }
  return FALSE;
}

function _bc_islandora_get_sequence($object) {
  $seq_results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
  if (!empty($seq_results)) {
    return $seq_results[0]['object']['value'];
  }
  else {
    return NULL;
  }
}
