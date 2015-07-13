<?php

/**
 * @file
 * Theming functions for Barnard Digital Collections.
 */

/**
 * Generates themed newspaper issue navigator output.
 *
 * @see theme_islandora_newspaper_issue_navigator()
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
 * Generates themed newspaper page controls output.
 *
 * @see theme_islandora_newspaper_page_controls()
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

/**
 * Helper function to generate newspaper pager.
 */
function _bc_islandora_np_page_pager($object) {
  $results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $result = reset($results);
  $parent = $result ? islandora_object_load($result['object']['value']) : islandora_object_load($object);
  $pages = $parent ? islandora_paged_content_get_pages($parent) : array();
  $pages = array_keys($pages);
  if (empty($pages)) {
    // The page object pid.
    $pages[] = $object->id;
  }
  // The index of the current page in the issue.
  $index = array_search($object->id, $pages);
  $page_links = array();
  foreach ($pages as $i => $pid) {
    $title = $i + 1;
    // Output just a title if we're at the current page.
    if ($index !== FALSE && $i == $index) {
      $page_links[] = $title;
    }
    // Otherwise output a link.
    else {
      $page_links[] = l($title, "islandora/object/{$pid}");
    }
  }
  return implode($page_links, '&nbsp;');
}

/**
 * Generates themed newspaper issue output.
 *
 * @see template_preprocess_islandora_newspaper_issue()
 */
function theme_bc_islandora_newspaper_issue(&$vars) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $output = '';
  $object = $vars['object'];
  $context = !empty($vars['context']) ? $vars['context'] : 'issue';
  $pages = $object ? islandora_paged_content_get_pages($object) : array();
  $start_page = $vars['start_page'];
  $identifiers = array();
  $thumbnails = array();
  // Populate $thumbnails and $identifiers.
  foreach ($pages as $pid => $page) {
    $page_metadata = islandora_paged_content_get_page_metadata_from_djatoka($pid);
    $thumbnails[] = array('pid' => $pid, 'url' => "/islandora/object/{$pid}/datastream/TN/view");
    if ($start_page > 1 && $page['page'] < $start_page) {
      $identifiers[count($pages) + $page['page'] - 2] = $page_metadata['identifier'];
    }
    elseif ($start_page == 1) {
      $identifiers[] = $page_metadata['identifier'];
    }
    else {
      $identifiers[$page['page'] - 2] = $page_metadata['identifier'];
    }
  }
  ksort($identifiers);
  // Extra settings to pass to the OpenSeadragon viewer.
  $osd_settings = $context == 'issue' ? array(
    'preserveViewport' => 'true',
    'showReferenceStrip' => 'true',
    'referenceStripSizeRatio' => '0.15',
  ) : array();
  $output .= theme('bc_islandora_openseadragon_viewer', array('uri' => $identifiers, 'settings' => $osd_settings));
  return $output;
}

/**
 * Generates themed newspaper page output.
 *
 * @see template_preprocess_islandora_newspaper_page()
 */
function theme_bc_islandora_newspaper_page(&$vars) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $object = $vars['object'];
  $issue = islandora_object_load(islandora_newspaper_get_issue($object));
  $pid = $object->id;
  $page_metadata = islandora_paged_content_get_page_metadata_from_djatoka($pid);

  return theme('bc_islandora_openseadragon_viewer', array(
    'uri' => $page_metadata['identifier'],
    'settings' => array(),
    'fedora_object' => $object,
  ));
}

/**
 * Generates themed featured object output for the front page.
 *
 * Called by barnard_theme's template.php.
 */
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

/**
 * Generates OpenSeadragon viewer output and accompanying JavaScript and CSS.
 * 
 * We reimplement this in order to pass in custom settings via
 * $variables['settings']. If we could set these values through the
 * islandora_openseadragon admin page, this wouldn't be necessary...
 */
function theme_bc_islandora_openseadragon_viewer(&$variables) {
  // Variable fedora_object included in results, but not used.
  $library_path = libraries_get_path('openseadragon');
  $module_path = drupal_get_path('module', 'islandora_openseadragon');
  $variables['viewer_id'] = 'islandora-openseadragon';
  // Here's where we look for additional custom settings.
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
  return "<div id=\"{$variables['viewer_id']}\" class=\"islandora-openseadragon\"></div>";
}

/**
 * Generates themed breadcrumb output.
 */
function theme_bc_islandora_breadcrumb(&$vars) {
  $output = '';
  $bc = $vars['breadcrumb'];

  // Is this an islandora object?
  if (arg(2) && $object = islandora_object_load(arg(2))) {
    $bc[] = l(t('Collections'), 'collections');
    $collection_results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
    $cm_results = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
    $mods = isset($object['MODS']) ? simplexml_load_string($object['MODS']->getContent(NULL)) : NULL;
    if ($mods) {
      $identifier = preg_replace('/^BC/', '', array_shift(explode('_', (string) $mods->identifier)));
      switch ($identifier) {
        // Mortarboard.
        case '12-01':
          $bc[] = l(t('Yearbook'), 'yearbook');
          // Is it a page (= "member of" a book, not just book collection)?
          // If so, link to the parent book.
          if ($book_link = _bc_islandora_book_link($object, variable_get('bc_islandora_yearbook_pid', 'islandora:bookCollection'))) {
            $bc[] = $book_link;
          }
          break;

        // Student Publications.
        case '12-04':
        case '12-05':
        case '12-06':
        case '12-11':
        case '12-14':
        case '12-18':
        case '12-22':
        case '12-23':
        case '12-24':
          $bc[] = l(t('Student Publications'), 'student-publications');
          $is_pub = TRUE;
          if ($pub_link = _bc_islandora_book_link($object, variable_get('bc_islandora_pubs_pid', 'islandora:1022'))) {
            $bc[] = $pub_link;
          }
          break;

        // "Special" student publications: don't provide issue link.
        case '12-09':
        case '12-26':
          $bc[] = l(t('Student Publications'), 'student-publications');
          break;

        // Bulletin.
        case '12-03':
          module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
          $bc[] = l(t('Newspaper'), 'bulletin');
          // Is it a page? If so, link to parent issue.
          $issue_pid = islandora_newspaper_get_issue($object);
          if ($issue = islandora_object_load($issue_pid)) {
            $bc[] = l($issue->label, "islandora/object/{$issue->id}");
          }
          break;

        default:
          $genre = $mods && isset($mods->genre) ? (string) $mods->genre : NULL;
          // If a photo, generate search link for genre = photo.
          if ($genre == 'photographs') {
            $bc[] = l(t('Photographs'), 'islandora/search', array(
              'query' => array(
                'type' => 'dismax',
                'f[0]' => 'mods_genre_ms:"' . $genre . '"',
              ),
            ));
          }
          // If not a photo, generate links based on relatedItem metadata.
          else {
            $links = array();
            foreach ($mods->relatedItem as $ri) {
              if ($host_title = (string) $ri->titleInfo->title) {
                $bc_link = l($host_title, 'islandora/search', array(
                  'query' => array(
                    'type' => 'dismax',
                    'f[0]' => 'mods_relatedItem_host_titleInfo_title_ms' . ':"' . $host_title . '"',
                  ),
                ));
              }
              // Record group.
              if (isset($ri['displayLabel']) && $ri['displayLabel'] == 'Record Group') {
                $links['rg'] = $bc_link;
              }
              // Collection.
              elseif (isset($bc_link)) {
                $links['c'] = $bc_link;
              }
            }
            // Record group precedes collection in breadcrumb sequence.
            if (isset($links['rg'])) {
              $bc[] = $links['rg'];
            }
            if (isset($links['c'])) {
              $bc[] = $links['c'];
            }
          }
          break;
      }
    }
  }
  // Is this a landing page or exhibition node?
  elseif (arg(0) == 'node' && is_numeric(arg(1))) {
    $node = menu_get_object();
    $is_exhibition = $node->type == 'exhibition';
    $alias = drupal_get_path_alias(implode('/', arg()));
    if (in_array($alias, array('bulletin', 'yearbook'))) {
      $bc[] = l(t('Collections'), 'collections');
    }
    elseif ($is_exhibition) {
      $bc[] = l(t('Exhibits'), 'exhibits');
    }
  }

  // Add the current page's title to the existing breadcrumb.
  if (count($bc) >= 1 && $title = menu_get_active_title()) {
    $title_parts = explode(',', $title);
    $last_part = array_pop($title_parts);
    // In these special cases, use whatever text occurs after the last comma
    // (e.g. "issue 1") as the title.
    if (array_shift(explode(' ', preg_replace('/^ /', '', $last_part))) == 'page' || $is_pub) {
      $bc[] = $last_part;
    }
    else {
      $bc[] = $title;
    }
  }

  $output .= '<div class="breadcrumb">' . implode(' » ', $bc) . '</div>';
  return $output;
}

/**
 * Provides a link to a parent book object.
 */
function _bc_islandora_book_link($object, $parent_collection) {
  $parent = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $book = islandora_object_load($parent[0]['object']['value']);
  // Don't generate a link to islandora:bookCollection or whatever.
  if ($book->id != $parent_collection) {
    return l($book->label, "islandora/object/{$book->id}");
  }
  return NULL;
}

/**
 * Generates themed exhibit output.
 *
 * @todo A lot of this stuff could be done on the front end using AJAX calls...
 */
function theme_bc_islandora_exhibition($args) {
  $node = $args['node'];
  $objects = array();
  $themes = array();
  $slides_output = $img_output = '';
  $themes_output = '<div id="ex-themes">';
  // Create a structured list of exhibition objects.
  foreach ($node->field_object[$node->language] as $i => $object) {
    $objects[$i]['description'] = isset($object['node']->field_description[$node->language]) ?
      check_markup($object['node']->field_description[$node->language][0]['value'], $object['node']->field_description[$node->language][0]['format']) :
      '';
    $objects[$i]['pid'] = isset($object['node']->field_fedora_pid[$node->language]) ?
      $object['node']->field_fedora_pid[$node->language][0]['value'] :
      '';
    $objects[$i]['layout'] = isset($object['node']->field_layout[$node->language]) ?
      _bc_islandora_get_ex_layout($object['node']->field_layout[$node->language][0]['value']) :
      '';
    $objects[$i]['theme'] = $object['node']->field_theme[$node->language][0]['taxonomy_term']->name;
    $images = isset($object['node']->field_images[$node->language]) ?
      $object['node']->field_images[$node->language] :
      array();
    $objects[$i]['images'] = array();
    foreach ($images as $j => $fc_item) {
      $fc_id = $fc_item['value'];
      $fc_entity = entity_load('field_collection_item', array($fc_id));
      $objects[$i]['images'][$j] = array(
        'uri' => $fc_entity[$fc_id]->field_image[$node->language][0]['uri'],
        'caption' => $fc_entity[$fc_id]->field_caption[$node->language][0]['value'],
        'pid' => $fc_entity[$fc_id]->field_fedora_pid[$node->language][0]['value'],
      );
    }
  }
  // Cycle through objects and generate output.
  foreach ($objects as $i => $item) {
    // Anchor values for current, next, and previous page.
    $pg_frag = 'page' . strval($i + 1);
    $prev_frag = 'page' . ($i ? strval($i) : strval(count($objects)));
    $next_frag = 'page' . ($i != count($objects) - 1 ? strval($i + 2) : '1');
    $first = !$i;
    $img_output = '';
    $slides_output .= '<div class="ex-obj';
    // Generate a new theme link if we didn't get this theme yet.
    if (!in_array($item['theme'], $themes)) {
      $themes[] = $item['theme'];
      $themes_output .= l($item['theme'], request_path(), array(
        'attributes' => array(
          'class' => 'theme',
          'data-theme' => $item['theme'],
        ),
        'fragment' => $pg_frag,
      ));
    }
    if ($first) {
      $slides_output .= ' first';
    }
    $slides_output .= '" data-layout="' . $item['layout'];
    $slides_output .= '" data-theme="' . $item['theme'];
    $slides_output .= '" id="slide' . strval($i + 1) . '">';
    $slides_output .= '<div class="ex-images">';
    $tn_output = '<div class="ex-thumbnails">';
    $show_tn = count($item['images']) > 1;
    // Generate image output (including thumbnails).
    foreach ($item['images'] as $j => $img) {
      $first_img = !$j;
      $img_index = strval($j + 1);
      $img_path = variable_get('file_public_path', conf_path() . '/files') . '/' . file_uri_target($img['uri']);
      $img_output .= '<div class="large-image';
      if ($first_img) {
        $img_output .= ' first';
      }
      $img_output .= '" id="' . $img_index . '">';
      // (Uses the image style "exhibit" to get image URL.)
      $img_output .= l(theme('image_style', array(
          'style_name' => 'exhibit',
          'path' => $img['uri'],
        )),
        $img_path,
        array(
          'attributes' => array(
            'rel' => 'slide' . $i,
            'class' => 'colorbox-load',
            'title' => $img['caption'],
          ),
          'html' => TRUE,
        )
      );
      $img_output .= '<br/>';
      $img_output .= '<span class="caption">' . $img['caption'];
      if ($img['pid']) {
        $img_output .= '&nbsp;(' . l(t('View object'),
          'islandora/object/' . $img['pid'], array(
            'attributes' => array('target' => '_blank'),
          )
        ) . ')';
      }
      $img_output .= '</span>';
      $img_output .= '</div>';
      // Show thumbnails below image if more than one image attached to slide.
      if ($show_tn) {
        $tn_id = 'tn' . $img_index;
        // (Uses the image style "thumbnail" to get image URL.)
        $tn_output .= l(theme('image_style', array(
            'style_name' => 'thumbnail',
            'path' => $img['uri'],
            'attributes' => array('id' => $tn_id),
          )),
          request_path(),
          array(
            'fragment' => $pg_frag . '#' . $tn_id,
            'html' => TRUE,
          )
        );
      }
    }
    $tn_output .= '</div>';
    $img_output .= $tn_output;
    $slides_output .= $img_output . '</div>';
    $slides_output .= '<div class="ex-txt">' . $item['description'] . '</div>';
    // Navigation links.
    $slides_output .= '<div id="ex-nav">';
    $slides_output .= l(t('« Prev'), request_path(), array(
      'attributes' => array('id' => 'prev'),
      'fragment' => $prev_frag,
    ));
    $slides_output .= l(t('Next »'), request_path(), array(
      'attributes' => array('id' => 'next'),
      'fragment' => $next_frag,
    ));
    $slides_output .= '</div>';
    $slides_output .= '</div>';
  }
  $themes_output .= '</div>';
  $output = $themes_output . $slides_output;
  return $output;
}

/**
 * Helper function to make exhibit layout field values human-readable.
 */
function _bc_islandora_get_ex_layout($lid) {
  $layout = '';
  switch ($lid) {
    case 1:
      $layout = 'left';
      break;

    case 2:
      $layout = 'right';
      break;

    case 4:
      $layout = 'center';
      break;
  }
  return $layout;
}

/**
 * Helper function to generate download links for given datastreams.
 */
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

/**
 * Helper function to determine whether $object is a textual document.
 *
 * Called by barnard_theme's template.php.
 */
function _bc_islandora_is_document($object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $output = '';
  if (is_object($object->relationships)) {
    $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
    if (!empty($results) && $parent_collection = $results[0]['object']['value']) {
      $collection = islandora_object_load($parent_collection);
      // If this object's parent collection's pid is the same as our database
      // variable bc_islandora_documents_pid, the answer is YES.
      if ($collection->id == variable_get('bc_islandora_documents_pid', '')) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * Helper function to get the sequential position of an object.
 *
 * Called by barnard_theme's template.php
 */
function _bc_islandora_get_sequence($object) {
  $seq_results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
  if (!empty($seq_results)) {
    return $seq_results[0]['object']['value'];
  }
  return NULL;
}
