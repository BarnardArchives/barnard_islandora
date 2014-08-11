<?php

function theme_bc_islandora_exhibit($vars) {
  global $base_url;
  $node = isset($vars['node']) ? $vars['node'] : NULL;
  $output = '<div id="exhibition-nav">';
  $output .= '<span class="exhibition-prev"><a href="#">prev</a></span>';
  $output .= '<span class="exhibition-next"><a href="#">next</a></span>';
  $output .= '</div>';
  $output .= '<div class="exhibition">';
  if ($node && $node->type == 'exhibition') {
    $exhibition_objects = array();
    foreach (field_get_items('node', $node, 'field_object') as $i => $obj) {
      if (isset($obj['node'])) {
        $pid = !empty($obj['node']->field_fedora_pid) ? array_shift(field_get_items('node', $obj['node'], 'field_fedora_pid')) : NULL;
        $description = !empty($obj['node']->field_description) ? array_shift(field_get_items('node', $obj['node'], 'field_description')) : '';
        $caption = !empty($obj['node']->field_caption) ? array_shift(field_get_items('node', $obj['node'], 'field_caption')) : array('value' => '');
        $obj_img = !empty($obj['node']->field_object_image) ? array_shift(field_get_items('node', $obj['node'], 'field_object_image')) : NULL;
        $title = $obj['node']->title;
        $f_o = $pid ? islandora_object_load($pid['value']) : NULL;
        $img_url_m = $pid ? url($base_url . '/islandora/object/' . $pid['value'] . '/datastream/JPG/view') : NULL;
        //$img_url_l = url($base_url . '/islandora/object/' . $pid['value'] . '/datastream/JP2/view');
        $img_url_l = $img_url_m;
        $output .= '<div class="exhibition-object ' . $i . '">';
        $output .= '<h2>' . $title . '</h2>';
        $output .= '<div class="object-description">';
        $output .= '<span class="description">' . $description['value'] . '</span>';
        $output .= '</div>';
        if ($img_url_m || $obj_img) {
          $output .= '<div class="object-image">';
          if ($obj_img) {
            $obj_img['path'] = file_create_url($obj_img['uri']);
            $output .= '<a href="' . $obj_img['path']  . '" data-lightbox="' . $pid['value'] . '" data-title="' . $caption['value'] . '">';
            $output .= theme('image', $obj_img);
          }
          else {
            $output .= '<a href="' . $img_url_l . '" data-lightbox="' . $pid['value'] . '" data-title="' . $caption['value'] . '">';
            $output .= '<img src="' . $img_url_m . '">';
          }
          $output .= '</a>';
          $output .= '<span class="caption">' . $caption['value'] . '</span>';
          // TODO
          $output .= '<span class="dc-link"><p>(' . l(t('View in Digital Collections.'), 'islandora/object/'. $pid['value']) . ')</p</span>';
          $output .= '</div>';
        }
        $output .= '</div>';
      }
    }
  }

  $output .= '</div>';
  return $output;
}

/**
 * Modified version of theme_islandora_newspaper_issue_navigator.
 */
function theme_bc_islandora_newspaper_issue_navigator(&$vars) {
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');

  $object = $vars['object'];
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
  return theme('links', array('links' => $links, 'attributes' => $attributes));
}

/**
 * Modified version of theme_islandora_newspaper_page_controls.
 */
function theme_bc_islandora_newspaper_page_controls(&$vars) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  global $base_url;
  $view_prefix = '<strong>' . t('View:') . ' </strong>';
  $download_prefix = '<strong>' . t('Download:') . ' </strong>';
  $object = $vars['object'];
  $issue = islandora_newspaper_get_issue($object);
  $issue = $issue ? islandora_object_load($issue) : FALSE;
  $controls = array(
    //'page_select' => theme('islandora_newspaper_page_select', array('object' => $object)),
    'page_pager' => theme('islandora_paged_content_page_navigator', array('object' => $object)),
    'issue_pager' => ($issue ?
      theme('islandora_newspaper_issue_navigator', array('object' => $issue)) :
      ''),
  );

  // Text view.
  if (isset($object['OCR']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['OCR'])) {
    $url = islandora_datastream_get_url($object['OCR'], 'view');
    $attributes = array('attributes' => array('title' => t('View text')));
    $controls['text_view'] = $view_prefix . l(t('Text'), $url, $attributes);
  }
  // PDF download.
  if (isset($object['PDF']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['PDF'])) {
    $size = islandora_datastream_get_human_readable_size($object['PDF']);
    $text = t('PDF (@size)', array('@size' => $size));
    $url = islandora_datastream_get_url($object['PDF'], 'download');
    $attributes = array('attributes' => array('title' => t('Download PDF')));
    $controls['pdf_download'] = $download_prefix . l($text, $url, $attributes);
  }
  if (isset($object['JP2']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JP2'])) {
    // JP2 download.
    $size = islandora_datastream_get_human_readable_size($object['JP2']);
    $text = t('JP2 (@size)', array('@size' => $size));
    $url = islandora_datastream_get_url($object['JP2'], 'download');
    $attributes = array('attributes' => array('title' => t('Download JP2')));
    $controls['jp2_download'] = $download_prefix . l($text, $url, $attributes);

    // JP2 Clip.
    $jp2_ds = $object['JP2'];
    $module_path = drupal_get_path('module', 'islandora_newspaper');
    $path = "$module_path/images/clip_icon.png";
    $image = theme('image', array('path' => $path, 'attributes' => array()));
    $url = "islandora/object/{$object->id}/print";
    $attributes = array(
      'attributes' => array(
        'title' => t('Clip Image'),
        'id' => 'clip',
      ),
      'html' => TRUE,
    );
    $controls['clip'] = l($image, $url, $attributes);
  }
  // TIFF download.
  if (isset($object['OBJ']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['OBJ'])) {
    $size = islandora_datastream_get_human_readable_size($object['OBJ']);
    $text = t('TIFF (@size)', array('@size' => $size));
    $url = islandora_datastream_get_url($object['OBJ'], 'download');
    $attributes = array('attributes' => array('title' => t('Download TIFF')));
    $controls['tiff_download'] = $download_prefix . l($text, $url, $attributes);
  }
  return theme('item_list', array('items' => $controls, 'attributes' => array('class' => array('items', 'inline'))));
}


function theme_bc_islandora_newspaper_issue(&$vars) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  drupal_add_js(libraries_get_path('openseadragon') . '/openseadragon.js', array('group' => JS_LIBRARY));
  
  $output = '';
  $object = $vars['object'];
  //$pages = $vars['pages'];
  $pages = islandora_paged_content_get_pages($object);
  $start_page = $vars['start_page'];
  $identifiers = array();
  foreach ($pages as $pid => $page) {
    $page_metadata = islandora_paged_content_get_page_metadata_from_djatoka($pid);
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
  $output .= theme('bc_islandora_openseadragon_viewer', array('uri' => $identifiers));
  return $output;
}

function theme_bc_islandora_newspaper_page(&$vars) {
  $object = $vars['object'];
  $issue = islandora_object_load(islandora_newspaper_get_issue($object));
  $pid = $object->id;
  $page_results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');

  if (!empty($page_results)) {
    $start_page = $page_results[0]['object']['value'];
  }
  else {
    $start_page = array_pop(explode('-', $pid));
  }
  return theme('bc_islandora_newspaper_issue', array('object' => $issue, 'start_page' => $start_page));
}

function theme_bc_islandora_book($vars) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $output = '';
  $object = $vars['object'];
  $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');

  if (!empty($results)) {
    $collection = islandora_object_load($results[0]['object']['value']);
    if ($collection->id == variable_get('bc_islandora_documents_pid', '')) {
      $output .= theme('bc_islandora_newspaper_issue', array('object' => $object));
    }
  }
  else {
    $output .= theme('islandora_book_book', array('object' => $object));
  }
  return $output;
}

function theme_bc_islandora_featured(&$vars) {
  $output = '';
  $nid = $vars['nid'] ? $vars['nid'] : variable_get('bc_islandora_featured_nid', NULL);
  // If we didn't get an nid, get the newest Featured Object node.
  if (!$nid) {
    $node_q = db_select('node', 'n')
      ->fields('n', array('nid'))
      ->condition('type', 'featured_object');
    $nid = $node_q->execute()->fetchField();
    var_export($nid);
    if (!$nid) {
      return $output;
    }
  }
  // Get featured object image.
  $img_q = db_select('file_managed', 'fm')
    ->fields('fm', array('uri'));
  $img_q->join('field_data_field_object_image', 'foi', 'fm.fid = foi.field_object_image_fid');
  $img_q->condition('foi.entity_id', $nid, '=');
  if ($img_uri = $img_q->execute()->fetchField()) {
    drupal_add_js(array('featured_img_path' => file_create_url($img_uri)), 'setting');
  }
  // What if we didn't get image???
  else {
  }
  // Get featured object caption.
  $caption_q = db_select('field_data_field_caption', 'fc')
    ->fields('fc', array('field_caption_value'))
    ->condition('fc.entity_id', $nid, '=');
  if ($caption = $caption_q->execute()->fetchField()) {
    $output .= 'Featured: ';
    // Get featured object PID.
    $pid_q = db_select('field_data_field_fedora_pid', 'fp')
      ->fields('fp', array('field_fedora_pid_value'))
      ->condition('fp.entity_id', $nid, '=');
    if ($pid = $pid_q->execute()->fetchField()) {
      $obj_url = url('islandora/object/' . $pid);
      $output .= l(t($caption), $obj_url);
    }
    else {
      $output .= $caption;
    }
  }
  return $output;
}

function theme_bc_islandora_openseadragon_viewer(&$variables) {
  // Variable fedora_object included in results, but not used.
  $library_path = libraries_get_path('openseadragon');
  $module_path = drupal_get_path('module', 'islandora_openseadragon');
  $variables['viewer_id'] = 'islandora-openseadragon';
  $settings = array(
    'id' => $variables['viewer_id'],
    'prefixUrl' => file_create_url("$library_path/images/"),
    'toolbar' => 'osd-toolbar',
  );

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
  dpm($settings);
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
  drupal_add_js("$module_path/js/islandora_openseadragon.js");
  drupal_add_js("$library_path/openseadragon.js");
  drupal_add_js("$module_path/js/djtilesource.js");
  drupal_add_css("$module_path/css/islandora_openseadragon.theme.css");
  return "<div id=\"osd-toolbar\"></div>\n<div id=\"{$variables['viewer_id']}\" class=\"islandora-openseadragon\"></div>";
}

function theme_bc_islandora_breadcrumb(&$vars) {
  $output = '';
  $bc = $vars['breadcrumb'];
  if (arg(2) && $object = islandora_object_load(arg(2))) {
    // Should this logic be based on the model of the object? Parent
    // collection? Both???
    //$results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
    $results = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
    if (!empty($results) && $parent_collection = $results[0]['object']['value']) {
      switch ($parent_collection) {
        // Do something based on what the parent collection is.
        // (If there is a landing page, add it to the breadcrumb!)
        default:
          dpm($parent_collection);
          break;
      }
    }
  }
  else {
    // Do something else if we couldn't load an object?
  }
  $bc[] = menu_get_active_title();
  $output .= '<div class="breadcrumb">' . implode(' » ', $bc) . '</div>';
  return $output;
}

function _bc_islandora_breadcrumb() {
  $breadcrumb = drupal_get_breadcrumb();
  if (array_shift(arg()) == 'view') {
    $type = array_shift(explode('-', arg(1)));
    $path = drupal_get_normal_path($type);
    if ($node = menu_get_item($path)) {
      $breadcrumb[] = l($node['title'], $path);
    }
  }
  return theme_breadcrumb(array('breadcrumb' => $breadcrumb));
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

function _bc_islandora_get_sequence($object) {
  $seq_results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
  if (!empty($seq_results)) {
    return $seq_results[0]['object']['value'];
  }
  else {
    return NULL;
  }
}
